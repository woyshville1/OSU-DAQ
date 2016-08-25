#include <stdlib.h>
#include <CAENDigitizer.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include <chrono>
#include <iostream>

#include "DAQ.h"
#include "DataFormat.h"

#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TGraph.h"

using namespace std;

bool RunActive = false;
pthread_mutex_t RunStatusMutex;

bool StopAcquisition = false;			// Controls when run is stopped
pthread_mutex_t AcquisitionStatusMutex; // Locks access to StopAcquisition while it's being checked

EventNode * masterHead = NULL;          // pointers to keep track of the master queue
EventNode * masterTail = NULL;          // used to move events between threads

pthread_mutex_t MasterQueueMutex;		// locks control of the master event queue

unsigned int eventsInQueue = 0;
unsigned int recordedEvents = 0;

bool useChargeMode;

//wip
//use getch to assign return vals
bool quitRun;
bool closeCaen;

uint32_t recordLength = 0;

string outputFileName = "";

//wip
void * kbdMonLoop(void*) {
	
	acq->ConnectToBoard();
	
	while(!closeCaen){
	
		while(!quitRun){
			//When kbdMonLoop detects a "q" keypress, set quitRun=1
			//break this loop
		}
		//Now that we have quitRun, go back to start of acquisition process(?) 
		//such as choosing filename and time limit?
		
		//Then when kbdMonLoop detects an "x" keypress, set closeCaen=1
	}
	
	acq->CloseDevice();
	pthread_exit(NULL)
	return NULL;
}

void * acquisitionLoop(void*) {

  cout << "Acquisition loop started..." << endl;

  DAQ * acq = new DAQ();

  // durp -- shouldn't need to write this every time
  string configFile = "";
  cout << endl << "Enter configuration file name (e.g. pmt.xml): ";
  cin >> configFile;
  cout << endl << endl;

  acq->ReadConfiguration(configFile);

  useChargeMode = acq->UseChargeMode();
  recordLength = acq->GetRecordLength();

  acq->InitializeBoardParameters();
  acq->MallocReadoutBuffer();

  pthread_mutex_lock(&RunStatusMutex);
  RunActive = true;
  pthread_mutex_unlock(&RunStatusMutex);

  acq->StartRun();

  EventNode * localHead = NULL;
  EventNode * localTail = NULL;

  pthread_mutex_lock(&AcquisitionStatusMutex);

  while (!StopAcquisition) {
    pthread_mutex_unlock(&AcquisitionStatusMutex);

    acq->PrepareEvent();

    acq->ReadCycle(&localHead, &localTail, eventsInQueue);

    if (localHead != 0) {
      pthread_mutex_lock(&MasterQueueMutex);

      if (masterHead == NULL) {
	       masterHead = localHead;
	       masterTail = localTail;
      }
      else {
	       masterTail->nxt = localHead;
	       localHead->prv = masterTail;
	       masterTail = localTail;
      }

      pthread_mutex_unlock(&MasterQueueMutex);

      localHead = NULL;
      localTail = NULL;
    }

    pthread_mutex_lock(&AcquisitionStatusMutex);
  }
  pthread_mutex_unlock(&AcquisitionStatusMutex);

  acq->StopRun();

  delete acq;

  pthread_exit(NULL);

  return NULL;
}

void * eventProcessLoop(void*) {

  cout << "Process thread has begun " << endl;

  EventNode * localHead;
  EventNode * localTail;

  // Wait for run to start
  pthread_mutex_lock(&RunStatusMutex);
  while (!RunActive) {
	  pthread_mutex_unlock(&RunStatusMutex);
	  Sleep(100); // ms
	  pthread_mutex_lock(&RunStatusMutex);
  }
  pthread_mutex_unlock(&RunStatusMutex);

  // durp -- can't accept charge integral window from RunConfiguration because cfg isn't in this thread/scope
  // durp -- needs an option to enable/disable saving all waveforms, since they become large
  // durp -- lastly, needs a copy of cfg so that it can shut off channels to save space
  DataFormat * output = new DataFormat(outputFileName.c_str(), recordLength, useChargeMode);

  pthread_mutex_lock(&AcquisitionStatusMutex);
  while (!StopAcquisition) {
    pthread_mutex_unlock(&AcquisitionStatusMutex);

    if (pthread_mutex_trylock(&MasterQueueMutex) == 0) { // Did we get control of the master queue?

      localHead = masterHead;
      localTail = masterTail;
      masterHead = NULL;
      masterTail = NULL;

      pthread_mutex_unlock(&MasterQueueMutex);
    }
    else continue;

    while (localHead != NULL) {

      freeEvent(localHead->prv);

      output->AddEvent(localHead); // durp -- doesn't calculate charge

      if(useChargeMode) {
        eventsInQueue -= localHead->NumEvents[0];
        recordedEvents += localHead->NumEvents[0];
      }
      else {
        eventsInQueue--;
        recordedEvents++;
      }

      localHead = localHead->nxt;
    }

    freeEvent(localTail);
    pthread_mutex_lock(&AcquisitionStatusMutex);
  }
  pthread_mutex_unlock(&AcquisitionStatusMutex);

  delete output; // deconstructor calls TFile::Write()

  cout << "All events processed" << endl;
  pthread_exit(NULL);

  return NULL;
}

int main(int argc, char* argv[]) {

  // If runtime is not specified
  if (argc < 2) {
    cout << "Usage: " << argv[0] << " <seconds>" << endl;
    return(0);
  }

  // Get output file name
  cout << endl << "Enter desired output file name: ";
  cin >> outputFileName;
  cout << endl << endl;

  // Set up threads
  pthread_t readThread;
  pthread_t processThread;
  //wip
  pthread_t kbdMonThread;

  pthread_attr_t joinableAttr;
  pthread_attr_init(&joinableAttr);

  pthread_attr_setdetachstate(&joinableAttr, PTHREAD_CREATE_JOINABLE);

  pthread_mutex_init(&MasterQueueMutex, NULL);
  pthread_mutex_init(&AcquisitionStatusMutex, NULL);
  pthread_mutex_init(&RunStatusMutex, NULL);

  pthread_create(&readThread, &joinableAttr, acquisitionLoop, NULL);
  pthread_create(&processThread, &joinableAttr, eventProcessLoop, NULL);
  //wip
  pthread_create(&kbdMonThread, &joinableAttr, kbdMonLoop, NULL);

  // Set up timing variables
  typedef chrono::high_resolution_clock time;
  time::time_point initial, final;
  chrono::duration<float> elapsed(0);
  double lastPrint = 0;
  chrono::duration<float> runLength(atoi(argv[1]));

  // Wait for run to start
  pthread_mutex_lock(&RunStatusMutex);
  while (!RunActive) {
    pthread_mutex_unlock(&RunStatusMutex);
    Sleep(100); // ms
    pthread_mutex_lock(&RunStatusMutex);
  }
  pthread_mutex_unlock(&RunStatusMutex);

  bool endRun = false;
  initial = time::now();

  // If maximum number of events is specified
  if (argc == 3) {
    unsigned int maxEvents = atoi(argv[2]);
    while (!endRun) {
      final = time::now();
      if ((int)elapsed.count() % 5 == 0 && (int)elapsed.count() > lastPrint) { // every 5 seconds
	       lastPrint = (int)elapsed.count(); // elapsed iterates in seconds, so this prevents many messages in the same second
	       cout << "Recorded: " << recordedEvents << ", in queue: " << eventsInQueue << endl;
      }
      elapsed = final - initial;
      if (elapsed >= runLength || recordedEvents >= maxEvents) { endRun = true; }
    }
  }
  // If maximum number of events is not specified
  else if (argc == 2) {
    while (!endRun) {
      final = time::now();
      if ((int)elapsed.count() % 5 == 0 && (int)elapsed.count() > lastPrint) { // every 5 seconds
	       lastPrint = (int)elapsed.count(); // elapsed iterates in seconds, so this prevents many messages in the same second
         cout << "Recorded: " << recordedEvents << ", in queue: " << eventsInQueue << ", ";
         cout << "Time elapsed: " << elapsed.count() << " seconds" << endl;
      }
      elapsed = final - initial;
      if (elapsed >= runLength) { endRun = true; }
    }
  }

  pthread_mutex_lock(&AcquisitionStatusMutex);
  StopAcquisition = true;
  pthread_mutex_unlock(&AcquisitionStatusMutex);

  void* status;

  pthread_attr_destroy(&joinableAttr);

  int rc; // Is rc ever used?
  rc = pthread_join(readThread, &status);
  rc = pthread_join(processThread, &status);

  pthread_mutex_destroy(&MasterQueueMutex);
  pthread_mutex_destroy(&AcquisitionStatusMutex);
  pthread_mutex_destroy(&RunStatusMutex);

  cout << endl << "Acquisition complete!" << endl << endl;
  cout << "Recorded: " << recordedEvents << " events!" << endl;
  cout << "Run Time: " << elapsed.count() << "s" << endl;

  /*
  if (!useChargeMode) {
    TFile * fTest = new TFile((TString)outputFileName, "READ");
    TTree * tTest = (TTree*)fTest->Get("Channel_1");
    unsigned short nEff;
    unsigned short totalEff = 0;
    tTest->SetBranchAddress("TriggerCount", &nEff);
    for (int i = 0; i < tTest->GetEntries(); i++) {
      tTest->GetEntry(i);
      if (nEff == 0 || nEff == 1) totalEff += 1;
      else totalEff += nEff;
    }
    fTest->Close();
    cout << "Effective events: " << totalEff << endl << endl;
  }
  */

  return 0;
}

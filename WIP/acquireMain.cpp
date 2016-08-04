#include <stdlib.h>
#include <CAENDigitizer.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include <chrono>

#include "DAQ.h"

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

uint32_t recordLength = 0;

string outputFileName = "";

void * acquisitionLoop(void*) {

  cout << "Acquisition loop started..." << endl;

  int statusCode = -1;

  RunConfiguration cfg;
  statusCode = cfg.ParseConfigFile("pmt.xml");
  recordLength = cfg.RecordLength;

  if(statusCode != 0 || !cfg.CheckAllParametersSet()) {
    cout << endl << endl << "Invalid configuration file!" << endl << endl;
    return 0;
  }

  useChargeMode = cfg.useChargeMode;

  DAQ * acq = new DAQ();

  acq->ConnectToBoard(cfg);

  acq->InitializeBoardParameters(cfg);
  acq->MallocReadoutBuffer();

  pthread_mutex_lock(&RunStatusMutex);
  RunActive = true;
  pthread_mutex_unlock(&RunStatusMutex);

  acq->StartRun();

  EventNode* localHead = NULL;
  EventNode* localTail = NULL;

  pthread_mutex_lock(&AcquisitionStatusMutex);

  while (!StopAcquisition) {
    pthread_mutex_unlock(&AcquisitionStatusMutex);

    acq->PrepareEvent();

    if (useChargeMode) { statusCode = acq->ProcessDPPEvent(&localHead, &localTail, eventsInQueue); }
    else { statusCode = acq->ProcessEvent(&localHead, &localTail, eventsInQueue); }

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

    /////////////////////////////////////////////////
    //end get and append
    ////////////////////////////////////////////////

    pthread_mutex_lock(&AcquisitionStatusMutex);
  }
  pthread_mutex_unlock(&AcquisitionStatusMutex);

  acq->StopRun();
  acq->CloseDevice();

  delete acq;

  pthread_exit(NULL);

  return NULL;
}

void * eventProcessLoop(void*) {

  cout << "Process thread has begun " << endl;

  EventNode * localHead;
  EventNode * localTail;

  TFile * outputFile = new TFile((TString)outputFileName, "RECREATE");

  unsigned short _TriggerCount; // uint16_t
  unsigned short _TimeCount; // uint16_t
  unsigned char _EventId; // uint8_t
  unsigned long long _TDC; // uint64_t

  float _waveMax;
  float _waveMin;

  float _chargeIntegral;

  unsigned long long _StartIndexCell;
  float _Charge;

  vector<TTree*> trees;
  for (int i = 0; i < nChannels; i++) {
    TString name = "Channel_";
    name += Form("%d", i);

    TTree * tree = new TTree(name, "CAEN Events");
    tree->SetAutoSave(10000000); // 10 MB

    if (useChargeMode) {
      tree->Branch("StartIndexCell", &_StartIndexCell, "_StartIndexCell/l");
      tree->Branch("Charge", &_Charge, "_Charge/F");
    }
    else {
      tree->Branch("TriggerCount", &_TriggerCount, "_TriggerCount/s");
      tree->Branch("TimeCount", &_TimeCount, "_TimeCount/s");
      tree->Branch("EventId", &_EventId, "_EventId/b");
      tree->Branch("TDC", &_TDC, "_TDC/l");

      tree->Branch("Max", &_waveMax, "_waveMax/F");
      tree->Branch("Min", &_waveMin, "_waveMin/F");

      tree->Branch("Charge", &_chargeIntegral, "_chargeIntegral/F");
    }

    trees.push_back(tree);
  }

  int numSavedGraphs = 0;
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

      if (useChargeMode) {

	for (int ch = 0; ch < nChannels; ch++) {

	  for (int ev = 0; ev < localHead->NumEvents[ch]; ev++) {
	    _StartIndexCell = localHead->StartIndexCell[ch][ev];
	    _Charge = (float)(localHead->Charge[ch][ev]);
	    trees[ch]->Fill();
	  }

	}

	eventsInQueue -= localHead->NumEvents[0];
	recordedEvents += localHead->NumEvents[0];

      }

      else {

	for (int i = 0; i < nChannels; i++) {

	  int samIndex = (int)(i / 2);
	  int chanIndex = i % 2;

	  _TriggerCount = localHead->TriggerCount[samIndex][chanIndex];
	  _TimeCount = localHead->TimeCount[samIndex][chanIndex];
	  _EventId = localHead->EventId[samIndex];
	  _TDC = localHead->TDC[samIndex];

	  float _max = -1.e6;
	  float _min = 1.e6;

	  // Find max and min
	  for (int j = 0; j < (int)recordLength; j++) {
	    if (localHead->Waveform[i][j] * ADCTOVOLTS > _max) {
	      _max = localHead->Waveform[i][j] * ADCTOVOLTS;
	    }
	    if (localHead->Waveform[i][j] * ADCTOVOLTS < _min) {
	      _min = localHead->Waveform[i][j] * ADCTOVOLTS;
	    }
	  }

	  int _chargeStartCell = 0;
	  int _chargeLength = 0;
	  double _avgBaseline = 0;
	  if (i == 3) {
	    _chargeStartCell = 17;
	    _chargeLength = 363;
	  }
	  if (i == 7) {
	    _chargeStartCell = 17;
	    _chargeLength = 367;
	  }
	  /*if (i == 6) {
	    _chargeStartCell = 255;
	    _chargeLength = 644;
	    }
	    if (i == 12) {
	    _chargeStartCell = 255;
	    _chargeLength = 385;
	    }*/
					

	  // Calculate baseline
	  for (int j = _chargeStartCell; j < _chargeStartCell + 16; j++) {
	    _avgBaseline += localHead->Waveform[i][j] * ADCTOVOLTS;
	  }
	  _avgBaseline = _avgBaseline / 16.;
	  // Calculate charge integral
	  _chargeIntegral = 0;
	  for (int j = _chargeStartCell; j < _chargeStartCell + _chargeLength; j++) {
	    // Charge in pC = (Voltage in mV - Baseline) * Time per sample / 50 Ohms * 1e9 pC/mC
	    _chargeIntegral += (localHead->Waveform[i][j] * ADCTOVOLTS - _avgBaseline) * (12.50e-10 / 50.0) * 1e9;
	  }
					
	  // Save waveforms
	  if ((i == 1)  && numSavedGraphs < 50 && true) {
	    float gr_x[1024];
	    for (unsigned int j = 0; j < 1024; j++) gr_x[j] = (float)j;
	    float gr_y[1024];
	    for (unsigned int j = 0; j < 1024; j++) {
	      if (j >= localHead->ChSize[i / 2]) gr_y[j] = -10000;
	      else gr_y[j] = localHead->Waveform[i][j] * ADCTOVOLTS;
	    }							TGraph * gr = new TGraph(1024, gr_x, gr_y);
	    TString gr_name = Form("gr_ch%d_%d", i, recordedEvents);
	    gr->Write(gr_name);
	    numSavedGraphs++;			
	  }

	  _waveMax = _max;
	  _waveMin = _min;

	  trees[i]->Fill();
	}
	eventsInQueue--;
	recordedEvents++;
      }
      localHead = localHead->nxt;
    }

    freeEvent(localTail);
    pthread_mutex_lock(&AcquisitionStatusMutex);
  }
  pthread_mutex_unlock(&AcquisitionStatusMutex);

  for (int i = 0; i < nChannels; i++) trees[i]->Write();

  outputFile->Close();

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

  pthread_attr_t joinableAttr;
  pthread_attr_init(&joinableAttr);

  pthread_attr_setdetachstate(&joinableAttr, PTHREAD_CREATE_JOINABLE);

  pthread_mutex_init(&MasterQueueMutex, NULL);
  pthread_mutex_init(&AcquisitionStatusMutex, NULL);
  pthread_mutex_init(&RunStatusMutex, NULL);

  pthread_create(&readThread, &joinableAttr, acquisitionLoop, NULL);
  pthread_create(&processThread, &joinableAttr, eventProcessLoop, NULL);

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
    int maxEvents = atoi(argv[2]);
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
	cout << "time elapsed: " << elapsed.count() << " seconds" << endl;
	//cout << "Time Elapsed: " << elapsed.count() << " Seconds" << endl;
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

  return 0;
}

#ifndef __DAQ_H
#define __DAQ_H

#include <stdbool.h>
#include <iostream>
#include <vector>
#include <map>

#include "CAENDigitizer.h"
#include "RunConfiguration.h"

#define nChannels            16
#define nSamBlocks           8
#define nChannelsPerSamBlock 2

#define MIN_DAC_RAW_VALUE -1.25
#define MAX_DAC_RAW_VALUE 1.25

#define ADCTOVOLTS 0.61

using namespace std;

typedef struct EventNode EventNode;
struct EventNode {
  EventNode * nxt;
  EventNode * prv;

  uint8_t  GrPresent[nSamBlocks];       // if the group has data the value is 1 (else 0)

  uint32_t ChSize[nSamBlocks];          // number of samples stored in DataChannel array
  float** Waveform;
  uint16_t TriggerCount[nSamBlocks][nChannelsPerSamBlock];
  uint16_t TimeCount[nSamBlocks][nChannelsPerSamBlock];
  uint8_t EventId[nSamBlocks];
  uint64_t TDC[nSamBlocks];
  float PosEdgeTimeStamp[nSamBlocks];
  float NegEdgeTimeStamp[nSamBlocks];
  uint16_t PeakIndex[nSamBlocks];
  float Peak[nSamBlocks];
  float Baseline[nSamBlocks];

  uint16_t StartIndexCell[nChannels][256];
  float Charge[nChannels][256];
  int NumEvents[nChannels];

};

void freeEvent(EventNode*);

class DAQ {

 public:
  DAQ();
  virtual ~DAQ() { errorMessageMap.clear(); };

  // DAQ() first reads default.xml, and then ReadConfiguration can be called to overwrite those settings
  bool ReadConfiguration(string filename);

  // CAEN V1743 settings read from RunConfiguration object
  // Called in InitializeBoardParameters()
  void SetTriggerDelay();
  void SetPulserParameters();
  void SetSamplingFrequency();
  void SetPuslerParameters();
  void SetTriggerThresholds();
  void SetTriggerPolarities();
  void SetTriggerSource();
  void SetIOLevel();
  void SetChannelDCOffsets();
  void SetCorrectionLevel();
  void SetMaxNumEventsBLT();
  void SetRecordLength();
  void SetAnalogMonitorOutput();

  void ConnectToBoard();
  void InitializeBoardParameters();
  void MallocReadoutBuffer();
  void SetGroupEnableMask();

  void StartRun();
  void StopRun();

  void PrepareEvent();

  int ReadEventBuffer();
  int GetNumberOfEvents();
  int DecodeEvent(int eventNumber);

  void CloseDevice();

  // Read the event buffer, and for every event read create an EventNode
  // Then insert the EventNode into the master queue
  int ReadCycle(EventNode** head, EventNode** tail, unsigned int& queueCount) {
    if(cfg->useChargeMode) return ProcessDPPEvent(head, tail, queueCount);
    else return ProcessEvent(head, tail, queueCount);
  }

  int ProcessEvent(EventNode** head, EventNode** tail, unsigned int& queueCount);
  int ProcessDPPEvent(EventNode** head, EventNode** tail, unsigned int& queueCount);

  bool UseChargeMode() { return cfg->useChargeMode; };
  uint32_t GetRecordLength() { return cfg->RecordLength; };

 private:

  map<CAEN_DGTZ_ErrorCode, string> errorMessageMap;
  void Try(char * name, CAEN_DGTZ_ErrorCode code, bool verbose = false);
  void Require(char * name, CAEN_DGTZ_ErrorCode code, bool verbose = false);

  void ReadRegister(uint16_t reg, uint32_t * data, bool verbose = false);
  void WriteRegister(uint16_t reg, uint32_t data, bool verbose = false);

  int DeviceHandle;

  TriggerType_t TriggerType;

  uint32_t * ReadoutBuffer;
  uint32_t ReadoutBufferSize;
  uint32_t MaxReadoutBufferSize;
  uint32_t DPPReadoutBufferSize;

  CAEN_DGTZ_EventInfo_t eventInfo;

  CAEN_DGTZ_X743_EVENT_t * CurrentEvent;
  CAEN_DGTZ_DPP_X743_Event_t * DPPEvents[nChannels];

  RunConfiguration * cfg;

  int DACValue(double value) { return (int)((MAX_DAC_RAW_VALUE - value) / (MAX_DAC_RAW_VALUE - MIN_DAC_RAW_VALUE) * 0xFFFF); };

};

#endif

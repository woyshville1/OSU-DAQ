#ifndef DAQ_H
#define DAQ_H

#include <stdbool.h>
#include <iostream>
#include <vector>
#include <map>

#include "CAENDigitizer.h"
#include "RunConfiguration.h"

#include "TROOT.h"

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

  //uint8_t  GrPresent[nSamBlocks];       // if the group has data the value is 1 (else 0)

  uint32_t ChSize[nSamBlocks];          // number of samples stored in DataChannel array
  float** Waveform;
  uint16_t TriggerCount[nSamBlocks][nChannelsPerSamBlock];
  uint16_t TimeCount[nSamBlocks][nChannelsPerSamBlock];
  uint8_t EventId[nSamBlocks];
  //uint16_t StartIndexCell[nSamBlocks];
  uint64_t TDC[nSamBlocks];
  //float PosEdgeTimeStamp[nSamBlocks];
  //float NegEdgeTimeStamp[nSamBlocks];
  //uint16_t PeakIndex[nSamBlocks];
  //float Peak[nSamBlocks];
  //float Baseline[nSamBlocks];
  //float Charge[nSamBlocks];

  uint16_t StartIndexCell[nChannels][256];
  float Charge[nChannels][256];
  int NumEvents[nChannels];

};

void freeEvent(EventNode*);

class DAQ {

 public:
  DAQ();
  virtual ~DAQ() { errorMessageMap.clear(); };

  void SetTriggerDelay(uint8_t TriggerDelay);
  void SetPulserParameters(vector<ChannelConfiguration> channels);
  void SetSamplingFrequency(CAEN_DGTZ_SAMFrequency_t SamplingFrequency);
  void SetPuslerParameters(vector<ChannelConfiguration> channels);
  void SetTriggerThreshold(double TriggerThreshold, int channel);
  void SetTriggerPolarity(CAEN_DGTZ_TriggerPolarity_t TriggerPolarity, int channel);
  void SetTriggerSource(RunConfiguration cfg);
  void SetIOLevel(CAEN_DGTZ_IOLevel_t level);
  void SetChannelDCOffset(RunConfiguration cfg);
  void SetCorrectionLevel(CAEN_DGTZ_SAM_CORRECTION_LEVEL_t level);
  void SetMaxNumEventsBLT(int MaxNumEventsBLT);
  void SetRecordLength(uint32_t RecordLength);
  void SetAnalogMonitorOutput();

  void ConnectToBoard(RunConfiguration cfg);
  void InitializeBoardParameters(RunConfiguration cfg);
  void MallocReadoutBuffer();
  void SetGroupEnableMask(RunConfiguration cfg);

  void StartRun();
  void StopRun();

  void PrepareEvent();

  int ReadEventBuffer();
  int GetNumberOfEvents();
  int DecodeEvent(int eventNumber);

  void CloseDevice();

  int ProcessEvent(EventNode** head, EventNode** tail, unsigned int& queueCount);
  int ProcessDPPEvent(EventNode** head, EventNode** tail, unsigned int& queueCount);

 private:

  map<CAEN_DGTZ_ErrorCode, string> errorMessageMap;
  void Try(char * name, CAEN_DGTZ_ErrorCode code, bool verbose = false);
  void Require(char * name, CAEN_DGTZ_ErrorCode code, bool verbose = false);

  void ReadRegister(uint16_t reg, uint32_t * data, bool verbose = false);
  void WriteRegister(uint16_t reg, uint32_t data, bool verbose = false);

  bool useChargeMode = false;
  bool useIRQ = true;

  int DeviceHandle;
  int EventNumber;
  int TotalEventNumber;

  TriggerType_t TriggerType;

  uint32_t * ReadoutBuffer;
  uint32_t ReadoutBufferSize;
  uint32_t MaxReadoutBufferSize;
  uint32_t DPPReadoutBufferSize;

  uint32_t irqTimeout;

  CAEN_DGTZ_EventInfo_t eventInfo;

  CAEN_DGTZ_X743_EVENT_t * CurrentEvent;
  CAEN_DGTZ_DPP_X743_Event_t * DPPEvents[nChannels];

  int DACValue(double value) { return (int)((MAX_DAC_RAW_VALUE - value) / (MAX_DAC_RAW_VALUE - MIN_DAC_RAW_VALUE) * 0xFFFF); };

};

#endif

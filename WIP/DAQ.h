#ifndef DAQ_H
#define DAQ_H

#include <stdbool.h>
#include <iostream>
#include <vector>

#include "CAENDigitizer.h"
#include "RunConfiguration.h"

#include "TROOT.h"

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
	void WriteRegister(uint16_t reg, uint32_t * data, bool verbose = false);

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

};

#endif

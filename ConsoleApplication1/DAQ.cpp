#ifndef DAQ_CPP
#define DAQ_CPP

#include "DAQ.h"

using namespace std;

// CopyEvent translates the CAEN Event structure into the much simpler EventNode structure
// Once done, waveform samples can be accessed with EventNode->Waveform[channel][sample]
EventNode * CopyEvent(CAEN_DGTZ_X743_EVENT_t * evt) {

	EventNode * node = new EventNode();

	node->nxt = NULL;
	node->prv = NULL;

	float **obj = (float**)malloc(nChannels * sizeof(float*));

	for (int samIndex = 0; samIndex < nSamBlocks; samIndex++) {

		//node->GrPresent[samIndex] = evt->GrPresent[samIndex];

		node->ChSize[samIndex] = evt->DataGroup[samIndex].ChSize;

		for (int i = 0; i < nChannelsPerSamBlock; i++) {
			node->TriggerCount[samIndex][i] = evt->DataGroup[samIndex].TriggerCount[i];
			node->TimeCount[samIndex][i] = evt->DataGroup[samIndex].TimeCount[i];

			size_t m = (size_t)evt->DataGroup[samIndex].ChSize * sizeof(float);
			obj[2 * samIndex + i] = (float*)malloc(m);
			memmove((void*)obj[nChannelsPerSamBlock * samIndex + i], (const void*)evt->DataGroup[samIndex].DataChannel[i], m);
		}

		node->EventId[samIndex] = evt->DataGroup[samIndex].EventId;
		//node->StartIndexCell[samIndex] = evt->DataGroup[samIndex].StartIndexCell;
		node->TDC[samIndex] = evt->DataGroup[samIndex].TDC;
		//node->PosEdgeTimeStamp[samIndex] = evt->DataGroup[samIndex].PosEdgeTimeStamp;
		//node->NegEdgeTimeStamp[samIndex] = evt->DataGroup[samIndex].NegEdgeTimeStamp;
		//node->PeakIndex[samIndex] = evt->DataGroup[samIndex].PeakIndex;
		//node->Peak[samIndex] = evt->DataGroup[samIndex].Peak;
		//node->Baseline[samIndex] = evt->DataGroup[samIndex].Baseline;
		//node->Charge[samIndex] = evt->DataGroup[samIndex].Charge;

	}

	node->Waveform = obj;

	return node;

}

EventNode * CopyDPPEvent(CAEN_DGTZ_DPP_X743_Event_t ** evt, uint32_t * NumEvents) {

	EventNode * node = new EventNode();

	node->nxt = NULL;
	node->prv = NULL;

	for (int ch = 0; ch < nChannels; ch++) {

		node->NumEvents[ch] = (int)NumEvents[ch];

		for (int ev = 0; ev < node->NumEvents[ch]; ev++) {
			node->Charge[ch][ev] = evt[ch][ev].Charge;
			node->StartIndexCell[ch][ev] = evt[ch][ev].StartIndexCell;
		}
	}

	return node;

}

void freeArray(void** obj) {
	if (obj != NULL) {
		for (int i = 0; i < nChannels; i++) free(obj[i]);
		free(obj);
	}
}

void freeEvent(EventNode * node) {
	if (node != NULL) {
		freeArray((void**)node->Waveform);
		free(node);
	}
}

void DAQ::ConnectToBoard(RunConfiguration cfg) {

	CAEN_DGTZ_ErrorCode error = CAEN_DGTZ_OpenDigitizer(cfg.LinkType,
																											cfg.LinkNum,
																											cfg.ConetNode,
																											cfg.VMEBaseAddress,
																											&DeviceHandle);
	if (error != CAEN_DGTZ_Success) {
		cout << "Failed to open digitizer: error " << error << endl;
		exit(0);
	}

	CAEN_DGTZ_BoardInfo_t info;
	error = CAEN_DGTZ_GetInfo(DeviceHandle, &info);
	if (error != CAEN_DGTZ_Success) {
		printf("Failed to get digitizer info: error %d \n", error);
		exit(0);
	}

	printf("\n\n");
	printf("Digitizer opened\n");
	printf("Getting Info From Digitizer:\n");
	printf("Model Name:              %s\n", info.ModelName);
	printf("Model:                   %d\n", info.Model);
	printf("Channels:                %d\n", info.Channels);
	printf("Form Factor:             %d\n", info.FormFactor);
	printf("Family Code:             %d\n", info.FamilyCode);
	printf("ROC_FirmwareRel:         %s\n", info.ROC_FirmwareRel);
	printf("AMC_FirmwareRel:         %s\n", info.AMC_FirmwareRel);
	printf("SerialNumber:            %d\n", info.SerialNumber);
	printf("PCB_Revision:            %d\n", info.PCB_Revision);
	printf("ADC_NBits:               %d\n", info.ADC_NBits);
	printf("SAMCorrectionDataLoaded: %d\n", info.SAMCorrectionDataLoaded);
	printf("CommHandle:              %d\n", info.CommHandle);
	printf("License:                 %s\n", info.License);
	printf("\n\n");

	CAEN_DGTZ_Reset(DeviceHandle);

}

void DAQ::InitializeBoardParameters(RunConfiguration cfg) {

	uint32_t modeSelection = 0;
	modeSelection |= 1 << 0;
	modeSelection |= 1 << 1;
	int status_wr = CAEN_DGTZ_WriteRegister(DeviceHandle, 0x8144, modeSelection);
	printf("Write Register Status: %d\n", status_wr);

	status_wr = CAEN_DGTZ_WriteRegister(DeviceHandle, 0x81B4, 4);
	printf("Write Register 0x81B4 status: %d\n", status_wr);

	useChargeMode = cfg.useChargeMode;
	useIRQ = cfg.useIRQ;

	SetRecordLength(cfg.RecordLength);
	SetTriggerDelay(cfg.TriggerDelay);
	SetSamplingFrequency(cfg.SAMFrequency);

	SetPulserParameters(cfg.channels);

	for(int i = 0; i < nChannels; i++) SetTriggerThreshold(cfg.channels[i].triggerThreshold, i);

	SetTriggerSource(cfg);
	SetIOLevel(cfg.IOLevel);

	for(int i = 0; i < nChannels; i++) SetTriggerPolarity(cfg.channels[i].triggerPolarity, i);

	for(int i = 0; i < nChannels; i++) SetChannelDCOffset(cfg.channels[i].dcOffset, i);

	SetCorrectionLevel(cfg.SAMCorrectionLevel);

	if(cfg.useIRQ) {
		CAEN_DGTZ_SetInterruptConfig(DeviceHandle,
																 CAEN_DGTZ_ENABLE,
															   cfg.irqLevel,
															   cfg.irqStatusId,
															   cfg.irqNEvents,
															   cfg.irqMode);

		irqTimeout = cfg.irqTimeout;
	}

	SetMaxNumEventsBLT(cfg.MaxNumEventsBLT);

	CAEN_DGTZ_SetAcquisitionMode(DeviceHandle, CAEN_DGTZ_SW_CONTROLLED);

	if(cfg.useChargeMode) {

		CAEN_DGTZ_SetSAMAcquisitionMode(DeviceHandle, CAEN_DGTZ_AcquisitionMode_DPP_CI);

		CAEN_DGTZ_DPP_X743_Params_t DPPParams;

		if      (cfg.suppressChargeBaseline == 1) { DPPParams.disableSuppressBaseline = CAEN_DGTZ_DISABLE; }
		else if (cfg.suppressChargeBaseline == 0) { DPPParams.disableSuppressBaseline = CAEN_DGTZ_ENABLE; }
		// DPPParams.disableSuppressBaseline = cfg.suppressChargeBaseline;
		for (int i = 0; i < nChannels; i++) {
			DPPParams.startCell[i] = cfg.channels[i].chargeStartCell;
			DPPParams.chargeLength[i] = cfg.channels[i].chargeLength;
			DPPParams.enableChargeThreshold[i] = cfg.channels[i].enableChargeThreshold;
			DPPParams.chargeThreshold[i] = cfg.channels[i].chargeThreshold;
		}

		CAEN_DGTZ_SetDPPParameters(DeviceHandle, 0, &DPPParams);

	}

	SetChannelEnableMask(cfg.channels);

}

void DAQ::MallocReadoutBuffer() {
	CAEN_DGTZ_MallocReadoutBuffer(DeviceHandle, (char**)&ReadoutBuffer, &MaxReadoutBufferSize);
	if(useChargeMode) CAEN_DGTZ_MallocDPPEvents(DeviceHandle, (void**)DPPEvents, &DPPReadoutBufferSize);
}

void DAQ::SetChannelEnableMask(vector<ChannelConfiguration> channels) {

	int channelsMask = 0;
	for (int i = 0; i < nChannels / 2; i++) channelsMask +=
		(channels[2*i].enable + (channels[2*i + 1].enable << 1)) << (2*i);
	channelsMask = (~channelsMask) & 0xFF;

	CAEN_DGTZ_SetChannelEnableMask(DeviceHandle, channelsMask); // 0xFF = all channels enabled
}

void DAQ::StartRun() {

	if(!useChargeMode) CAEN_DGTZ_AllocateEvent(DeviceHandle, (void**)&CurrentEvent);
	
	CAEN_DGTZ_ClearData(DeviceHandle);
	CAEN_DGTZ_SWStartAcquisition(DeviceHandle);

}

void DAQ::StopRun() {
	
	uint32_t nReadBytes = 1;
	int error = 0;

	CAEN_DGTZ_SWStopAcquisition(DeviceHandle);

	// continue reading data from the board until it's empty
	printf("Clearing data...\n");
	while (error == 0 && nReadBytes > 0) {
		error = CAEN_DGTZ_ReadData(DeviceHandle,
			CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,
			(char*)ReadoutBuffer,
			&nReadBytes);
	}

	if(!useChargeMode) CAEN_DGTZ_FreeEvent(DeviceHandle, (void**)&CurrentEvent);
	else CAEN_DGTZ_FreeDPPEvents(DeviceHandle, (void**)DPPEvents);

}

void DAQ::PrepareEvent() {

	// if you shoud be sending software triggers, send one now
	if (TriggerType == SYSTEM_TRIGGER_AUTO || TriggerType == SYSTEM_TRIGGER_SOFT) CAEN_DGTZ_SendSWtrigger(DeviceHandle);

}

// Read an event from the board and store them in ReadoutBuffer and ReadoutBufferSize
int DAQ::ReadEventBuffer() {

	int error = CAEN_DGTZ_Success;

	if(useIRQ) {
		int error = CAEN_DGTZ_IRQWait(DeviceHandle, irqTimeout); // timeout in ms
		/*
		if (error != CAEN_DGTZ_Success) {
			//printf("IRQWait: %d\n", error);
			return -1;
		}
		*/
	}

	error = CAEN_DGTZ_ReadData(DeviceHandle,
							   CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,
							   (char*)ReadoutBuffer,
							   &ReadoutBufferSize);

	if (error != CAEN_DGTZ_Success) cout << "Error Code in function CAEN_DGTZ_ReadData: " << error << endl;

	return (ReadoutBufferSize > 0 && error == CAEN_DGTZ_Success) ? 0 : -1;

}

int DAQ::GetNumberOfEvents() {

	uint32_t numEvents;
	int status = CAEN_DGTZ_GetNumEvents(DeviceHandle, (char*)ReadoutBuffer, ReadoutBufferSize, &numEvents);
	if (status != 0) printf("Error in GetNumEvents: %d\n", status);

	return numEvents;

}

// Read the event in ReadoutBuffer and decode it into CurrentEvent
int DAQ::DecodeEvent(int eventNumber) {

	CAEN_DGTZ_EventInfo_t eventInfo;
	char * eventPtr = NULL;

	int result = 0;

	CAEN_DGTZ_ErrorCode error = CAEN_DGTZ_GetEventInfo(DeviceHandle,
																										 (char*)ReadoutBuffer,
																										 ReadoutBufferSize,
																										 eventNumber,
																										 &eventInfo,
																										 &eventPtr);

	if (error == CAEN_DGTZ_Success) {
		//printf("Going to DecodeEvent(%d...", eventNumber);
		error = CAEN_DGTZ_DecodeEvent(DeviceHandle, eventPtr, (void**)&CurrentEvent);
		//printf("done\n");
		if (error < 0) {
			cout << "Error Code in function CAEN_DGTZ_DecodeEvent: " << error << endl;
			result = -1;
		}
	}
	else {
		cout << "Error Code in function CAEN_DGTZ_GetEventInfo: " << error << endl;
		result = -1;
	}

	return result;
}

void DAQ::CloseDevice() {

	//StopRun(); // already done
	CAEN_DGTZ_FreeReadoutBuffer((char**)&ReadoutBuffer);
	CAEN_DGTZ_CloseDigitizer(DeviceHandle);

}

// Read the event buffer, and for every event read create an EventNode
// Then insert the EventNode into the master queue
int DAQ::ProcessEvent(EventNode** head, EventNode** tail, unsigned int& queueCount) {

	int status = 0;
	int nEventsRead = 0;

	status = ReadEventBuffer();
	if (status < 0) return status;

	nEventsRead = GetNumberOfEvents();
	queueCount += nEventsRead;

	for (int i = 0; i < nEventsRead; i++) {
		status = DecodeEvent(i);
		if (status < 0) return status;

		EventNode * newEvent = CopyEvent(CurrentEvent);

		if (*head == NULL) *head = newEvent;
		if (*tail != NULL) {
			(*tail)->nxt = newEvent;
			newEvent->prv = *tail;
		}
		*tail = newEvent;

	}

	return status;

}

int DAQ::ProcessDPPEvent(EventNode** head, EventNode** tail, unsigned int& queueCount) {

	int status = 0;

	status = ReadEventBuffer();
	if (status < 0) return status;

	uint32_t NumEvents_[nChannels];

	status = CAEN_DGTZ_GetDPPEvents(DeviceHandle, (char*)ReadoutBuffer, ReadoutBufferSize, (void**)DPPEvents, NumEvents_);
	if (status < 0) {
		printf("Error in GetDPPEvents: %d\n", status);
		return status;
	}

	queueCount += NumEvents_[0]; // all channels should have the same number of events

	EventNode * newEvent = CopyDPPEvent(DPPEvents, NumEvents_);

	if (*head == NULL) *head = newEvent;
	if (*tail != NULL) {
		(*tail)->nxt = newEvent;
		newEvent->prv = *tail;
	}
	*tail = newEvent; 

	return status;

}

// =================================

void DAQ::SetTriggerDelay(uint8_t TriggerDelay) {
	int status = 0;
	for (int i = 0; i < nSamBlocks; i++) {
		status = CAEN_DGTZ_SetSAMPostTriggerSize(DeviceHandle, i, TriggerDelay & 0xFF);
		if (status != 0) printf("Error in SetSAMPostTriggerSize: %d\n", status);
	}

}

void DAQ::SetSamplingFrequency(CAEN_DGTZ_SAMFrequency_t SamplingFrequency) {
	CAEN_DGTZ_SetSAMSamplingFrequency(DeviceHandle, SamplingFrequency);
}

void DAQ::SetPulserParameters(vector<ChannelConfiguration> channels) {

	for(int i = 0; i < nChannels; i++) {
		if(channels[i].testPulseEnable) CAEN_DGTZ_EnableSAMPulseGen(DeviceHandle,
																	i,
																	channels[i].testPulsePattern,
																	channels[i].testPulseSource);
		else CAEN_DGTZ_DisableSAMPulseGen(DeviceHandle, i);
	}

}

void DAQ::SetTriggerThreshold(double TriggerThreshold, int channel) {

	CAEN_DGTZ_SetChannelTriggerThreshold(DeviceHandle,
		channel,
		DACValue(TriggerThreshold));

}

void DAQ::SetTriggerPolarity(CAEN_DGTZ_TriggerPolarity_t TriggerPolarity, int channel) {

	CAEN_DGTZ_SetTriggerPolarity(DeviceHandle, channel, TriggerPolarity);

}

void DAQ::SetTriggerSource(RunConfiguration cfg) {

	TriggerType = cfg.TriggerType;

	if (TriggerType == SYSTEM_TRIGGER_SOFT) {
		CAEN_DGTZ_SetChannelSelfTrigger(DeviceHandle, CAEN_DGTZ_TRGMODE_DISABLED, 0xFF);
		CAEN_DGTZ_SetExtTriggerInputMode(DeviceHandle, CAEN_DGTZ_TRGMODE_DISABLED);
	}
	else if (TriggerType == SYSTEM_TRIGGER_NORMAL) {

		int channelsMask = 0;
		for (int i = 0; i < nChannels / 2; i++) channelsMask +=
			(cfg.channels[2*i].triggerEnable + (cfg.channels[2*i + 1].triggerEnable << 1)) << (2*i);
		channelsMask = (~channelsMask) & 0xFF;

		CAEN_DGTZ_SetChannelSelfTrigger(DeviceHandle, CAEN_DGTZ_TRGMODE_ACQ_ONLY, 0xFF);
		CAEN_DGTZ_SetChannelSelfTrigger(DeviceHandle, CAEN_DGTZ_TRGMODE_DISABLED, channelsMask);
		CAEN_DGTZ_SetExtTriggerInputMode(DeviceHandle, CAEN_DGTZ_TRGMODE_DISABLED);
	}
	else if (TriggerType == SYSTEM_TRIGGER_AUTO) {

		int channelsMask = 0;
		for (int i = 0; i < nChannels / 2; i++) channelsMask += (cfg.channels[2*i].triggerEnable + (cfg.channels[2*i + 1].triggerEnable << 1)) << (2*i);
		channelsMask = (~channelsMask) & 0xFF;

		CAEN_DGTZ_SetChannelSelfTrigger(DeviceHandle, CAEN_DGTZ_TRGMODE_ACQ_ONLY, 0xFF);
		CAEN_DGTZ_SetChannelSelfTrigger(DeviceHandle, CAEN_DGTZ_TRGMODE_DISABLED, channelsMask);
		CAEN_DGTZ_SetExtTriggerInputMode(DeviceHandle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);
	}
	else { // EXTERNAL ONLY
		CAEN_DGTZ_SetChannelSelfTrigger(DeviceHandle, CAEN_DGTZ_TRGMODE_DISABLED, 0xFF);
		CAEN_DGTZ_SetExtTriggerInputMode(DeviceHandle, CAEN_DGTZ_TRGMODE_ACQ_ONLY);
	}

}

void DAQ::SetIOLevel(CAEN_DGTZ_IOLevel_t level) {
	CAEN_DGTZ_SetIOLevel(DeviceHandle, level);
}

void DAQ::SetChannelDCOffset(double DCOffset, int channel) {

	CAEN_DGTZ_SetChannelDCOffset(DeviceHandle, channel, DACValue(DCOffset));

}

void DAQ::SetCorrectionLevel(CAEN_DGTZ_SAM_CORRECTION_LEVEL_t level) {
	CAEN_DGTZ_SetSAMCorrectionLevel(DeviceHandle, level);
}

void DAQ::SetMaxNumEventsBLT(int MaxNumEventsBLT) {
	CAEN_DGTZ_SetMaxNumEventsBLT(DeviceHandle, MaxNumEventsBLT);
}

void DAQ::SetRecordLength(uint32_t RecordLength) {
	CAEN_DGTZ_SetRecordLength(DeviceHandle, RecordLength);
}

#endif

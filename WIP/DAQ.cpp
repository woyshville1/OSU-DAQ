#ifndef DAQ_CPP
#define DAQ_CPP

#include "DAQ.h"
#include "Utilities.h"

DAQ::DAQ() {

	DeviceHandle = -1;

	// https://github.com/cjpl/caen-suite/blob/master/CAENDigitizer/include/CAENDigitizerType.h#L239
	errorMessageMap[CAEN_DGTZ_Success] = "Operation completed successfully.";
	errorMessageMap[CAEN_DGTZ_CommError] = "Communication error.";
	errorMessageMap[CAEN_DGTZ_GenericError] = "Unspecified error.";
	errorMessageMap[CAEN_DGTZ_InvalidParam] = "Invalid parameter.";
	errorMessageMap[CAEN_DGTZ_InvalidLinkType] = "Invalid link type.";
	errorMessageMap[CAEN_DGTZ_InvalidHandle] = "Invalid device handle.";
  errorMessageMap[CAEN_DGTZ_MaxDevicesError] = "Maximum number of devices exceeded.";
	errorMessageMap[CAEN_DGTZ_BadBoardType] = "The operation is not allowed on this type of board.";
  errorMessageMap[CAEN_DGTZ_BadInterruptLev] = "The interrupt level is not allowed.";
  errorMessageMap[CAEN_DGTZ_BadEventNumber] = "The event number is bad.";
  errorMessageMap[CAEN_DGTZ_ReadDeviceRegisterFail] = "Unable to read the registry.";
  errorMessageMap[CAEN_DGTZ_WriteDeviceRegisterFail] = "Unable to write into the registry.";
  errorMessageMap[CAEN_DGTZ_InvalidChannelNumber] = "The channel number is invalid.";
  errorMessageMap[CAEN_DGTZ_ChannelBusy] = "The channel is busy.";
  errorMessageMap[CAEN_DGTZ_FPIOModeInvalid] = "Invalid FPIO mode.";
  errorMessageMap[CAEN_DGTZ_WrongAcqMode] = "Wrong acquisition mode.";
	errorMessageMap[CAEN_DGTZ_FunctionNotAllowed] = "This function is not allowed for this module.";
  errorMessageMap[CAEN_DGTZ_Timeout] = "Communication timeout.";
  errorMessageMap[CAEN_DGTZ_InvalidBuffer] = "The buffer is invalid.";
  errorMessageMap[CAEN_DGTZ_EventNotFound] = "The event is not found.";
  errorMessageMap[CAEN_DGTZ_InvalidEvent] = "The event is invalid.";
	errorMessageMap[CAEN_DGTZ_OutOfMemory] = "Out of memory.";
  errorMessageMap[CAEN_DGTZ_CalibrationError] = "Unable to calibrate the board.";
  errorMessageMap[CAEN_DGTZ_DigitizerNotFound] = "Unable to open the digitizer.";
  errorMessageMap[CAEN_DGTZ_DigitizerAlreadyOpen] = "The digitizer is already open.";
	errorMessageMap[CAEN_DGTZ_DigitizerNotReady] = "The digitizer is not ready to operate.";
  errorMessageMap[CAEN_DGTZ_InterruptNotConfigured] = "The digitizer is not configured for IRQ.";
  errorMessageMap[CAEN_DGTZ_DigitizerMemoryCorrupted] = "The digitizer flash memory is corrupted.";
  errorMessageMap[CAEN_DGTZ_DPPFirmwareNotSupported] = "The digitizer DPP firmware is not supported in this lib version.";
  errorMessageMap[CAEN_DGTZ_InvalidLicense] = "Invalid firmware license.";
  errorMessageMap[CAEN_DGTZ_InvalidDigitizerStatus] = "The digitizer is found in a corrupted status.";
  errorMessageMap[CAEN_DGTZ_UnsupportedTrace] = "The given trace is not supported by the digitizer.";
  errorMessageMap[CAEN_DGTZ_InvalidProbe] = "The given probe is not supported for the given digitizer's trace.";
  errorMessageMap[CAEN_DGTZ_NotYetImplemented] = "The function is not yet implemented.";

}

void DAQ::Try(char * name, CAEN_DGTZ_ErrorCode code, bool verbose) {
  if(code || verbose) {
    printf("%s: %s\n", name, errorMessageMap.find(code)->second);
  }
}

void DAQ::Require(char * name, CAEN_DGTZ_ErrorCode code, bool verbose) {
  if(code || verbose) {
    if(code) printf("\nFatal Error!\n");
    printf("%s: %s\n", name, errorMessageMap.find(code)->second);
    exit(0);
  }
}

void DAQ::ReadRegister(uint16_t reg, uint32_t * data, bool verbose) {
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_ReadRegister(DeviceHandle, reg, data);
	if(ret) {
		printf("Error reading from %x: %d\n", reg, errorMessageMap.find(ret)->second);
		return;
	}

	if(verbose) printf("Successfully read from %x: %d\n", reg, data);
}

void DAQ::WriteRegister(uint16_t reg, uint32_t * data, bool verbose) {
	CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_WriteRegister(DeviceHandle, reg, data);
	if(ret) {
		printf("Error writing to register %x: %s\n", reg, errorMessageMap.find(ret)->second);
		return;
	}

	uint32_t value = 0;
	CAEN_DGTZ_ReadRegister(DeviceHandle, reg, &value);
	if(value != data) {
		printf("Error writing to register %x: tried to write %d but reads %d\n", reg, data, value);
		return;
	}

	if(verbose) printf("Successfully wrote to %d to register %x\n", data, reg);
}

void DAQ::ConnectToBoard(RunConfiguration cfg) {

	Require("OpenDigitizer", CAEN_DGTZ_OpenDigitizer(cfg.LinkType,
																									 cfg.LinkNum,
																									 cfg.ConetNode,
																									 cfg.VMEBaseAddress,
																									 &DeviceHandle));

	CAEN_DGTZ_BoardInfo_t info;
	Require("GetInfo", CAEN_DGTZ_GetInfo(DeviceHandle, &info));

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

	// Causes calibrations to be lost?
	Try("Reset", CAEN_DGTZ_Reset(DeviceHandle));

	//mask=0; WRITE(Device,0x8010,&mask); //FPGAs reset
  //mask=0x80; WRITE(Device,0x8000,&mask); //Channel config, supposed to be set?

}

void DAQ::InitializeBoardParameters(RunConfiguration cfg) {

	uint32_t bitMask = 0;

	SetGroupEnableMask(cfg);
	SetRecordLength(cfg.RecordLength);
	SetTriggerDelay(cfg.TriggerDelay);
	SetCorrectionLevel(cfg.SAMCorrectionLevel);
	SetMaxNumEventsBLT(cfg.MaxNumEventsBLT);
	Try("SetAcquisitionMode", CAEN_DGTZ_SetAcquisitionMode(DeviceHandle, CAEN_DGTZ_SW_CONTROLLED));

	// Acquisition Control (0x8100)
	// bit[3] Trigger Counting Mode: 0=COUNT ONLY ACCEPTED TRIGGERS, 1=COUNT ALL TRIGGERS
	bitMask = (1 << 3);
	// bit[5] Memory Full Mode: 0=NORMAL, 1=ONE BUFFER free
	// value |= (1 << 5);
	WriteRegister(0x8100, bitMask);

	// Memory Buffer Almost Full Level (0x816C)
	// bit[10:0] LEVEL
	//bitMask = 0x1;
	//WriteRegister(0x816C, bitMask)

	SetAnalogMonitorOutput();

	SetChannelDCOffset(cfg);

	SetTriggerSource(cfg);
	for(int i = 0; i < nChannels; i++) SetTriggerThreshold(cfg.channels[i].triggerThreshold, i);

	// durp??
	CAEN_DGTZ_EnaDis_t enable;
  uint32_t vetoWindow;
  Try("GetSAMTriggerCountVetoParam", CAEN_DGTZ_GetSAMTriggerCountVetoParam(DeviceHandle, 0, &enable, &vetoWindow));
  printf("%d %d\n", enable, vetoWindow);
  vetoWindow = 125;
  Try("SetSAMTriggerCountVetoParam", CAEN_DGTZ_SetSAMTriggerCountVetoParam(DeviceHandle, 0, CAEN_DGTZ_DISABLE, vetoWindow));
  Try("GetSAMTriggerCountVetoParam", CAEN_DGTZ_GetSAMTriggerCountVetoParam(DeviceHandle, 0, &enable, &vetoWindow));
  printf("%d %d\n", enable, vetoWindow);

	useChargeMode = cfg.useChargeMode;
	useIRQ = cfg.useIRQ;

	SetSamplingFrequency(cfg.SAMFrequency);
	SetPulserParameters(cfg.channels);

	SetIOLevel(cfg.IOLevel);

	for(int i = 0; i < nChannels; i++) SetTriggerPolarity(cfg.channels[i].triggerPolarity, i);





	if(cfg.useIRQ) {
		Try("SetInterruptConfig", CAEN_DGTZ_SetInterruptConfig(DeviceHandle,
																 													 CAEN_DGTZ_ENABLE,
															   											 		 cfg.irqLevel,
															   											     cfg.irqStatusId,
															   											     cfg.irqNEvents,
															   											     cfg.irqMode));

		irqTimeout = cfg.irqTimeout;
	}

	if(cfg.useChargeMode) {

		Try("SetSAMAcquisitionMode", CAEN_DGTZ_SetSAMAcquisitionMode(DeviceHandle, CAEN_DGTZ_AcquisitionMode_DPP_CI));

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

		Try("SetDPPParameters", CAEN_DGTZ_SetDPPParameters(DeviceHandle, 0, &DPPParams));

	}

}

void DAQ::MallocReadoutBuffer() {
	Try("MallocReadoutBuffer", CAEN_DGTZ_MallocReadoutBuffer(DeviceHandle, (char**)&ReadoutBuffer, &MaxReadoutBufferSize));
	if(useChargeMode) Try("MallocDPPEvents", CAEN_DGTZ_MallocDPPEvents(DeviceHandle, (void**)DPPEvents, &DPPReadoutBufferSize));
}

void DAQ::SetGroupEnableMask(RunConfiguration cfg) {

	uint32_t groupMask = 0;

	for(int i = 0; i < nSamBlocks; i++) {
		if(cfg.channels[i].enable || cfg.channels[i+1].enable) groupMask += (1 << i);
	}

	Try("SetGroupEnableMask", CAEN_DGTZ_SetGroupEnableMask(DeviceHandle, groupMask));

}

void DAQ::StartRun() {

	if(!useChargeMode) Try("AllocateEvent", CAEN_DGTZ_AllocateEvent(DeviceHandle, (void**)&CurrentEvent));

	Try("ClearData", CAEN_DGTZ_ClearData(DeviceHandle));
	Try("SWStartAcquisition", CAEN_DGTZ_SWStartAcquisition(DeviceHandle));

}

void DAQ::StopRun() {

	uint32_t nReadBytes = 1;
	int error = 0;

	Try("SWStopAcquisition", CAEN_DGTZ_SWStopAcquisition(DeviceHandle));

	// continue reading data from the board until it's empty
	printf("Clearing data...\n");
	while (error == 0 && nReadBytes > 0) {
		error = CAEN_DGTZ_ReadData(DeviceHandle,
															 CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT,
															 (char*)ReadoutBuffer,
															 &nReadBytes);
	}

	if(!useChargeMode) Try("FreeEvent", CAEN_DGTZ_FreeEvent(DeviceHandle, (void**)&CurrentEvent));
	else Try("FreeDPPEvents", CAEN_DGTZ_FreeDPPEvents(DeviceHandle, (void**)DPPEvents));

}

void DAQ::PrepareEvent() {

	// if you shoud be sending software triggers, send one now
	if (TriggerType == SYSTEM_TRIGGER_AUTO || TriggerType == SYSTEM_TRIGGER_SOFT) Try("SendSWTrigger", CAEN_DGTZ_SendSWtrigger(DeviceHandle));

}

// Read an event from the board and store them in ReadoutBuffer and ReadoutBufferSize
int DAQ::ReadEventBuffer() {

	int error = CAEN_DGTZ_Success;

	if(useIRQ) {
		error = CAEN_DGTZ_IRQWait(DeviceHandle, irqTimeout); // timeout in ms
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
	Try("GetNumEvents", CAEN_DGTZ_GetNumEvents(DeviceHandle, (char*)ReadoutBuffer, ReadoutBufferSize, &numEvents));

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
		error = CAEN_DGTZ_DecodeEvent(DeviceHandle, eventPtr, (void**)&CurrentEvent);
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

	Try("FreeReadoutBuffer", CAEN_DGTZ_FreeReadoutBuffer((char**)&ReadoutBuffer));
	Try("CloseDigitizer", CAEN_DGTZ_CloseDigitizer(DeviceHandle));

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

	// durp: doesn't need to be set to all the same delay

	for(int i = 0; i < nSamBlocks; i++) {
		Try("SetSAMPostTriggerSize", CAEN_DGTZ_SetSAMPostTriggerSize(DeviceHandle, i, TriggerDelay));
	}

}

void DAQ::SetSamplingFrequency(CAEN_DGTZ_SAMFrequency_t SamplingFrequency) {
	Try("SetSAMSamplingFrequency", CAEN_DGTZ_SetSAMSamplingFrequency(DeviceHandle, SamplingFrequency));
}

void DAQ::SetPulserParameters(vector<ChannelConfiguration> channels) {

	for(int i = 0; i < nChannels; i++) {
		if(channels[i].testPulseEnable) Try("EnableSAMPulseGen", CAEN_DGTZ_EnableSAMPulseGen(DeviceHandle,
																																												 i,
																																												 channels[i].testPulsePattern,
																																												 channels[i].testPulseSource));
		else Try("DisableSAMPulseGen", CAEN_DGTZ_DisableSAMPulseGen(DeviceHandle, i));
	}

}

void DAQ::SetTriggerThreshold(double TriggerThreshold, int channel) {

	Try("SetChannelTriggerThreshold", CAEN_DGTZ_SetChannelTriggerThreshold(DeviceHandle,
																																				 channel,
																																				 DACValue(TriggerThreshold)));

}

void DAQ::SetTriggerPolarity(CAEN_DGTZ_TriggerPolarity_t TriggerPolarity, int channel) {

	Try("SetTriggerPolarity", CAEN_DGTZ_SetTriggerPolarity(DeviceHandle, channel, TriggerPolarity));

}

void DAQ::SetTriggerSource(RunConfiguration cfg) {

	TriggerType = cfg.TriggerType;

	if (TriggerType == SYSTEM_TRIGGER_SOFT) {
		Try("SetSWTriggerMode", CAEN_DGTZ_SetSWTriggerMode(DeviceHandle, CAEN_DGTZ_TRGMODE_ACQ_ONLY));
	}

	if (TriggerType == SYSTEM_TRIGGER_NORMAL || TriggerType == SYSTEM_TRIGGER_AUTO) {

		// First enable the trigger request generation from each group with an enabled channel trigger
		uint32_t bitMask = 0;
		for(int i = 0; i < nSamBlocks; i++) {
			if(cfg.channels[i].triggerEnable || cfg.channels[i+1].triggerEnable) bitMask += (1 << i);
		}
		WriteRegister(0x810C, &groupMask);

		// Secondly enable the self-trigger request generation from each group with an enabled channel trigger
		// FPGA self-trigger management registers (0x1n3C, n = group number 0-7)
		// bit[4] = ch0_trig_mask
		// bit[6] = ch1_trig_mask
		for(int i = 0; i < nSamBlocks; i++) {
			bitMask = 0x1;
			if(cfg.channels[i].triggerEnable) bitMask |= (1 << 4);
			if(cfg.channels[i+1].triggerEnable) bitMask |= (1 << 6);
			WriteRegister(0x103C + i*0x0100, bitMask);
		}

		// Testing? durp
		printf("Group 0 pulse is:\n");
		ReadRegister(0x102C, &bitMask, true);

	}

	if (TriggerType == SYSTEM_TRIGGER_AUTO || TriggerType == SYSTEM_TRIGGER_EXTERN) {

		Try("SetExtTriggerInputMode", CAEN_DGTZ_SetExtTriggerInputMode(DeviceHandle, CAEN_DGTZ_TRGMODE_ACQ_ONLY));

	}

}

void DAQ::SetIOLevel(CAEN_DGTZ_IOLevel_t level) {
	CAEN_DGTZ_SetIOLevel(DeviceHandle, level);
}

void DAQ::SetChannelDCOffset(RunConfiguration cfg) {

	for(int i = 0; i < nChannels; i++) CAEN_DGTZ_SetChannelDCOffset(DeviceHandle, i, DACValue(cfg.channels[i].dcOffset));

}

void DAQ::SetCorrectionLevel(CAEN_DGTZ_SAM_CORRECTION_LEVEL_t level) {
	CAEN_DGTZ_SetSAMCorrectionLevel(DeviceHandle, level);
}

void DAQ::SetMaxNumEventsBLT(int MaxNumEventsBLT) {
	CAEN_DGTZ_SetMaxNumEventsBLT(DeviceHandle, MaxNumEventsBLT);
}

void DAQ::SetRecordLength(uint32_t RecordLength) {
	Try("SetRecordLength", CAEN_DGTZ_SetRecordLength(DeviceHandle, RecordLength));
}

void DAQ::SetAnalogMonitorOutput() {

	// Analog Monitor Output mode:
	// 000 = Trigger Majority Mode
	// 001 = Test Mode
	// 010 = reserved
	// 011 = Buffer Occupancy Mode
	// 100 = Voltage Level Mode

	WriteRegister(0x8144, 0x3); // buffer occupancy
	WriteRegister(0x81B4, 0x4); // set buffer occupancy gain to 2^4 (x 1V/1024 = 0.976 mV per step)
}

#endif

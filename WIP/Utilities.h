#ifndef UTILITIES_H
#define UTILITIES_H

#include <map>

#include "V1743.h"
#include "CAENDigitizerTypes.h"

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

};

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

};

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

#endif

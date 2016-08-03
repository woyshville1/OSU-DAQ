#include "ChannelData.h"

//ClassImp(ChannelData)
/*
ChannelData::ChannelData() {
	Waveform.resize(numSamples, 0.);

}

void ChannelData::Copy(EventNode * node) {

	int i = 0;

	int samIndex = (int)(i / 2);
	int chanIndex = i % 2;

	GroupPresent = node->GrPresent[samIndex];
	ChannelSize = node->ChSize[samIndex];

	for (int j = 0; j < numSamples; j++) {
		Waveform[j] = node->Waveform[i][j];
	}

	TriggerCount = node->TriggerCount[samIndex][chanIndex];
	TimeCount = node->TimeCount[samIndex][chanIndex];
	EventId = node->EventId[samIndex];
	TDC = node->TDC[samIndex];
	PosEdgeTimeStamp = node->PosEdgeTimeStamp[samIndex];
	NegEdgeTimeStamp = node->NegEdgeTimeStamp[samIndex];
	PeakIndex = node->PeakIndex[samIndex];
	Peak = node->Peak[samIndex];
	Baseline = node->Baseline[samIndex];
	Charge = node->Charge[samIndex];

}
*/
#ifndef CHANNELDATA_H
#define CHANNELDATA_H
/*
//#include <vector>

//#include "TROOT.h"
#include "DAQ.h"
//#include "LinkDef.h"

using namespace std;

/*
class ChannelData {
public:
	ChannelData() {
		GroupPresent.resize(numChannels);
		ChannelSize.resize(numChannels);

		Waveform.clear();
		for (int i = 0; i < numChannels; i++) {
			vector<float> wave(numSamples, 0.);
			Waveform.push_back(wave);
		}

		TriggerCount.resize(numChannels);
		TimeCount.resize(numChannels);
		EventId.resize(numChannels);
		StartIndexCell.resize(numChannels);
		TDC.resize(numChannels);
		PosEdgeTimeStamp.resize(numChannels);
		NegEdgeTimeStamp.resize(numChannels);
		PeakIndex.resize(numChannels);
		Peak.resize(numChannels);
		Baseline.resize(numChannels);
		Charge.resize(numChannels);
	};
	virtual ~ChannelData() {
		GroupPresent.clear();
		ChannelSize.clear();

		for (unsigned int i = 0; i < Waveform.size(); i++) Waveform[i].clear();
		Waveform.clear();

		TriggerCount.clear();
		TimeCount.clear();
		EventId.clear();
		StartIndexCell.clear();
		TDC.clear();
		PosEdgeTimeStamp.clear();
		NegEdgeTimeStamp.clear();
		PeakIndex.clear();
		Peak.clear();
		Baseline.clear();
		Charge.clear();
	};

	void Copy(EventNode * node) {

		for (int i = 0; i < numChannels; i++) {

			int samIndex = (int)(i / 2);
			int chanIndex = i % 2;

			GroupPresent[i] = node->GrPresent[samIndex];
			ChannelSize[i] = node->ChSize[samIndex];

			for (int j = 0; j < numSamples; j++) {
				Waveform[i][j] = node->Waveform[i][j];
			}

			TriggerCount[i] = node->TriggerCount[samIndex][chanIndex];
			TimeCount[i] = node->TimeCount[samIndex][chanIndex];
			EventId[i] = node->EventId[samIndex];
			TDC[i] = node->TDC[samIndex];
			PosEdgeTimeStamp[i] = node->PosEdgeTimeStamp[samIndex];
			NegEdgeTimeStamp[i] = node->NegEdgeTimeStamp[samIndex];
			PeakIndex[i] = node->PeakIndex[samIndex];
			Peak[i] = node->Peak[samIndex];
			Baseline[i] = node->Baseline[samIndex];
			Charge[i] = node->Charge[samIndex];
		}

	};

	vector<unsigned char> GroupPresent; // uint8_t
	vector<unsigned int> ChannelSize; // uint32_t

	vector<vector<float> > Waveform;

	vector<unsigned short> TriggerCount; // uint16_t
	vector<unsigned short> TimeCount; // uint16_t
	vector<unsigned char> EventId; // uint8_t
	vector<unsigned short> StartIndexCell; // uint16_t
	vector<unsigned long long> TDC; // uint64_t
	vector<float> PosEdgeTimeStamp;
	vector<float> NegEdgeTimeStamp;
	vector<unsigned short> PeakIndex; // uint16_t
	vector<float> Peak;
	vector<float> Baseline;
	vector<float> Charge;

private:
	const int numChannels = 16;
	const int numSamples = 1024;

};

class ChannelData {
public:
	ChannelData();
	virtual ~ChannelData() {};

	void Copy(EventNode * node);

	unsigned char GroupPresent; // uint8_t
	unsigned int ChannelSize; // uint32_t

	float Waveform[numSamples];

	unsigned short TriggerCount; // uint16_t
	unsigned short TimeCount; // uint16_t
	unsigned char EventId; // uint8_t
	unsigned short StartIndexCell; // uint16_t
	unsigned long long TDC; // uint64_t
	float PosEdgeTimeStamp;
	float NegEdgeTimeStamp;
	unsigned short PeakIndex; // uint16_t
	float Peak;
	float Baseline;
	float Charge;

private:
	//const int numChannels = 16;
	const int numSamples = 1024;

};
*/
#endif
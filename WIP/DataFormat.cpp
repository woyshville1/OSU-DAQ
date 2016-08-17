#ifndef __DATAFORMAT_CPP
#define __DATAFORMAT_CPP

#include "DataFormat.h"

using namespace std;

DataFormat::DataFormat(const char * filename, int samples, bool chargeMode) {


	printf("\n\n\nNew DataFormat object!\n\n\n");
	
  numSamples = samples;
  useChargeMode = chargeMode;

  tree = new TTree("data", "Data");
  tree->SetAutoSave(10000000); // 10 MB
  DefineData();

  output = new TFile(filename, "RECREATE");

}

DataFormat::~DataFormat() {
  WriteData();
  output->Close();
  if(tree) delete tree;
  for(int i = 0; i < nChannels; i++) {
    if(_histograms[i]) delete _histograms[i];
  }
}

void DataFormat::DefineData() {

  TString branchName, name, title;
  TString channelName;
  for(int i = 0; i < nChannels; i++) {

    channelName = Form("%d", i);

    if(useChargeMode) tree->Branch("charge_" + channelName, &_charge[i]);

    else {
      _histograms[i] = new TH1D("trace_" + channelName, "Trace of channel " + channelName, numSamples, 0, numSamples);
      tree->Branch("channel_" + channelName, &(_histograms[i]));

      tree->Branch("triggerCount_" + channelName, &_trigCount[i]);
      tree->Branch("timeCount_" + channelName, &_timeCount[i]);
      tree->Branch("TDC_" + channelName, &_tdc[i]);

      tree->Branch("min_" + channelName, &_min[i]);
      tree->Branch("max_" + channelName, &_max[i]);
    }

  }

}

void DataFormat::AddEvent(EventNode * node) {

  if(useChargeMode) {

    for(int event = 0; event < node->NumEvents[0]; event++) {
      for(int ch = 0; ch < nChannels; ch++) {
        _charge[ch] = node->Charge[ch][event];
      }
      tree->Fill();
    }

  }

  else {

    for(int i = 0; i < nChannels; i++) {

  	  int groupIndex = (int)(i / 2);
  	  int channelIndex = i % 2;

      _trigCount[i] = node->TriggerCount[groupIndex][channelIndex];
  	  _timeCount[i] = node->TimeCount[groupIndex][channelIndex];
  	  _tdc[i] = node->TDC[groupIndex];

      _min[i] = 1.e6;
      _max[i] = -1.e6;

      for(int j = 0; j < numSamples; j++) {
        _histograms[i]->SetBinContent(j+1, node->Waveform[i][j] * ADCTOVOLTS);
        if(node->Waveform[i][j] * ADCTOVOLTS < _min[i]) _min[i] = node->Waveform[i][j] * ADCTOVOLTS;
        if(node->Waveform[i][j] * ADCTOVOLTS > _max[i]) _max[i] = node->Waveform[i][j] * ADCTOVOLTS;
    	}

    } // for channels

    tree->Fill();

  } // if not charge mode

}

void DataFormat::WriteData() {

  tree->Write();
  output->Close();

}

#endif
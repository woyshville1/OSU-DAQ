#include "DataFormat.h"

using namespace std;

DataFormat::DataFormat(char * filename, int samples) {

  numSamples = samples;

  tree = new TTree("data", "Data");
  tree->SetAutoSave(10000000); // 10 MB
  DefineData();

  output = new TFile(filename, "RECREATE");

}

DataFormat::~DataFormat() {
  WriteData();
  output->Close();
  if(data) delete data;
  for(int i = 0; i < numChannels; i++) {
    if(histograms[i]) delete histograms[i];
  }
}

void DataFormat::DefineData() {

  TString branchName, name, title;
  TString channelName;
  for(int i = 0; i < numChannels; i++) {

    channelName = Form("%d", i);

    if(useChargeMode) tree->Branch("charge_" + channelName, &charge[i]);

    else {
      histograms[i] = new TH1D("trace_" + channelName, "Trace of channel " + channelName, numSamples, 0, numSamples);
      tree->Branch("channel_" + channelName, &(histograms[i]));

      tree->Branch("triggerCount_" + channelName, &trigCount[i]);
      tree->Branch("timeCount_" + channelName, &timeCount[i]);
      tree->Branch("TDC_" + channelName, &tdc[i]);

      tree->Branch("min", &min[i]);
      tree->Branch("max", &max[i]);
    }

  }

}

void DataFormat::AddEvent(EventNode * node) {

  if(useChargeMode) {

    for(int event = 0; event < node->NumEvents[0]; i++) {
      for(int ch = 0; ch < numChannels; ch++) {
        charge[i] = node->Charge[ch][ev];
      }
      tree->Fill();
    }

  }

  else {

    for(int i = 0; i < nChannels; i++) {

  	  int groupIndex = (int)(i / 2);
  	  int channelIndex = i % 2;

      trigCount[i] = node->TriggerCount[samIndex][chanIndex];
  	  timeCount[i] = node->TimeCount[samIndex][chanIndex];
  	  tdc[i] = node->TDC[samIndex];

      min[i] = 1.e6;
      max[i] = -1.e6;

      for(int j = 0; j < numSamples; j++) {
        histograms[i]->SetBinContent(i+1, node->Waveform[i][j] * ADCTOVOLTS);
        if(node->Waveform[i][j] * ADCTOVOLTS < min[i]) min[i] = node->Waveform[i][j] * ADCTOVOLTS;
        if(node->Waveform[i][j] * ADCTOVOLTS > max[i]) max[i] = node->Waveform[i][j] * ADCTOVOLTS;
    	}

    } // for channels

    tree->Fill();

  } // if not charge mode

}

void DataFormat::WriteData() {

  tree->Write();
  output->Close();

}

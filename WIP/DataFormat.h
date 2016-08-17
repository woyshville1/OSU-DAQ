#ifndef __DATAFORMAT_H
#define __DATAFORMAT_H

#include "CAENDigitizer.h"
#include "TH1D.h"
#include "TTree.h"
#include "TFile.h"

#include "DAQ.h"

class DataFormat {

public:
  DataFormat(const char * filename, int samples, bool chargeMode);
  virtual ~DataFormat();

  void AddEvent(EventNode * node);

private:

  TFile * output;
  void DefineData();
  void WriteData();

  TTree * tree;
  TH1D * _histograms[nChannels];

  float _charge[nChannels];
  uint16_t _timeCount[nChannels];
  uint16_t _trigCount[nChannels];
  uint64_t _tdc[nChannels];
  float _min[nChannels];
  float _max[nChannels];

  int numSamples;
  bool useChargeMode;

};

#endif
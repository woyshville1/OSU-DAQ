#include "CAENDigitizer.h"
#include "TH1D.h"
#include "TTree.h"
#include "TFile.h"

#include "DAQ.h"

class DataFormat {

public:
  DataFormat(char * filename, int samples);
  virtual ~DataFormat();

protected:
  TFile * output;
  void DefineData();
  void AddEvent(EventNode * node);
  void WriteData();

private:

  TTree * tree;
  TH1D * histograms[numChannels];

  int numSamples;

  const int numChannels = 16;

};

#ifndef __RUNCONFIGURATION_H
#define __RUNCONFIGURATION_H

#include <iostream>
#include <fstream>
#include <tuple>
#include <vector>

#include "CAENDigitizer.h"

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;

typedef enum {
  SYSTEM_TRIGGER_SOFT,
  SYSTEM_TRIGGER_NORMAL,
  SYSTEM_TRIGGER_AUTO,
  SYSTEM_TRIGGER_EXTERN
} TriggerType_t;

struct GroupConfiguration {
  CAEN_DGTZ_TrigerLogic_t logic;
  uint16_t coincidenceWindow;

  bool modified;
};

struct ChannelConfiguration {
  bool enable;

  bool testPulseEnable;
  unsigned short testPulsePattern;
  CAEN_DGTZ_SAMPulseSourceType_t testPulseSource;

  bool triggerEnable;
  double triggerThreshold;
  CAEN_DGTZ_TriggerPolarity_t triggerPolarity;

  double dcOffset;

  unsigned int chargeStartCell;
  unsigned short chargeLength;
  CAEN_DGTZ_EnaDis_t enableChargeThreshold;
  float chargeThreshold;

  bool modified;
};

class RunConfiguration {

 public:

  RunConfiguration() {
    groups.resize(numGroups);
    for(int i = 0; i < numGroups; i++) groups[i].modified = false;

    channels.resize(numChannels);
    for(int i = 0; i < numChannels; i++) channels[i].modified = false;

    numCommonParametersRead = 0;
  };

  virtual ~RunConfiguration() {
    groups.clear();
    channels.clear();
  };

  bool ParseConfigFile(string filename);

  bool CheckAllParametersSet() {
    bool value = true;

    for(int i = 0; i < numGroups; i++) value &= groups[i].modified;
    for(int i = 0; i < numChannels; i++) value &= channels[i].modified;

    value &= (numCommonParametersRead == numExpectedCommonParameters);

    return value;
  }

  ////////////////////////////////

  CAEN_DGTZ_ConnectionType LinkType;
  int LinkNum;
  int ConetNode;
  uint32_t VMEBaseAddress;

  bool useIRQ;
  uint8_t irqLevel;
  uint32_t irqStatusId;
  uint16_t irqNEvents;
  CAEN_DGTZ_IRQMode_t irqMode;
  uint32_t irqTimeout;

  bool useChargeMode;
  CAEN_DGTZ_EnaDis_t suppressChargeBaseline;

  CAEN_DGTZ_SAM_CORRECTION_LEVEL_t SAMCorrectionLevel;

  CAEN_DGTZ_SAMFrequency_t SAMFrequency;

  uint32_t RecordLength;

  uint8_t TriggerDelay;

  TriggerType_t TriggerType;

  CAEN_DGTZ_IOLevel_t IOLevel;

  uint32_t MaxNumEventsBLT;

  CAEN_DGTZ_TrigerLogic_t GroupTriggerLogic;
  uint32_t GroupTriggerMajorityLevel;

  vector<GroupConfiguration> groups;
  vector<ChannelConfiguration> channels;

 private:
  const int numChannels = 16;
  const int numGroups = 8;
  const int numExpectedCommonParameters = 11;

  int numCommonParametersRead;
};

#endif

#ifndef RUNCONFIGURATION_CPP
#define RUNCONFIGURATION_CPP

#include "RunConfiguration.h"

int RunConfiguration::ParseConfigFile(string filename) {
  printf("reading %s\n", filename.c_str());
  using namespace boost;
  using namespace property_tree;
  ptree pt;

  try {
    read_xml(filename, pt);
  }
  catch (const ptree_error &e) {
    printf("%s\n", e.what());
  }

  string str_;

  for (const auto& i : pt.get_child("config.common")) {
    string name;
    ptree sub_pt;
    tie(name, sub_pt) = i;

    if (name == "connectionType") {
      LinkNum = sub_pt.get<int>("<xmlattr>.linkNum");
      ConetNode = sub_pt.get<int>("<xmlattr>.conetNode");
      VMEBaseAddress = sub_pt.get<uint32_t>("<xmlattr>.vmeBaseAddress");

      str_ = sub_pt.get<string>("<xmlattr>.type");
      if (str_ == "opticalLink") LinkType = CAEN_DGTZ_OpticalLink;
      else if (str_ == "USB") LinkType = CAEN_DGTZ_USB;
      else {
	cout << "Invalid connection type!" << endl;
	return -1;
      }

      numCommonParametersRead++;
    }

    if (name == "IRQ") {
      useIRQ = sub_pt.get<bool>("<xmlattr>.use");
      irqLevel = sub_pt.get<uint8_t>("<xmlattr>.level");
      irqStatusId = sub_pt.get<uint32_t>("<xmlattr>.status_id");
      irqNEvents = sub_pt.get<uint16_t>("<xmlattr>.nevents");
      irqTimeout = sub_pt.get<uint32_t>("<xmlattr>.timeout");

      str_ = sub_pt.get<string>("<xmlattr>.mode");
      if (str_ == "RORA") irqMode = CAEN_DGTZ_IRQ_MODE_RORA;
      else if (str_ == "ROAK") irqMode = CAEN_DGTZ_IRQ_MODE_ROAK;
      else {
	cout << "Invalid IRQ mode!" << endl;
	return -1;
      }

      numCommonParametersRead++;
    }

    if (name == "chargeMode") {
      useChargeMode = sub_pt.get<bool>("<xmlattr>.use");

      str_ = sub_pt.get<string>("<xmlattr>.suppressBaseline");
      if (str_ == "1") suppressChargeBaseline = CAEN_DGTZ_ENABLE;
      else if (str_ == "0") suppressChargeBaseline = CAEN_DGTZ_DISABLE;
      else {
	cout << "Invalid value for supressChargeBaseline!" << endl;
	return -1;
      }

      numCommonParametersRead++;
    }

    if (name == "samCorrection") {
      str_ = sub_pt.get<string>("<xmlattr>.level");
      if (str_ == "all") SAMCorrectionLevel = CAEN_DGTZ_SAM_CORRECTION_ALL;
      else if (str_ == "INL") SAMCorrectionLevel = CAEN_DGTZ_SAM_CORRECTION_INL;
      else if (str_ == "pedestalOnly") SAMCorrectionLevel = CAEN_DGTZ_SAM_CORRECTION_PEDESTAL_ONLY;
      else if (str_ == "correctionDisabled") SAMCorrectionLevel = CAEN_DGTZ_SAM_CORRECTION_DISABLED;
      else {
	cout << "Invalid SAM correction level!" << endl;
	return -1;
      }

      numCommonParametersRead++;
    }

    if (name == "samFrequency") {
      str_ = sub_pt.get<string>("<xmlattr>.rate");
      if (str_ == "3.2") SAMFrequency = CAEN_DGTZ_SAM_3_2GHz;
      else if (str_ == "1.6") SAMFrequency = CAEN_DGTZ_SAM_1_6GHz;
      else if (str_ == "0.8") SAMFrequency = CAEN_DGTZ_SAM_800MHz;
      else if (str_ == "0.4") SAMFrequency = CAEN_DGTZ_SAM_400MHz;
      else {
	cout << "Invalid SAM frequency!" << endl;
	return -1;
      }

      numCommonParametersRead++;
    }

    if (name == "recordLength") {
      RecordLength = sub_pt.get<unsigned int>("<xmlattr>.length");
      if (RecordLength % 16 != 0 || RecordLength < 4 * 16 || RecordLength <= 0) {
	cout << "Invalid record length!" << endl;
	return -1;
      }

      numCommonParametersRead++;
    }

    if (name == "triggerDelay") {
      TriggerDelay = sub_pt.get<unsigned char>("<xmlattr>.delay");
      if ((uint32_t)TriggerDelay >= RecordLength) {
	cout << "Invalid trigger delay!" << endl;
	return -1;
      }

      numCommonParametersRead++;
    }

    if (name == "triggerType") {
      str_ = sub_pt.get<string>("<xmlattr>.type");
      if (str_ == "software") TriggerType = SYSTEM_TRIGGER_SOFT;
      else if (str_ == "normal") TriggerType = SYSTEM_TRIGGER_NORMAL;
      else if (str_ == "auto") TriggerType = SYSTEM_TRIGGER_AUTO;
      else if (str_ == "external") TriggerType = SYSTEM_TRIGGER_EXTERN;
      else {
	cout << "Invalid trigger type!" << endl;
	return -1;
      }

      numCommonParametersRead++;
    }

    if (name == "ioLevel") {
      str_ = sub_pt.get<string>("<xmlattr>.type");
      if (str_ == "TTL") IOLevel = CAEN_DGTZ_IOLevel_TTL;
      else if (str_ == "NIM") IOLevel = CAEN_DGTZ_IOLevel_NIM;
      else {
	cout << "Invalid IO level!" << endl;
	return -1;
      }

      numCommonParametersRead++;
    }

    if (name == "maxNumEventsBLT") {
      MaxNumEventsBLT = sub_pt.get<uint32_t>("<xmlattr>.number");
      numCommonParametersRead++;
    }

    if (name == "groupTriggerLogic") {
      GroupTriggerMajorityLevel = sub_pt.get<uint32_t>("<xmlattr>.majorityLevel");

      str_ = sub_pt.get<string>("<xmlattr>.logic");
      if (str_ == "or") GroupTriggerLogic = CAEN_DGTZ_LOGIC_OR;
      else if (str_ == "and") GroupTriggerLogic = CAEN_DGTZ_LOGIC_AND;
      else if (str_ == "majority") GroupTriggerLogic = CAEN_DGTZ_LOGIC_MAJORITY;
      else {
	cout << "Invalid group trigger logic!" << endl;
	return -1;
      }

      numCommonParametersRead++;
    }

  }

  for (const auto& i : pt.get_child("config.groups")) {

    string name;
    ptree sub_pt;
    tie(name, sub_pt) = i;

    if (name != "group") continue;

    int groupNumber = sub_pt.get<int>("<xmlattr>.number");
    if (groupNumber > numGroups) continue;

    uint16_t gate = sub_pt.get<unsigned short>("<xmlattr>.coincidenceWindow");
		
    CAEN_DGTZ_TrigerLogic_t logic_;
    str_ = sub_pt.get<string>("<xmlattr>.logic");
    if (str_ == "or") logic_ = CAEN_DGTZ_LOGIC_OR;
    else if (str_ == "and") logic_ = CAEN_DGTZ_LOGIC_AND;
    else if (str_ == "majority") logic_ = CAEN_DGTZ_LOGIC_MAJORITY;
    else continue;

    if (groupNumber < 0) {
      for (int j = 0; j < numGroups; j++) {
	groups[j].logic = logic_;
	groups[j].coincidenceWindow = gate;
	groups[j].modified = true;
      }
    }

    else {
      groups[groupNumber].logic = logic_;
      groups[groupNumber].coincidenceWindow = gate;
      groups[groupNumber].modified = true;
    }

  }

  for (const auto& i : pt.get_child("config.channels")) {

    string name;
    ptree sub_pt;
    tie(name, sub_pt) = i;

    if (name != "channel") continue;

    int channelNumber = sub_pt.get<int>("<xmlattr>.number");
    if (channelNumber > numChannels) continue;

    ChannelConfiguration ch;

    ch.enable = sub_pt.get<bool>("<xmlattr>.channelEnable");

    ch.testPulseEnable = sub_pt.get<bool>("<xmlattr>.testPulseEnable");
    ch.testPulsePattern = stoi(sub_pt.get<string>("<xmlattr>.testPulsePattern"), 0, 16);

    str_ = sub_pt.get<string>("<xmlattr>.testPulseSource");
    if (str_ == "continuous") ch.testPulseSource = CAEN_DGTZ_SAMPulseCont;
    else if (str_ == "software") ch.testPulseSource = CAEN_DGTZ_SAMPulseSoftware;
    else continue;

    ch.triggerEnable = sub_pt.get<bool>("<xmlattr>.triggerEnable");
    ch.triggerThreshold = sub_pt.get<double>("<xmlattr>.triggerThreshold");

    str_ = sub_pt.get<string>("<xmlattr>.triggerPolarity");
    if (str_ == "risingEdge") ch.triggerPolarity = CAEN_DGTZ_TriggerOnRisingEdge;
    else if (str_ == "fallingEdge") ch.triggerPolarity = CAEN_DGTZ_TriggerOnFallingEdge;
    else continue;

    ch.dcOffset = sub_pt.get<double>("<xmlattr>.dcOffset");

    ch.chargeStartCell = sub_pt.get<unsigned int>("<xmlattr>.chargeStartCell");
    ch.chargeLength = sub_pt.get<unsigned short>("<xmlattr>.chargeLength");

    str_ = sub_pt.get<string>("<xmlattr>.enableChargeThreshold");
    if (str_ == "1") ch.enableChargeThreshold = CAEN_DGTZ_ENABLE;
    else if (str_ == "0") ch.enableChargeThreshold = CAEN_DGTZ_DISABLE;
    else continue;

    ch.chargeThreshold = sub_pt.get<float>("<xmlattr>.chargeThreshold");

    ch.modified = true;

    if (channelNumber < 0) {
      for (int j = 0; j < numChannels; j++) channels[j] = ch;
    }

    else channels[channelNumber] = ch;

  }

  return 0;

}

#endif

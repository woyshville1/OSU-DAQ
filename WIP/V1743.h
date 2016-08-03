#ifndef V1743_H
#define V1743_H

#define nChannels            16
#define nSamBlocks           8
#define nChannelsPerSamBlock 2

#define MIN_DAC_RAW_VALUE -1.25
#define MAX_DAC_RAW_VALUE 1.25

#define ADCTOVOLTS 0.61

int DACValue(double value) { return (int)((MAX_DAC_RAW_VALUE - value) / (MAX_DAC_RAW_VALUE - MIN_DAC_RAW_VALUE) * 0xFFFF); };

#endif

#ifndef LowPassFilter_H
#define LowPassFilter_H

#include <vector>
#include <JuceHeader.h>

class LowPassFilter {
public:
    // Constructor
    LowPassFilter();


    void setHighpass(bool highpass);
    void setCutoffFrequency(float cutoffFrequency);
    void setSamplingRate(float samplingRate);
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&);

private:
    bool highpass;
    float cutoffFrequency; // Cutoff frequency in Hz
    float samplingRate; // Sampling rate in Hz

    std::vector<float> dnBuffer; // Buffers for processing per channel
    std::vector<float> dnBuffer2;
};

#endif // LowPassFilter_H

#pragma once
#include <JuceHeader.h>

class FrequencyBands {
public:
    FrequencyBands();
    ~FrequencyBands() = default;

    void prepare(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer, const juce::MidiBuffer&);

    void setBassGain(float gain) { bassGain = gain; }
    void setMidsGain(float gain) { midsGain = gain; }
    void setTopsGain(float gain) { topsGain = gain; }

private:
    // Filters
    juce::IIRFilter bassLowPass, midsBandPass, topsHighPass;

    // Gains
    float bassGain = 1.0f, midsGain = 1.0f, topsGain = 1.0f;

    // Sample rate
    double currentSampleRate = 44100.0;
};

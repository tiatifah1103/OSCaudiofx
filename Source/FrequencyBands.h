#pragma once
#include <JuceHeader.h>
#include "LowPassFilter.h"
#include "MainComponent.h"

class FrequencyBands
{
public:
    FrequencyBands();
    ~FrequencyBands();

    void prepare(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midiBuffer);

    // Setters for filter parameters
    void setBassCutoff(float frequency);
    void setMidsCutoff(float frequency);
    void setTopsCutoff(float frequency);

    void setBassResonance(float resonance);
    void setMidsResonance(float resonance);
    void setTopsResonance(float resonance);

private:
    // Filters for bass, mids, and tops
    LowPassFilter bassFilter;  
    LowPassFilter midsFilter;
    LowPassFilter topsFilter;

    float sampleRate = 44100.0f;  // Default sample rate
};

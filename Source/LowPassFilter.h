#pragma once

#include <JuceHeader.h>

class LowPassFilter
{
public:
    LowPassFilter();

    void prepare(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);
    void setCutoffFrequency(float newCutoff);

private:
    float cutoffFrequency;
    double sampleRate; // Stores sample rate for coefficient calculations

    //ProcessorDuplicator to apply the filter to both channels
    using Filter = juce::dsp::IIR::Filter<float>;
    using DuplicatedFilter = juce::dsp::ProcessorDuplicator<Filter, juce::dsp::IIR::Coefficients<float>>;

    DuplicatedFilter filter;
};

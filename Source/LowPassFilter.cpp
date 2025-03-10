#include "LowPassFilter.h"

LowPassFilter::LowPassFilter() : cutoffFrequency(1000.0f) {} // Initialises cutoff

void LowPassFilter::prepare(double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate; // Stores sample rate

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 2; // Sets to stereo

    filter.prepare(spec);

    DBG("Initial cutoff frequency: " << cutoffFrequency);

    setCutoffFrequency(cutoffFrequency); // Apply the initial cutoff
}

void LowPassFilter::process(juce::AudioBuffer<float>& buffer)
{
    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing<float> context(block);
    filter.process(context);
}

void LowPassFilter::setCutoffFrequency(float newCutoff)
{
    cutoffFrequency = newCutoff;

    // Create new coefficients for the current sample rate
    auto newCoefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, cutoffFrequency);

    // Apply the new coefficients to both channels - stereo not mono processing
    *filter.state = *newCoefficients;
}

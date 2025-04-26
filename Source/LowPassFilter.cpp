#include "LowPassFilter.h"

// Constructor to initialise default values
LowPassFilter::LowPassFilter() {
    highpass = false;
    cutoffFrequency = 100.0f;
    samplingRate = 44100.0f;
    
}

// Sets high-pass / low-pass mode
void LowPassFilter::setHighpass(bool highpass) {
    this->highpass = highpass;
}

// Sets the cutoff frequency
void LowPassFilter::setCutoffFrequency(float cutoffFrequency) {
    this->cutoffFrequency = cutoffFrequency;
}

// Sets the sampling rate
void LowPassFilter::setSamplingRate(float samplingRate) {
    this->samplingRate = samplingRate;
}

// Processes the buffer with the low-pass filter
void LowPassFilter::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    constexpr auto PI = 3.14159265359f;

    dnBuffer.resize(buffer.getNumChannels(), 0.f);
    dnBuffer2.resize(buffer.getNumChannels(), 0.f); // Second-stage buffer

    const auto sign = highpass ? -1.f : 1.f;
    const auto tanVal = std::tan(PI * cutoffFrequency / samplingRate);
    const auto a1 = (tanVal - 1.f) / (tanVal + 1.f);

    //Applies a gain  after filtering
    const float gainBoost = 7.0f;

    for (auto channel = 0; channel < buffer.getNumChannels(); ++channel) {
        auto* channelSamples = buffer.getWritePointer(channel);

        for (auto i = 0; i < buffer.getNumSamples(); ++i) {
            const auto inputSample = channelSamples[i];

            // First filter stage
            const auto allpassFilteredSample1 = a1 * inputSample + dnBuffer[channel];
            dnBuffer[channel] = inputSample - a1 * allpassFilteredSample1;
            const auto filterOutput1 = 0.5f * (inputSample + sign * allpassFilteredSample1);

            // Second filter stage for stronger cutoff
            const auto allpassFilteredSample2 = a1 * filterOutput1 + dnBuffer2[channel];
            dnBuffer2[channel] = filterOutput1 - a1 * allpassFilteredSample2;
            const auto filterOutput2 = 0.5f * (filterOutput1 + sign * allpassFilteredSample2);

            //Applies gain boost
            channelSamples[i] = filterOutput2 * gainBoost;
        }
    }
}




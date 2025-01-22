/*
  ==============================================================================

    DelayEffect.cpp
    Created: 13 Dec 2024 3:27:36pm
    Author:  Latifah Dickson

  ==============================================================================
*/

#include <JuceHeader.h>
#include "DelayEffect.h"

//==============================================================================
DelayEffect::DelayEffect()
{
    delayBuffer.setSize(2, 44100 * 2); // Stereo, 2 seconds max delay
    delayBuffer.clear();

}

DelayEffect::~DelayEffect()
{
}

void DelayEffect::process(const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto numSamples = bufferToFill.numSamples;
    auto numChannels = bufferToFill.buffer->getNumChannels();

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = bufferToFill.buffer->getWritePointer(channel);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            auto inSample = channelData[sample];
            auto delayedSample = delayBuffer.getSample(channel, (writePosition + sample) % delayBuffer.getNumSamples());

            auto outSample = inSample + delayedSample * mix;
            channelData[sample] = outSample;

            delayBuffer.setSample(channel, (writePosition + sample) % delayBuffer.getNumSamples(), inSample + delayedSample * feedback);
        }
    }

    writePosition = (writePosition + numSamples) % delayBuffer.getNumSamples();
}

void DelayEffect::setDelayTime(int newDelayTimeMs)
{
    delayTimeSamples = newDelayTimeMs * 44.1; // Convert ms to samples
}

void DelayEffect::setFeedback(float newFeedback)
{
    feedback = newFeedback;
}

void DelayEffect::setMix(float newMix)
{
    mix = newMix;
}

void DelayEffect::prepare(double sampleRate, int samplesPerBlock)
{
    // Initialize delay buffer with max size, for 2 seconds of delay at 44.1kHz sample rate)
    delayBuffer.setSize(2, (int)(sampleRate * 2));  // 2 seconds of delay for stereo (2 channels)
    delayBuffer.clear();  // Clears any existing data in the buffer
    writePosition = 0;    // Resets the write position in the delay buffer
}

void DelayEffect::resized()
{


}

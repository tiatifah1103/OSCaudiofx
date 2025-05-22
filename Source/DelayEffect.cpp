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

// Main audio processing function
void DelayEffect::process(const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto numSamples = bufferToFill.numSamples;
    auto numChannels = bufferToFill.buffer->getNumChannels();

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* channelData = bufferToFill.buffer->getWritePointer(channel);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Reads input sample from audio stream
            auto inSample = channelData[sample];

            // Reads delayed sample from delay buffer
            auto delayedSample = delayBuffer.getSample(channel, (writePosition + sample) % delayBuffer.getNumSamples());

            // Mixes original signal and delayed signal
            auto outSample = inSample + delayedSample * mix;

            // Outputs mixed sample
            channelData[sample] = outSample;

            // Writes to delay buffer with feedback
            delayBuffer.setSample(channel, (writePosition + sample) % delayBuffer.getNumSamples(), inSample + delayedSample * feedback);
        }
    }

    // Moves the write position forward, wrapping around the buffer length
    writePosition = (writePosition + numSamples) % delayBuffer.getNumSamples();
}

// Sets delay time in milliseconds
void DelayEffect::setDelayTime(int newDelayTimeMs)
{
    // Converts delay time from milliseconds to samples at 44.1kHz
    delayTimeSamples = newDelayTimeMs * 44.1;
}

// Sets feedback amount
void DelayEffect::setFeedback(float newFeedback)
{
    feedback = newFeedback;
}

// Sets mix amount between dry and delayed signal
void DelayEffect::setMix(float newMix)
{
    mix = newMix;
}

// Prepares the delay buffer before playback

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

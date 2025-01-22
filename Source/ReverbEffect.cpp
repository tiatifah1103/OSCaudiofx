/*
  ==============================================================================

    ReverbEffect.cpp
    Created: 15 Dec 2024 4:34:12pm
    Author:  Latifah Dickson

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ReverbEffect.h"

//==============================================================================
ReverbEffect::ReverbEffect()
{
    // Initialize default reverb parameters
    reverbParams.roomSize = 0.5f;   // Medium room size
    reverbParams.damping = 0.5f;   // Medium damping
    reverbParams.wetLevel = 0.3f;  // Moderate reverb in the mix
    reverbParams.dryLevel = 0.7f;  // More dry than wet
    reverbParams.width = 1.0f;     // Full stereo width
    reverbParams.freezeMode = 0;   // Freeze mode off

    reverb.setParameters(reverbParams);

}

ReverbEffect::~ReverbEffect()
{
}

void ReverbEffect::prepare(double sampleRate, int samplesPerBlock)
{
    // Prepare the reverb processor
    reverb.setSampleRate(sampleRate);
}

void ReverbEffect::process(juce::AudioBuffer<float>& buffer)
{

    reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), buffer.getNumSamples());
}

void ReverbEffect::setRoomSize(float size)
{
    reverbParams.roomSize = juce::jlimit(0.0f, 1.0f, size);
    reverb.setParameters(reverbParams);
}

void ReverbEffect::setDamping(float damping)
{
    reverbParams.damping = juce::jlimit(0.0f, 1.0f, damping);
    reverb.setParameters(reverbParams);
}

void ReverbEffect::setWetLevel(float wet)
{
    reverbParams.wetLevel = juce::jlimit(0.0f, 1.0f, wet);
    reverb.setParameters(reverbParams);
}

void ReverbEffect::setDryLevel(float dry)
{
    reverbParams.dryLevel = juce::jlimit(0.0f, 1.0f, dry);
    reverb.setParameters(reverbParams);
}

void ReverbEffect::setWidth(float width)
{
    reverbParams.width = juce::jlimit(0.0f, 1.0f, width);
    reverb.setParameters(reverbParams);
}

void ReverbEffect::setFreezeMode(bool freeze)
{
    reverbParams.freezeMode = freeze ? 1.0f : 0.0f;
    reverb.setParameters(reverbParams);
}

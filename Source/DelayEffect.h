/*
  ==============================================================================

    DelayEffect.h
    Created: 13 Dec 2024 3:27:36pm
    Author:  Latifah Dickson

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/

class DelayEffect  : public juce::Component
{
    

public:
    DelayEffect();
    ~DelayEffect();

    void prepare(double sampleRate, int samplesPerBlock); 
    void process(const juce::AudioSourceChannelInfo& bufferToFill);

    void setDelayTime(int newDelayTimeMs);
    void setFeedback(float newFeedback);
    void setMix(float newMix);
    
    void resized();

private:
    juce::AudioBuffer<float> delayBuffer;
    int writePosition = 0;
    int delayTimeSamples = 0;
    float feedback = 0.5f;
    float mix = 0.5f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayEffect)
};

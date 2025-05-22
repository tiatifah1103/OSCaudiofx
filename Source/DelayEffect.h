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
    bool isActive() const {
        return (mix > 0.001f) || (feedback > 0.001f) || (delayTimeSamples > 44);
    }

private:
    juce::AudioBuffer<float> delayBuffer;
    int writePosition = 0;
    int delayTimeSamples = 0;
    float feedback = 0.0f;
    float mix = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayEffect)
};


#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class ReverbEffect  : public juce::Component
{
public:
    ReverbEffect();
    ~ReverbEffect() override;
    
    // Prepare the reverb for audio processing
        void prepare(double sampleRate, int samplesPerBlock);

        // Process audio buffer with reverb
        void process(juce::AudioBuffer<float>& buffer);

        // Set reverb parameters
        void setRoomSize(float size);
        void setDamping(float damping);
        void setWetLevel(float wet);
        void setDryLevel(float dry);
        void setWidth(float width);
        void setFreezeMode(bool freeze);

private:
    
    juce::Reverb reverb;   // JUCE's built-in reverb object
    juce::Reverb::Parameters reverbParams;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbEffect)
};

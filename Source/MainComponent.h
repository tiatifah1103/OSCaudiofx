#pragma once

#include <JuceHeader.h>
#include "MainComponent.h"
#include "DelayEffect.h"
#include "ReverbEffect.h"
#include <juce_osc/juce_osc.h>
#include "LowPassFilter.h"
#include "FrequencyBands.h"

class MainComponent  : public juce::AudioAppComponent, public juce::MidiInputCallback

{
public:
    //==============================================================================
    MainComponent();
        ~MainComponent();

        void paint(juce::Graphics&) override;
        void resized() override;
    // Helper methods for file handling
    juce::File getAssetsFolder();
        void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
        void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
        void releaseResources() override;

        void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;
    void togglePlayPause();
        void nextTrack();
    void handleVolumeControl(int controllerValue);
    void loadAudioFile(const juce::File& file);
    void listAudioDevices();
    void playNextTrack();

    

    private:
        //==============================================================================
        juce::AudioTransportSource audioTransportSource;
        juce::AudioFormatManager formatManager;

        juce::StringArray playlistFiles;
        juce::StringArray trackTitles;
        juce::StringArray trackArtists;
        int currentTrackIndex = 0;

        std::unique_ptr<juce::MidiInput> midiInput;

        DelayEffect delayEffect;
        ReverbEffect reverbEffect;
        FrequencyBands frequencyBands;
    
// float values to store the knob values
    float bassCutoff = 200.0f;
    float midsCutoff = 1000.0f;
    float topsCutoff = 5000.0f;

        void loadPlaylist();
        void shutdownAudio();
    
    // Members for storing playlist data
     juce::StringArray trackNames;
     juce::Array<juce::File> trackFiles;
    juce::AudioTransportSource audioSource;
    juce::AudioTransportSource transportSource;
    // Reader source to hold the audio data
        std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    
    juce::AudioDeviceManager speakerDeviceManager;
    juce::AudioDeviceManager headphoneDeviceManager;

    juce::AudioSourcePlayer speakerSourcePlayer;
    juce::AudioSourcePlayer headphoneSourcePlayer;
    
    std::unique_ptr<juce::AudioProcessor> speakerProcessor;
        std::unique_ptr<juce::AudioProcessor> headphoneProcessor;
    
  //  juce::IIRFilterAudioSource lowPassFilter { &transportSource, false }; // Filter for speakers
    
    
   // juce::OSCReceiver oscReceiver;  // Receiver for OSC messages
    juce::OSCSender oscSender;
   // juce::OSCMessage oscMessage;

    LowPassFilter lowPassFilter;
    
    LowPassFilter topsFilter;  // Will be used for high-pass (treble)
    juce::SmoothedValue<float> smoothedTopsCutoff;
    bool topsFilterNeedsReset = false;
    
    // For bypass smoothing
    juce::LinearSmoothedValue<float> topsFilterMix;
    juce::AudioBuffer<float> topsFilterBuffer; // Temporary buffer for processing
    
    // Reverb parameters to send via OSC
    struct ReverbParams
    {
        float roomSize = 0.5f;
        float wetLevel = 0.5f;
    };
    
    ReverbParams reverbParams;  // Instance of the reverb parameters struct
    FrequencyBands eq;
    
    void initializeOSC();
    
    bool isDelayActive = false;
    bool isReverbActive = false;
    float volumeLevel = 1.0f;
    double currentSampleRate = 44100.0;
    int currentBlockSize = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

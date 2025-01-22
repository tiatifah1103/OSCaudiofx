#pragma once

#include <JuceHeader.h>
#include "MainComponent.h"
#include "DelayEffect.h"
#include "ReverbEffect.h"
#include <juce_osc/juce_osc.h>

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

        void loadPlaylist();
        void shutdownAudio();
    
    // Members for storing playlist data
     juce::StringArray trackNames;
     juce::Array<juce::File> trackFiles;
    juce::AudioTransportSource audioSource;
    juce::AudioTransportSource transportSource;
    // Reader source to hold the audio data
        std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    
    
   // juce::OSCReceiver oscReceiver;  // Receiver for OSC messages
    juce::OSCSender oscSender; 
   // juce::OSCMessage oscMessage;


    
    
    // Reverb parameters to send via OSC
    struct ReverbParams
    {
        float roomSize = 0.5f;
        float wetLevel = 0.5f;
    };
    
    ReverbParams reverbParams;  // Instance of the reverb parameters struct
    
    void initializeOSC();
    
    bool isDelayActive = true;
    bool isReverbActive = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

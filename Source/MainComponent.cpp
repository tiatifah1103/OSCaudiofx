#include "MainComponent.h"
#include "DelayEffect.h"
#include "ReverbEffect.h"

//==============================================================================
MainComponent::MainComponent()

{

        // Initialise Audio Device Manager
        deviceManager.initialise(2, 2, nullptr, true);
    
        // Connect the sender to the host (e.g., localhost) and port
        if (!oscSender.connect("127.0.0.1", 9000))
            juce::Logger::writeToLog("Failed to connect to OSC receiver!");
        if (!oscSender.send("/build", juce::String("JUCE Build Complete!")))
            juce::Logger::writeToLog("Failed to send OSC message!");
        
        setSize(800, 600);
        formatManager.registerBasicFormats();

        // Configures the audio device
        setAudioChannels(2, 2); // deviceManager should be initialised before

        // Load playlist
        loadPlaylist();


    // Set up MIDI
    auto midiDevices = juce::MidiInput::getDevices();
    if (!midiDevices.isEmpty())
    {
        midiInput = juce::MidiInput::openDevice(0, this); //
        if (midiInput)
        {
            midiInput->start();
        }
        else
        {
            juce::Logger::writeToLog("Failed to open MIDI device.");
        }
    }
    else
    {
        juce::Logger::writeToLog("No MIDI devices found.");
    }

    // Load the first track
    if (!playlistFiles.isEmpty())
    {
        formatManager.registerBasicFormats();
//        audioTransportSource.start();
    }

//    // initial effect parameters
//    delayEffect.setDelayTime(500);
//    delayEffect.setFeedback(0.5f);
//    delayEffect.setMix(0.3f);
}



MainComponent::~MainComponent()
{
    if (midiInput)
        midiInput->stop();

    shutdownAudio();
}

// Assets folder path
juce::File MainComponent::getAssetsFolder()
{
    auto currentWorkingDir = juce::File::getCurrentWorkingDirectory();

 
    auto assetsFolder = currentWorkingDir.getChildFile("/Users/latifahdickson/Documents/UNI/of_v0.12.0_osx_release/apps/myApps/audio_fx_test/Assets");
//    assetsFolder = currentWorkingDir.getChildFile("/Users/latifahdickson/Documents/UNI/of_v0.12.0_osx_release/apps/myApps/audio_fx_test/Assets");

    if (!assetsFolder.exists() || !assetsFolder.isDirectory())
    {
        juce::Logger::writeToLog("Assets folder not found!");
    }
    else
    {
        juce::Logger::writeToLog("Assets folder found at: " + assetsFolder.getFullPathName());
    }

    return assetsFolder;
}

void MainComponent::loadPlaylist()
{
        auto assetsFolder = getAssetsFolder();
        auto jsonFile = assetsFolder.getChildFile("playlist.json");

        if (!jsonFile.existsAsFile())
        {
            juce::Logger::writeToLog("Playlist JSON file not found!");
            return;
        }

        juce::Logger::writeToLog("Loading playlist from: " + jsonFile.getFullPathName());

        // Read and parse JSON
        juce::var jsonData;
        juce::FileInputStream fileStream(jsonFile);

        if (fileStream.openedOk())
        {
            auto jsonContent = fileStream.readEntireStreamAsString();
            jsonData = juce::JSON::parse(jsonContent);

            if (jsonData.isArray())
            {
                for (auto& track : *jsonData.getArray())
                {
                    if (track.isObject())
                    {
                        auto* trackObj = track.getDynamicObject();

                        //extract fields
                        auto trackPath = assetsFolder.getChildFile(trackObj->getProperty("file").toString());
                        auto trackTitle = trackObj->getProperty("title").toString();
                        auto trackArtist = trackObj->getProperty("artist").toString();

                        //verify file exists
                        if (trackPath.existsAsFile())
                        {
                            trackNames.add(trackTitle + " by " + trackArtist);
                            trackFiles.add(trackPath);
                            juce::Logger::writeToLog("Track added: " + trackTitle + " by " + trackArtist
                                                    + " (" + trackPath.getFullPathName() + ")");
                        }
                        else
                        {
                            juce::Logger::writeToLog("Track file not found: " + trackPath.getFullPathName());
                        }
                    }
                }
            }
            else
            {
                juce::Logger::writeToLog("Invalid JSON format: Expected an array.");
            }
        }
        else
        {
            juce::Logger::writeToLog("Failed to open playlist JSON file.");
        }
    
    
}


void MainComponent::loadAudioFile(const juce::File& file)
{
    auto* reader = formatManager.createReaderFor(file); // Create the reader for the file
    
    if (reader != nullptr)
    {
        DBG("Sample Rate: " << reader->sampleRate);  // Print the sample rate of the loaded file

        // Set the transport source with the audio reader
        transportSource.setSource(new juce::AudioFormatReaderSource(reader, true));
        
        //set position to 0 to start from the beginning
        transportSource.setPosition(0);
    }
    else
    {
        DBG("Error: Could not load audio file.");
    }
}
//==============================================================================

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    // Loads the playlist before attempting to play any track
    loadPlaylist();

    // Prepare the transport source and effects
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    delayEffect.prepare(sampleRate, samplesPerBlockExpected);
    lowPassFilter.prepare(sampleRate, samplesPerBlockExpected);

    // Check if any tracks have been loaded from the playlist folder
    if (!trackFiles.isEmpty())
    {
        // Load the first track from the playlist
        juce::File firstTrack = trackFiles.getFirst();
        auto* reader = formatManager.createReaderFor(firstTrack);
        currentTrackIndex = 0; // Start from the first track
        playNextTrack(); // Start playback automatically
        
        if (reader != nullptr)
        {
            std::unique_ptr<juce::AudioFormatReaderSource> newSource(
                new juce::AudioFormatReaderSource(reader, true)
            );
            transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
            readerSource.reset(newSource.release());

            juce::Logger::writeToLog("Loaded track: " + firstTrack.getFullPathName());
            transportSource.start(); // Start playback
        }
        else
        {
            juce::Logger::writeToLog("Error: Could not load audio file: " + firstTrack.getFullPathName());
        }
    }
    else
    {
        juce::Logger::writeToLog("No tracks found in the playlist folder.");
    }

    // Log and set the desired audio output devices
    auto& audioDevices = deviceManager.getAvailableDeviceTypes();
    juce::StringArray deviceNames;
    
    // Loop through available device types and their names
    for (auto* deviceType : audioDevices)
    {
        juce::StringArray names = deviceType->getDeviceNames();
        deviceNames.addArray(names);
    }
    
    juce::Logger::writeToLog("Available Audio Devices:");
    for (const auto& deviceName : deviceNames)
    {
        juce::Logger::writeToLog(deviceName);

    //check for specific audio device
        if (deviceName.containsIgnoreCase("KM_B2 AUX"))
        {
            juce::AudioDeviceManager::AudioDeviceSetup setup;
            deviceManager.getAudioDeviceSetup(setup);

            setup.outputDeviceName = deviceName;
            setup.sampleRate = sampleRate;
            setup.bufferSize = samplesPerBlockExpected;

            juce::String result = deviceManager.setAudioDeviceSetup(setup, true);
            if (result.isEmpty())
            {
                juce::Logger::writeToLog("Successfully set output device to: " + deviceName);
            }
            else
            {
                juce::Logger::writeToLog("Failed to set audio device setup: " + result);
            }
        }

        // Check for specific audio device
        if (deviceName.containsIgnoreCase("External Headphones"))
        {
            juce::AudioDeviceManager::AudioDeviceSetup setup;
            deviceManager.getAudioDeviceSetup(setup);

            setup.outputDeviceName = deviceName;
            setup.sampleRate = sampleRate;
            setup.bufferSize = samplesPerBlockExpected;

            juce::String result = deviceManager.setAudioDeviceSetup(setup, true);
            if (result.isEmpty())
            {
                juce::Logger::writeToLog("Successfully set output device to: " + deviceName);
            }
            else
            {
                juce::Logger::writeToLog("Failed to set audio device setup: " + result);
            }
        }
    }
}




void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    if (transportSource.isPlaying())
    {
        transportSource.getNextAudioBlock(bufferToFill);

        // Checks if playback has finished
        if (transportSource.getCurrentPosition() >= transportSource.getLengthInSeconds())
        {
            playNextTrack();
        }

        // Get the current audio device
        auto* currentDevice = deviceManager.getCurrentAudioDevice();
        if (currentDevice == nullptr)
            return;

        // Duplicate buffer for separate processing
        // Create a separate copy of the audio buffer to apply different effects
        // to the external speakers while keeping the original buffer for headphones.
        juce::AudioBuffer<float> speakerBuffer(*bufferToFill.buffer);

        // Check if the audio device is headphones or speakers
        if (currentDevice->getName().containsIgnoreCase("External Headphones"))
        {
            for (int channel = 0; channel < speakerBuffer.getNumChannels(); ++channel)
            {
                speakerBuffer.applyGain(channel, bufferToFill.startSample, bufferToFill.numSamples, volumeLevel);
            }
            bufferToFill.buffer->copyFrom(0, bufferToFill.startSample, speakerBuffer, 0, bufferToFill.startSample, bufferToFill.numSamples);
            if (speakerBuffer.getNumChannels() > 1)
                bufferToFill.buffer->copyFrom(1, bufferToFill.startSample, speakerBuffer, 1, bufferToFill.startSample, bufferToFill.numSamples);
        }
        else if (currentDevice->getName().containsIgnoreCase("KM_B2 AUX"))
        {
            // Apply the low-pass filter to the external speaker output
            lowPassFilter.process(speakerBuffer);

            //Volume control to the speaker buffer as well:
            for (int channel = 0; channel < speakerBuffer.getNumChannels(); ++channel)
            {
                speakerBuffer.applyGain(channel, bufferToFill.startSample, bufferToFill.numSamples, volumeLevel);
            }

            // Copies the processed speaker buffer back into the original buffer to output to the speakers
            bufferToFill.buffer->copyFrom(0, bufferToFill.startSample, speakerBuffer, 0, bufferToFill.startSample, bufferToFill.numSamples);
            if (speakerBuffer.getNumChannels() > 1)
                bufferToFill.buffer->copyFrom(1, bufferToFill.startSample, speakerBuffer, 1, bufferToFill.startSample, bufferToFill.numSamples);
        }

        // Apply effects (delay or reverb) to both outputs
        if (isDelayActive)
        {
            delayEffect.process(bufferToFill);
        }
        else if (isReverbActive)
        {
            reverbEffect.process(*bufferToFill.buffer);
        }
    }
}

// MIDI input callback -- handlding the knobs, sliders, buttons inputs
void MainComponent::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message)
{
    {
        if (message.isController())
        {
            // Extract MIDI controller data
            auto controllerNumber = message.getControllerNumber();
            auto controllerValue = message.getControllerValue();
            
            // Log the message to the console
            juce::Logger::writeToLog("MIDI Message Received: Controller Number = "
                                     + juce::String(controllerNumber)
                                     + ", Value = "
                                     + juce::String(controllerValue));
            
            if (controllerNumber == 9) //volume control for speaker
            {
                volumeLevel = juce::jmap(static_cast<float>(controllerValue), 0.0f, 127.0f, 0.0f, 1.0f);
                juce::Logger::writeToLog("Volume set to: " + juce::String(volumeLevel));
            }
            
            //toggling between audio effects :
            
            if (controllerNumber == 90) // Button for Delay
            {
                isDelayActive = !isDelayActive;
                isReverbActive = false; // Disable Reverb if Delay is toggled
                juce::Logger::writeToLog(isDelayActive ? "Delay Effect Enabled" : "Delay Effect Disabled");
            }
            else if (controllerNumber == 91) // Button for Reverb
            {
                isReverbActive = !isReverbActive;
                isDelayActive = false; // Disable Delay if Reverb is toggled
                juce::Logger::writeToLog(isReverbActive ? "Reverb Effect Enabled" : "Reverb Effect Disabled");
            }
            
            // Adjust delay parameters only if delay is active
            if (isDelayActive)
            {
                
                if (controllerNumber == 18)
                {
                    // Maps the controller value (0-127) to a range (0.0 to 1.0) for the delay mix parameter
                    float mixValue = juce::jmap<float>(controllerValue, 0, 127, 0.0f, 1.0f);
                    delayEffect.setMix(mixValue); // Updates the mix level of the delay effect
                    
                    // Creates an OSC message to communicate the updated mix value
                    juce::OSCMessage mixValueMessage("/delay/mixValue", mixValue);
                    oscSender.send(mixValueMessage); // Sends OSC message
                }
                else if (controllerNumber == 19)
                {
                    // Maps the controller value (0-127) to a range (0.0 to 0.9) for the delay feedback parameter
                    float feedbackValue = juce::jmap<float>(controllerValue, 0, 127, 0.0f, 0.9f);
                    delayEffect.setFeedback(feedbackValue); // Updates the feedback level of the delay effect
                    
                    // Create and send an OSC message for the feedback value
                    juce::OSCMessage feedbackValueMessage("/delay/feedbackValue", feedbackValue);
                    oscSender.send(feedbackValueMessage);
                }
                else if (controllerNumber == 20)
                {
                    // Map the controller value (0-127) to a range (50ms to 2000ms) for the delay time
                    int delayTime = juce::jmap<int>(controllerValue, 0, 127, 50, 2000);
                    delayEffect.setDelayTime(delayTime); // Update the delay time of the delay effect
                    
                    // Create and send an OSC message for the delay time
                    juce::OSCMessage delayTimeMessage("/delay/delayTime", delayTime);
                    oscSender.send(delayTimeMessage);
                }
                
                else if (controllerNumber == 21){
                    // Map the controller value (0-127) to a mix range (0 for dry to 1 for fully wet)
                    float mix = juce::jmap<float>(controllerValue, 0, 127, 0.0f, 1.0f);
                    delayEffect.setMix(mix); // Update the mix parameter of the delay effect

                    // Create and send an OSC message for the delay mix
                    juce::OSCMessage mixMessage("/delay/mix", mix);
                    oscSender.send(mixMessage);

                }
                
                // effects activation logic
                if (message.isNoteOn()) {
                    int midiNote = message.getNoteNumber();

                    if (midiNote == 91) { // Example MIDI note for Reverb
                        oscSender.send("/effect/reverb/activate", 1);
                    } else if (midiNote == 90) { // Example MIDI note for Delay
                        oscSender.send("/effect/delay/activate", 1);
                    }
                } else if (message.isNoteOff()) {
                    int midiNote = message.getNoteNumber();

                    if (midiNote == 91) { // Turn off Reverb
                        oscSender.send("/effect/reverb/activate", 0);
                    } else if (midiNote == 90) { // Turn off Delay
                        oscSender.send("/effect/delay/activate", 0);
                    }
                }
                
                
            }
            
            // Adjust reverb parameters only if delay is active
            if (isReverbActive)
            {
                
                // Reverb effect controls
                if (controllerNumber == 20)
                {
                    // Map the controller value (0-127) to a range (0.0 to 1.0) for the reverb room size
                    float roomSize = juce::jmap<float>(controllerValue, 0, 127, 0.0f, 1.0f);
                    reverbEffect.setRoomSize(roomSize); // Update the room size of the reverb effect
                    
                    // Create and send an OSC message for the room size parameter
                    juce::OSCMessage roomSizeMessage("/reverb/roomSize", roomSize);
                    oscSender.send(roomSizeMessage);
                }
                else if (controllerNumber == 21)
                {
                    // Map the controller value (0-127) to a range (0.0 to 1.0) for the wet level
                    float wetLevel = juce::jmap<float>(controllerValue, 0, 127, 0.0f, 1.0f);
                    reverbEffect.setWetLevel(wetLevel); // Update the wet level of the reverb effect
                    
                    // Create and send an OSC message for the wet level parameter
                    juce::OSCMessage wetLevelMessage("/reverb/wetLevel", wetLevel);
                    oscSender.send(wetLevelMessage);
                }
                
            }
        }
        else if (message.isNoteOnOrOff())
        {
            // Log note on/off messages
            juce::Logger::writeToLog("MIDI Note "
                                     + juce::String(message.isNoteOn() ? "On" : "Off")
                                     + ": Note Number = "
                                     + juce::String(message.getNoteNumber())
                                     + ", Velocity = "
                                     + juce::String(message.getVelocity()));
            
            // Play/Pause button
            if (message.getNoteNumber() == 59 && message.isNoteOn())
            {
                togglePlayPause();
            }
            
            if (message.getNoteNumber() == 66 && message.isNoteOn()) //for testing, advances to next track in the playlist
                    {
                        nextTrack();
                    }
            
            
            // Handle effect toggling based on note numbers
                    if (message.getNoteNumber() == 90 && message.isNoteOn()) // Button for Delay
                    {
                        isDelayActive = !isDelayActive;
                        isReverbActive = false; // Disable Reverb if Delay is toggled
                        juce::Logger::writeToLog(isDelayActive ? "Delay Effect Enabled" : "Delay Effect Disabled");
                    }
                    else if (message.getNoteNumber() == 91 && message.isNoteOn()) // Button for Reverb
                    {
                        isReverbActive = !isReverbActive;
                        isDelayActive = false; // Disable Delay if Reverb is toggled
                        juce::Logger::writeToLog(isReverbActive ? "Reverb Effect Enabled" : "Reverb Effect Disabled");
                    }
            
            // MIDI control for advancing the video
            else if (message.getNoteNumber() == 64 && message.isNoteOn())
            {
                // Send OSC message to advance the video
                juce::OSCMessage videoAdvanceMessage("/video/advance", 1); // Sending a signal to advance video
                oscSender.send(videoAdvanceMessage); 
                juce::Logger::writeToLog("Sending OSC message to advance video");
            }
            
            
        }
        
        
    }
    

}

// Play/Pause toggle function
void MainComponent::togglePlayPause()
{

        transportSource.start();
        juce::Logger::writeToLog("Playback started: " + trackNames[currentTrackIndex]);

}

// Volume control from MIDI CC
void MainComponent::handleVolumeControl(int controllerValue)
{
    // Maps the controller value to volume (0-1 range)
    float volume = juce::jmap<float>(controllerValue, 0, 127, 0.0f, 1.0f);
    audioTransportSource.setGain(volume); // Sets the volume of the audio transport source
    juce::Logger::writeToLog("Volume set to: " + juce::String(volume));
}


// Start next track
void MainComponent::nextTrack()
{
    if (!trackFiles.isEmpty())
    {
        currentTrackIndex = (currentTrackIndex + 1) % trackFiles.size(); // Loop back to the first track if at the end
        loadAudioFile(trackFiles[currentTrackIndex]);
        transportSource.start(); // Start playback
        juce::Logger::writeToLog("Playing next track: " + trackFiles[currentTrackIndex].getFullPathName());
    }
}

void MainComponent::playNextTrack()
{
    if (trackFiles.isEmpty()) return;

    currentTrackIndex = (currentTrackIndex + 1) % trackFiles.size(); // Loop playlist

    loadAudioFile(trackFiles[currentTrackIndex]); // Load new track
    transportSource.start(); // Start playback
}

// Shutdown
void MainComponent::shutdownAudio()
{
    audioTransportSource.setSource(nullptr);
    midiInput->stop();
}

void MainComponent::releaseResources()
{
    
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    
}

void MainComponent::resized()
{

}

//void MainComponent::listAudioDevices()
//{
//    auto& deviceTypes = deviceManager.getAvailableDeviceTypes();
//
//    for (auto* deviceType : deviceTypes)
//    {
//        juce::StringArray deviceNames = deviceType->getDeviceNames();
//        for (const auto& name : deviceNames)
//        {
//            juce::Logger::writeToLog("Available Audio Device: " + name);
//        }
//    }
//}


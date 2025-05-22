#include "MainComponent.h"
#include "DelayEffect.h"
#include "ReverbEffect.h"

//==============================================================================
MainComponent::MainComponent()

{

        // Initialise Audio Device Manager
     //   deviceManager.initialise(2, 2, nullptr, true);
    
        // Connect the sender to the host (e.g., localhost) and port
        if (!oscSender.connect("127.0.0.1", 9000))
            juce::Logger::writeToLog("Failed to connect to OSC receiver!");
        if (!oscSender.send("/build", juce::String("JUCE Build Complete!")))
            juce::Logger::writeToLog("Failed to send OSC message!");
        
        setSize(800, 600);
        formatManager.registerBasicFormats();

        // Configures the audio device
        setAudioChannels(1, 1); // deviceManager should be initialised before

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

}

MainComponent::~MainComponent()
{
    // Stops MIDI input if it was started
    if (midiInput)
        midiInput->stop();

    // Shuts down the audio system to free audio resources
    shutdownAudio();
}

// Assets folder path
juce::File MainComponent::getAssetsFolder()
{
    // Gets the working directory of the application
    auto currentWorkingDir = juce::File::getCurrentWorkingDirectory();

    // Defines the path to the Assets folder manually
    auto assetsFolder = currentWorkingDir.getChildFile("/Users/latifahdickson/Documents/UNI/of_v0.12.0_osx_release/apps/myApps/audio_fx_test/Assets");

    // Checks if it exists and is a directory
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
    
    // Get path to Assets folder and find the playlist JSON file
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

            // Expect the top-level structure to be an array
            if (jsonData.isArray())
            {
                for (auto& track : *jsonData.getArray())
                {
                    if (track.isObject())
                    {
                        auto* trackObj = track.getDynamicObject();

                        //extract info from .json
                        auto trackPath = assetsFolder.getChildFile(trackObj->getProperty("file").toString());
                        auto trackTitle = trackObj->getProperty("title").toString();
                        auto trackArtist = trackObj->getProperty("artist").toString();

                        // if  file exists, add it to the internal playlist
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
    
    currentBlockSize = samplesPerBlockExpected;  // Store the block size
       currentSampleRate = sampleRate;
    
    // Set up the Multi-Output Device
        auto* deviceType = deviceManager.getCurrentDeviceTypeObject();
        juce::StringArray allDevices = deviceType->getDeviceNames();

        // Find Multi-Output Device by name
        juce::String multiOutputDeviceName = "GIGAPORT eX";
        if (allDevices.contains(multiOutputDeviceName)) {
            juce::AudioDeviceManager::AudioDeviceSetup setup;
            deviceManager.getAudioDeviceSetup(setup);

            // Configure for Multi-Output Device
            setup.outputDeviceName = multiOutputDeviceName;
            setup.sampleRate = sampleRate;
            setup.bufferSize = samplesPerBlockExpected;

            juce::String error = deviceManager.setAudioDeviceSetup(setup, true);
            if (error.isEmpty()) {
                DBG("Successfully set up Multi-Output Device: " + multiOutputDeviceName);
            } else {
                DBG("Error setting device: " + error);
            }
        } else {
            DBG("Multi-Output Device not found!");
        
    }
    
    // Loads the playlist before attempting to play any track
    loadPlaylist();
    currentSampleRate = sampleRate; // Store the sample rate
    // Prepare the transport source and effects
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    delayEffect.prepare(sampleRate, samplesPerBlockExpected);
    reverbEffect.prepare(sampleRate, samplesPerBlockExpected);
    eq.prepare(sampleRate, samplesPerBlockExpected);
    

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


    
    
}




void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    
    // Clears the active region of the buffer to avoid leftover audio data from previous processing
   
        bufferToFill.clearActiveBufferRegion();

    // Checks if the audio transport source is currently playing
        if (transportSource.isPlaying())
        {
            // Fills the buffer with the next block of audio from the transport source
            transportSource.getNextAudioBlock(bufferToFill);
            
            // If playback has reached the end of the track, advances
            if (transportSource.getCurrentPosition() >= transportSource.getLengthInSeconds())
            {
                playNextTrack();
            }

            //Temp buffer to apply additional audio processing
            juce::AudioBuffer<float> processingBuffer(
                bufferToFill.buffer->getArrayOfWritePointers(),
                bufferToFill.buffer->getNumChannels(),
                bufferToFill.startSample,
                bufferToFill.numSamples
            );

            processingBuffer.applyGain(volumeLevel * 2.5);


            if (delayEffect.isActive())
                delayEffect.process(bufferToFill);
                
            if (reverbEffect.isActive())
                reverbEffect.process(processingBuffer);
                
            // Applies EQ filter to the buffer
            eq.process(*bufferToFill.buffer, {});

            //Safety checks to prevent audio clipping:
                    //  amplitude limited to between -0.95 and 0.95 on all channels
        for (int ch = 0; ch < bufferToFill.buffer->getNumChannels(); ++ch) {
            juce::FloatVectorOperations::clip(
                bufferToFill.buffer->getWritePointer(ch, bufferToFill.startSample),
                bufferToFill.buffer->getReadPointer(ch, bufferToFill.startSample),
                -0.95f, 0.95f,
                bufferToFill.numSamples
            );
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
            
            if (controllerNumber == 14) //volume control for user headphones
            {
                volumeLevel = juce::jmap(static_cast<float>(controllerValue), 0.0f, 127.0f, 0.0f, 1.0f);
                juce::Logger::writeToLog("Volume set to: " + juce::String(volumeLevel));
            }
            
            
            if (controllerNumber == 20) { // Room Size
                float roomSize = juce::jmap<float>(controllerValue, 0, 127, 0.0f, 1.0f);
                reverbEffect.setRoomSize(roomSize);
                oscSender.send("/reverb/roomSize", roomSize);
            }
            else if (controllerNumber == 18) { // Wet Level
                float wetLevel = juce::jmap<float>(controllerValue, 0, 127, 0.0f, 1.0f);
                reverbEffect.setWetLevel(wetLevel);
                oscSender.send("/reverb/wetLevel", wetLevel);
            }
            else if (controllerNumber == 12) { // Damping
                float damping = juce::jmap<float>(controllerValue, 0, 127, 0.0f, 1.0f);
                reverbEffect.setDamping(damping);
                oscSender.send("/reverb/damping", damping);
            }
            else if (controllerNumber == 11) { // Width
                float width = juce::jmap<float>(controllerValue, 0, 127, 0.0f, 1.0f);
                reverbEffect.setWidth(width);
                oscSender.send("/reverb/width", width);
            }
            
            if (controllerNumber == 21) { // Bass
                float bassGain = juce::jmap<float>(controllerValue, 0, 127, 0.0f, 1.0f);
                eq.setBassGain(bassGain);
                oscSender.send("/eq/bass", bassGain);
            }
            else if (controllerNumber == 19) { // Mids
                float midsGain = juce::jmap<float>(controllerValue, 0, 127, 0.0f, 1.0f);
                eq.setMidsGain(midsGain);
                oscSender.send("/eq/mids", midsGain);
            }
            else if (controllerNumber == 17) { // Tops
                float topsGain = juce::jmap<float>(controllerValue, 0, 127, 0.0f, 1.0f);
                eq.setTopsGain(topsGain);
                oscSender.send("/eq/tops", topsGain);
            }
            
            if (controllerNumber == 8)
            {
                
                float mixValue = juce::jmap<float>(controllerValue, 0, 127, 0.0f, 1.0f);
                delayEffect.setMix(mixValue); // Updates the mix level of the delay effect
                // Creates an OSC message to communicate the updated mix value
                juce::OSCMessage mixValueMessage("/delay/mixValue", mixValue);
                oscSender.send(mixValueMessage); // Sends OSC message
            }
            else if (controllerNumber == 23)
            {
                
                float feedbackValue = juce::jmap<float>(controllerValue, 0, 127, 0.0f, 1.0f);
                delayEffect.setFeedback(feedbackValue);
                juce::OSCMessage feedbackValueMessage("/delay/feedbackValue", feedbackValue);
                oscSender.send(feedbackValueMessage);
            }
            else if (controllerNumber == 9)
            {
                int delayTime = juce::jmap<int>(controllerValue, 0, 127, 50, 2000);
                delayEffect.setDelayTime(delayTime);
                juce::OSCMessage delayTimeMessage("/delay/delayTime", delayTime);
                oscSender.send(delayTimeMessage);
            }
        }
  
             if (message.isNoteOnOrOff())
            {
                // Log note on/off messages
                juce::Logger::writeToLog("MIDI Note "
                                         + juce::String(message.isNoteOn() ? "On" : "Off")
                                         + ": Note Number = "
                                         + juce::String(message.getNoteNumber())
                                         + ", Velocity = "
                                         + juce::String(message.getVelocity()));
         
                
                // MIDI control for advancing the split screen video
                 if (message.getNoteNumber() == 64 && message.isNoteOn())
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
        currentTrackIndex = (currentTrackIndex + 1) % trackFiles.size(); // Loops back to the first track if at the end
        loadAudioFile(trackFiles[currentTrackIndex]);
        transportSource.start(); // Start playback
        juce::Logger::writeToLog("Playing next track: " + trackFiles[currentTrackIndex].getFullPathName());
    }
}

void MainComponent::playNextTrack()
{
    if (trackFiles.isEmpty())
        return;

    
    // Stop the current playback and clean up the current audio source
    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset();

    // Move to the next track in the playlist --- loops back to start of playlist if it ends
    currentTrackIndex = (currentTrackIndex + 1) % trackFiles.size();
    juce::File nextTrack = trackFiles[currentTrackIndex];

    // Creates an audio reader for the next track
    auto* reader = formatManager.createReaderFor(nextTrack);
    if (reader != nullptr)
    {
        // Wrapped the reader in a reader source that can be used with the transport
        double fileSampleRate = reader->sampleRate;
        
        std::unique_ptr<juce::AudioFormatReaderSource> newSource(
            new juce::AudioFormatReaderSource(reader, true)
        );

        // Set the transport source to use the new audio source
        transportSource.setSource(newSource.get(), 0, nullptr, fileSampleRate);
        readerSource.reset(newSource.release());

        // Use the stored block size here
        transportSource.prepareToPlay(currentBlockSize, currentSampleRate);

        // Log the track being played and start playback
        juce::Logger::writeToLog("Now playing: " + nextTrack.getFullPathName());
        transportSource.setPosition(0.0); // start from beginning
        transportSource.start();
    }
    else
    {
        juce::Logger::writeToLog("Error loading next track: " + nextTrack.getFullPathName());
    }
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


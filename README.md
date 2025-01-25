# OSCaudiofx


## Description
This project is part of an interactive documentary that uses openFrameworks to create dynamic audio effects synchronised with audio manipulation. The visuals respond to user interactions through a DJ controller, providing an immersive experience.

### Key Features
- delay, reverb.
- Audio-synchronised visuals triggered by real-time DJ controller input.
- Modular class-based architecture for each effect.

# Requirements
### Software
- **openFrameworks** 
- A C++17-compatible compiler
- **JUCE Framework** (for the audio engine)
- corresponding openFramworks code (visual-soundfx-test2)

- Required openFrameworks addons:
  - `ofxOsc`
  - `ofxMidi` 
  - `ofxJSON` 
  
### Running Instructions

To see the visual effects synchronized with the audio manipulation:

Connect the Hardware:
Ensure the MIDI controller (e.g. DJ controller) is plugged into your computer.

Build and Run the openFrameworks Code:
Open the openFrameworks project in your preferred IDE (Xcode, Visual Studio, etc.).
Build and run the project to initialize the visual effects and MIDI controller functionality.

Build and Run the JUCE Audio Engine:
Open the JUCE project in Projucer and export it to your IDE.
Build and run the JUCE audio engine to enable Open Sound Control (OSC) communication with the openFrameworks application.

Interaction:
Use the DJ controller to adjust audio effects (e.g., reverb, delay) and navigate the timeline.
Observe synchronized visual effects on the screen and manipulated audio in from the speaker.

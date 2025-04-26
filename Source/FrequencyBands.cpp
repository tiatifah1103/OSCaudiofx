#include "FrequencyBands.h"

// Constructor to initialiase the filters with their defaults
FrequencyBands::FrequencyBands()
{
    bassFilter.setHighpass(false);  // Bass is low-pass
    bassFilter.setCutoffFrequency(120.0f);  // default cutoff for bass

    midsFilter.setHighpass(false);  // Mids is bandpass
    midsFilter.setCutoffFrequency(1200.0f);  // default cutoff for mids

    topsFilter.setHighpass(true);  // Tops is high-pass
    topsFilter.setCutoffFrequency(2000.0f);  //  default cutoff for tops
}

FrequencyBands::~FrequencyBands()
{
}

void FrequencyBands::prepare(double sampleRate, int samplesPerBlock)
{
    bassFilter.setSamplingRate(static_cast<float>(sampleRate));
    midsFilter.setSamplingRate(static_cast<float>(sampleRate));
    topsFilter.setSamplingRate(static_cast<float>(sampleRate));
}

void FrequencyBands::process(juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midiMessages)
{
    juce::AudioBuffer<float> bassBuffer(buffer.getNumChannels(), buffer.getNumSamples());
    juce::AudioBuffer<float> midsBuffer(buffer.getNumChannels(), buffer.getNumSamples());
    juce::AudioBuffer<float> topsBuffer(buffer.getNumChannels(), buffer.getNumSamples());

    // Copies of the original signal
    bassBuffer.makeCopyOf(buffer);
    midsBuffer.makeCopyOf(buffer);
    topsBuffer.makeCopyOf(buffer);


    juce::MidiBuffer emptyMidi;

    // Processes each band with the empty buffer
    bassFilter.processBlock(bassBuffer, emptyMidi);
    midsFilter.processBlock(midsBuffer, emptyMidi);
    topsFilter.processBlock(topsBuffer, emptyMidi);

    // Mix back
    buffer.clear();
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        buffer.addFrom(ch, 0, bassBuffer, ch, 0, buffer.getNumSamples());
        buffer.addFrom(ch, 0, midsBuffer, ch, 0, buffer.getNumSamples());
        buffer.addFrom(ch, 0, topsBuffer, ch, 0, buffer.getNumSamples());
    }
}


// Setters for the cutoff frequencies of each filter
void FrequencyBands::setBassCutoff(float frequency)
{
    bassFilter.setCutoffFrequency(frequency);
}

void FrequencyBands::setMidsCutoff(float frequency)
{
    midsFilter.setCutoffFrequency(frequency);
}

void FrequencyBands::setTopsCutoff(float frequency)
{
    topsFilter.setCutoffFrequency(frequency);
}

void FrequencyBands::setBassResonance(float resonance)
{
 
}

void FrequencyBands::setMidsResonance(float resonance)
{
   
}

void FrequencyBands::setTopsResonance(float resonance)
{

}

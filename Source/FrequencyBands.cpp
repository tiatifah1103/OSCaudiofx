#include "FrequencyBands.h"

FrequencyBands::FrequencyBands() {
    
}

void FrequencyBands::prepare(double sampleRate, int /*samplesPerBlock*/) {
    currentSampleRate = sampleRate;

    // Bass Low-pass
    bassLowPass.setCoefficients(juce::IIRCoefficients::makeLowPass(sampleRate, 250.0));

    // Mids Band-pass
    midsBandPass.setCoefficients(juce::IIRCoefficients::makeBandPass(sampleRate, 1200.0, 0.5));

    // Tops High-pass
    topsHighPass.setCoefficients(juce::IIRCoefficients::makeHighPass(sampleRate, 1000.0));
}

void FrequencyBands::process(juce::AudioBuffer<float>& buffer, const juce::MidiBuffer&) {
    // Separate buffers for each band
    juce::AudioBuffer<float> bassBuffer, midsBuffer, topsBuffer;
    bassBuffer.makeCopyOf(buffer);
    midsBuffer.makeCopyOf(buffer);
    topsBuffer.makeCopyOf(buffer);

    // Applies filters
    bassLowPass.processSamples(bassBuffer.getWritePointer(0), bassBuffer.getNumSamples());
    midsBandPass.processSamples(midsBuffer.getWritePointer(0), midsBuffer.getNumSamples());
    topsHighPass.processSamples(topsBuffer.getWritePointer(0), topsBuffer.getNumSamples());

    // Applies gains
    bassBuffer.applyGain(bassGain);
    midsBuffer.applyGain(midsGain);
    topsBuffer.applyGain(topsGain);

    // Mixes all bands back into the original buffer
    buffer.clear();
    buffer.addFrom(0, 0, bassBuffer, 0, 0, buffer.getNumSamples());
    buffer.addFrom(0, 0, midsBuffer, 0, 0, buffer.getNumSamples());
    buffer.addFrom(0, 0, topsBuffer, 0, 0, buffer.getNumSamples());
}

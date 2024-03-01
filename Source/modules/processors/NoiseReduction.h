/*
  ==============================================================================

    NoiseReduction.h
    Created: 1 Mar 2024 8:45:17am
    Author:  Przemysław Barski

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RMSMeters.h"

template <typename SampleType>
class NoiseReduction  :  public RMSMeters<float>
{
public:
    //==============================================================================
    /** Constructor. */
    NoiseReduction()
    {
        RMSFilter.setLevelCalculationType (juce::dsp::BallisticsFilterLevelCalculationType::RMS);
        RMSFilter.setAttackTime  (static_cast<SampleType> (0.0));
        RMSFilter.setReleaseTime (static_cast<SampleType> (50.0));
    }

    //==============================================================================
    /** Sets the threshold in dB of the noise-gate.*/
    void setThreshold (SampleType newThreshold) {noiseGate.setThreshold(newThreshold);}

    /** Sets the ratio of the noise-gate (must be higher or equal to 1).*/
    void setRatio (SampleType newRatio) {noiseGate.setRatio(newRatio);}

    /** Sets the attack time in milliseconds of the noise-gate.*/
    void setAttack (SampleType newAttack) {noiseGate.setAttack(newAttack);}

    /** Sets the release time in milliseconds of the noise-gate.*/
    void setRelease (SampleType newRelease) {noiseGate.setRelease(newRelease);}

    //==============================================================================
    /** Initialises the processor. */
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        jassert (spec.sampleRate > 0);
        jassert (spec.numChannels > 0);

        sampleRate = spec.sampleRate;

        RMSFilter.prepare (spec);
        noiseGate.prepare (spec);

        reset();
    }

    /** Resets the internal state variables of the processor. */
    void reset()
    {
        RMSFilter.reset();
        noiseGate.reset();
    }

    //==============================================================================
    /** Processes the input and output samples supplied in the processing context. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock      = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples  = outputBlock.getNumSamples();

        jassert (inputBlock.getNumChannels() == numChannels);
        jassert (inputBlock.getNumSamples() == numSamples);

        if (context.isBypassed)
        {
            outputBlock.copyFrom (inputBlock);
            return;
        }

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            auto* inputSamples  = inputBlock .getChannelPointer (channel);
            auto* outputSamples = outputBlock.getChannelPointer (channel);

            for (size_t i = 0; i < numSamples; ++i)
                outputSamples[i] = processSample ((int) channel, inputSamples[i]);
        }
    }

    /** Performs the processing operation on a single sample at a time. */
    SampleType processSample (int channel, SampleType sample)
    {
        auto env = RMSFilter.processSample (channel, sample);
        
        if (!channel)
            this->setInputRMS(float(env));
        
        SampleType modifiedSample = noiseGate.processSample(channel, sample);
        
        if (!channel)
            this->setGainReduction(juce::Decibels::gainToDecibels(modifiedSample / sample));
        
        return modifiedSample;
    }

private:
    //==============================================================================
    SampleType threshold, thresholdInverse, currentRatio;
    juce::dsp::BallisticsFilter<SampleType> RMSFilter;

    double sampleRate = 44100.0;
    juce::dsp::NoiseGate<SampleType> noiseGate;
};

/*
  ==============================================================================

    BuzzGate.h
    Created: 3 Feb 2024 11:42:29pm
    Author:  Przemysław Barski

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
template <typename SampleType>
class BuzzGate
{
public:
    BuzzGate();
//    ~BuzzGate() override;
    
    //==============================================================================
    /** Sets the threshold in dB of the noise-gate.*/
    void setThreshold (SampleType newThreshold);

    /** Sets the ratio of the noise-gate (must be higher or equal to 1).*/
    void setRatio (SampleType newRatio);

    /** Sets the attack time in milliseconds of the noise-gate.*/
    void setAttack (SampleType newAttack);

    /** Sets the release time in milliseconds of the noise-gate.*/
    void setRelease (SampleType newRelease);
    
    /** Sets the frequency ID (0 = 50 Hz, 1 = 60 Hz) of the noise gate.*/
    void setFrequencyID (int newFrequencyID);

    //==============================================================================
    /** Initialises the processor. */
    void prepare (const juce::dsp::ProcessSpec& spec);

    /** Resets the internal state variables of the processor. */
    void reset();

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

        for (size_t channel = 0; channel < numChannels && channel < 2; ++channel)
        {
            auto* inputSamples  = inputBlock .getChannelPointer (channel);
            auto* outputSamples = outputBlock.getChannelPointer (channel);

            for (size_t i = 0; i < numSamples; ++i)
                outputSamples[i] = processSample ((int) channel, inputSamples[i]);
        }
    }

    /** Performs the processing operation on a single sample at a time. */
    SampleType processSample (int channel, SampleType inputValue);

private:
    //==============================================================================
    void update();

    //==============================================================================
    SampleType threshold, thresholdInverse, currentRatio;
    juce::dsp::BallisticsFilter<SampleType> envelopeFilter, RMSFilter;
    int frequencyID;

    double sampleRate = 44100.0;
    SampleType thresholddB = -100, ratio = 10.0, attackTime = 1.0, releaseTime = 100.0, delaySampleDivider = 100;
    
    juce::dsp::DelayLine<float> delayLine;
    juce::dsp::IIR::Filter<float> buzzFilter[2][6] ;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BuzzGate)
};

/*
  ==============================================================================

    HissGate.h
    Created: 12 Feb 2024 5:14:06pm
    Author:  Przemysław Barski

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RMSMeters.h"

//==============================================================================
/*
*/
template <typename SampleType>
class HissGate  :  public RMSMeters<SampleType>
{
public:
    HissGate();
//    ~HissGate() override;
    
    //==============================================================================
    /** Sets the threshold in dB of the noise-gate.*/
    void setThreshold (SampleType newThreshold);

    /** Sets the ratio of the noise-gate (must be higher or equal to 1).*/
    void setRatio (SampleType newRatio);

    /** Sets the attack time in milliseconds of the noise-gate.*/
    void setAttack (SampleType newAttack);

    /** Sets the release time in milliseconds of the noise-gate.*/
    void setRelease (SampleType newRelease);
    
    void setCutoff (float newCutoff);
    
    float getCurrentGain();

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

    double sampleRate = 44100.0;
    SampleType thresholddB = -100, ratio = 10.0, attackTime = 1.0, releaseTime = 100.0, frequency = 2000.f;
    juce::Atomic<float> currentGain = 0.f;
    
    juce::dsp::IIR::Filter<SampleType> hissFilter[2];
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HissGate)
};

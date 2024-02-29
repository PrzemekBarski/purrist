/*
  ==============================================================================

    BuzzGate.cpp
    Created: 3 Feb 2024 11:42:29pm
    Author:  Przemysław Barski

  ==============================================================================
*/

#include <JuceHeader.h>
#include "BuzzGate.h"

//==============================================================================
//template <typename SampleType>
//BuzzGate<SampleType>::BuzzGate()
//{
//    // In your constructor, you should add any child components, and
//    // initialise any special settings that your component needs.
//
//}
//
//template <typename SampleType>
//BuzzGate<SampleType>::~BuzzGate()
//{
//}

template <typename SampleType>
BuzzGate<SampleType>::BuzzGate()
{
    update();

    RMSFilter.setLevelCalculationType (juce::dsp::BallisticsFilterLevelCalculationType::RMS);
    RMSFilter.setAttackTime  (static_cast<SampleType> (0.0));
    RMSFilter.setReleaseTime (static_cast<SampleType> (50.0));
}

template <typename SampleType>
void BuzzGate<SampleType>::setThreshold (SampleType newValue)
{
    thresholddB = newValue;
    update();
}

template <typename SampleType>
void BuzzGate<SampleType>::setRatio (SampleType newRatio)
{
    jassert (newRatio >= static_cast<SampleType> (1.0));

    ratio = newRatio;
    update();
}

template <typename SampleType>
void BuzzGate<SampleType>::setAttack (SampleType newAttack)
{
    attackTime = newAttack;
    update();
}

template <typename SampleType>
void BuzzGate<SampleType>::setRelease (SampleType newRelease)
{
    releaseTime = newRelease;
    update();
}

template <typename SampleType>
void BuzzGate<SampleType>::setFrequencyID (int newFrequencyID)
{
    frequencyID = newFrequencyID;   // 0 = 50 Hz, 1 = 60 Hz
    update();
}

template <typename SampleType>
float BuzzGate<SampleType>::getGainReduction ()
{
    return gainReduction.get();
}

//==============================================================================
template <typename SampleType>
void BuzzGate<SampleType>::prepare (const juce::dsp::ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    RMSFilter.prepare (spec);
    envelopeFilter.prepare (spec);
    
    delayLine.prepare(spec);
    delayLine.setMaximumDelayInSamples(sampleRate * 0.01);
    delayLine.setDelay(sampleRate / delaySampleDivider);
    
    int buzzFilterFreq = 50;
    for (int channel = 0; channel < 2 && channel; channel++) {
        for (int instance = 0; instance < 6; instance++) {
            *buzzFilter[channel][instance].coefficients = juce::dsp::IIR::ArrayCoefficients<float>::makePeakFilter(sampleRate, (float)buzzFilterFreq, 1000, 1);
            buzzFilter[channel][instance].prepare (spec);
            buzzFilterFreq += 50;
        }
    }

    update();
    reset();
}

template <typename SampleType>
void BuzzGate<SampleType>::reset()
{
    RMSFilter.reset();
    envelopeFilter.reset();
    delayLine.reset();
    for (int channel = 0; channel < 2 && channel; channel++) {
        for (int instance = 0; instance < 6; instance++) {
            buzzFilter[channel][instance].reset();
        }
    }
}

//==============================================================================
template <typename SampleType>
SampleType BuzzGate<SampleType>::processSample (int channel, SampleType sample)
{
    SampleType modifiedSample, delayedSample;
    
    delayLine.pushSample(channel, sample);
    delayedSample = delayLine.popSample(channel);
    delayLine.setDelay(sampleRate / delaySampleDivider);
    
    // RMS ballistics filter
    auto env = RMSFilter.processSample (channel, sample);
    
    if (!channel)
        this->setInputRMS(float(env));

    // Ballistics filter
    env = envelopeFilter.processSample (channel, env);
    
    auto minGain = juce::Decibels::decibelsToGain(static_cast<SampleType> (-24.0));
    auto gain = (env > threshold) ? static_cast<SampleType> (1.0)
                                  : std::pow (env * thresholdInverse, currentRatio - static_cast<SampleType> (1.0));
    gain = std::max(gain, minGain);
    gainReduction.set(juce::Decibels::gainToDecibels(gain));
    
    auto combGain = 1 - gain;
    modifiedSample = (sample + delayedSample * combGain) * (1 - 0.3f * combGain);
    
    int buzzFilterFreq = 50;
    for (int i = 0; i < 6; i++) {
        *buzzFilter[channel][i].coefficients = juce::dsp::IIR::ArrayCoefficients<float>::makePeakFilter(sampleRate, (float)buzzFilterFreq, 10000, gain);
        modifiedSample = buzzFilter[channel][i].processSample(modifiedSample);
        buzzFilterFreq += 50;
    }

    // Output
    return modifiedSample;
}

template <typename SampleType>
void BuzzGate<SampleType>::update()
{
    threshold = juce::Decibels::decibelsToGain (thresholddB, static_cast<SampleType> (-200.0));
    thresholdInverse = static_cast<SampleType> (1.0) / threshold;
    currentRatio = ratio;
    delaySampleDivider = frequencyID ? 120 : 100;

    envelopeFilter.setAttackTime  (attackTime);
    envelopeFilter.setReleaseTime (releaseTime);
}

//==============================================================================
template class BuzzGate<float>;
template class BuzzGate<double>;

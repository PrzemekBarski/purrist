/*
  ==============================================================================

    HissGate.cpp
    Created: 12 Feb 2024 5:14:06pm
    Author:  Przemysław Barski

  ==============================================================================
*/

#pragma once

#include "HissGate.h"

template <typename SampleType>
HissGate<SampleType>::HissGate()
{
    update();

    RMSFilter.setLevelCalculationType (juce::dsp::BallisticsFilterLevelCalculationType::RMS);
    RMSFilter.setAttackTime  (static_cast<SampleType> (0.0));
    RMSFilter.setReleaseTime (static_cast<SampleType> (50.0));
}

template <typename SampleType>
void HissGate<SampleType>::setThreshold (SampleType newValue)
{
    thresholddB = newValue;
    update();
}

template <typename SampleType>
void HissGate<SampleType>::setRatio (SampleType newRatio)
{
    jassert (newRatio >= static_cast<SampleType> (1.0));

    ratio = newRatio;
    update();
}

template <typename SampleType>
void HissGate<SampleType>::setAttack (SampleType newAttack)
{
    attackTime = newAttack;
    update();
}

template <typename SampleType>
void HissGate<SampleType>::setRelease (SampleType newRelease)
{
    releaseTime = newRelease;
    update();
}

template <typename SampleType>
void HissGate<SampleType>::setCutoff (float newCutoff)
{
    frequency = newCutoff;
    update();
}

template <typename SampleType>
float HissGate<SampleType>::getCurrentGain (int channel)
{
    return currentGain.get();
}

//==============================================================================
template <typename SampleType>
void HissGate<SampleType>::prepare (const juce::dsp::ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    RMSFilter.prepare (spec);
    envelopeFilter.prepare (spec);
    
    for (int channel = 0; channel < 2 && channel; channel++) {
        *hissFilter[channel].coefficients = juce::dsp::IIR::ArrayCoefficients<SampleType>::makeHighShelf(sampleRate, frequency, 1, 1);
        hissFilter[channel].prepare (spec);
    }

    update();
    reset();
}

template <typename SampleType>
void HissGate<SampleType>::reset()
{
    RMSFilter.reset();
    envelopeFilter.reset();
    for (int channel = 0; channel < 2 && channel; channel++) {
        hissFilter[channel].reset();
    }
}

//==============================================================================
template <typename SampleType>
SampleType HissGate<SampleType>::processSample (int channel, SampleType sample)
{
    SampleType modifiedSample = sample;
    
    // RMS ballistics filter
    auto env = RMSFilter.processSample (channel, sample);

    // Ballistics filter
    env = envelopeFilter.processSample (channel, env);
    
    auto gain = (env > threshold) ? static_cast<SampleType> (1.0)
                                  : std::pow (env * thresholdInverse, currentRatio - static_cast<SampleType> (1.0));

    auto filterGain = gain > 0.0631f ? gain : 0.0631f;
    currentGain.set(float(filterGain));
    *hissFilter[channel].coefficients = juce::dsp::IIR::ArrayCoefficients<SampleType>::makeHighShelf(sampleRate, frequency, 1, filterGain);
    modifiedSample = hissFilter[channel].processSample(modifiedSample);

    // Output
    return modifiedSample;
}

template <typename SampleType>
void HissGate<SampleType>::update()
{
    threshold = juce::Decibels::decibelsToGain (thresholddB, static_cast<SampleType> (-200.0));
    thresholdInverse = static_cast<SampleType> (1.0) / threshold;
    currentRatio = ratio;

    envelopeFilter.setAttackTime  (attackTime);
    envelopeFilter.setReleaseTime (releaseTime);
}

//==============================================================================
template class HissGate<float>;
template class HissGate<double>;

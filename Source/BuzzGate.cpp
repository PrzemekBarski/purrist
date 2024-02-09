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

//==============================================================================
template <typename SampleType>
void BuzzGate<SampleType>::prepare (const juce::dsp::ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    RMSFilter.prepare (spec);
    envelopeFilter.prepare (spec);
    
    int buzzFilterFreq = 50;
    for (int i = 0; i < 40; i++) {
        for (int j = 0; j < 2; j++) {
            *buzzFilter[j][i].coefficients = juce::dsp::IIR::ArrayCoefficients<float>::makePeakFilter(sampleRate, (float)buzzFilterFreq, 1000, 1);
            buzzFilter[j][i].prepare (spec);
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
    for (int i = 0; i < 40; i++) {
        for (int j = 0; j < 2; j++) {
            buzzFilter[j][i].reset();
        }
    }
}

//==============================================================================
template <typename SampleType>
SampleType BuzzGate<SampleType>::processSample (int channel, SampleType sample)
{
    SampleType modifiedSample = sample;
    
    // RMS ballistics filter
    auto env = RMSFilter.processSample (channel, sample);

    // Ballistics filter
    env = envelopeFilter.processSample (channel, env);
    
    auto gain = (env > threshold) ? static_cast<SampleType> (1.0)
                                  : std::pow (env * thresholdInverse, currentRatio - static_cast<SampleType> (1.0));
    
    SampleType ratioMultiplier = 2;
    int buzzFilterFreq = 50;
    for (int i = 0; i < 40; i++) {
        auto filterGain = gain;
        if (ratioMultiplier >= 1.25) {
            SampleType modifiedRatio = ratioMultiplier * currentRatio;
            filterGain = (env > threshold) ? static_cast<SampleType> (1.0)
                                          : std::pow (env * thresholdInverse, modifiedRatio - static_cast<SampleType> (1.0));
            ratioMultiplier -= static_cast<SampleType> (0.25);
        }
        
        filterGain = filterGain ? filterGain : 0.001f;
        *buzzFilter[channel][i].coefficients = juce::dsp::IIR::ArrayCoefficients<float>::makePeakFilter(sampleRate, (float)buzzFilterFreq, 10000, filterGain);
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

    envelopeFilter.setAttackTime  (attackTime);
    envelopeFilter.setReleaseTime (releaseTime);
}

//==============================================================================
template class BuzzGate<float>;
template class BuzzGate<double>;

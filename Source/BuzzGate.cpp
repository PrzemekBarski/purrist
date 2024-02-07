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
    
    int buzzFilterFreq = 50, frequency = 0;
    float filterGains[2][5];
    float attenuation[2] = {512.f, 128.f};
    
    for (int odd = 0; odd < 2; odd++) {
        for (int gain = 0; gain < 5; gain++) {
            filterGains[odd][gain] = 1.f / attenuation[odd];
            attenuation[odd] /= 2.f;
        }
    }
    
    float gain = 1;

    for (int overtone = 0; overtone < 30; overtone++) {
        for (int odd = 0; odd < 2; odd++) {
            frequency += buzzFilterFreq;
            for (int channel = 0; channel < 2; channel++) {
                if (overtone < 5) {
                    gain = filterGains[odd][overtone];
                }
                *buzzFilter[channel][odd][overtone].coefficients = juce::dsp::IIR::ArrayCoefficients<float>::makePeakFilter(sampleRate, (float)frequency, 1000, gain);
                buzzFilter[channel][odd][overtone].prepare (spec);
            }
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
    for (int overtone = 0; overtone < 30; overtone++) {
        for (int channel = 0; channel < 2; channel++) {
            for (int odd = 0; odd < 2; odd++) {
                buzzFilter[channel][odd][overtone].reset();
            }
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
    
    for (int overtone = 0; overtone < 30; overtone++) {
        for (int channel = 0; channel < 2; channel++) {
            for (int odd = 0; odd < 2; odd++) {
                modifiedSample = buzzFilter[channel][odd][overtone].processSample(modifiedSample);
            }
        }
    }
    
    // Output
    return sample * gain + modifiedSample * (1 - gain);
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

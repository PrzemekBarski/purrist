/*
  ==============================================================================

    GainReductionMeter.h
    Created: 29 Feb 2024 10:26:19pm
    Author:  Przemysław Barski

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../processors/RMSMeters.h"

enum meterRange
{
    range12 = -12,
    range24 = -24,
    range36 = -36,
    range48 = -48,
    range64 = -64,
    range80 = -80
};

class GainReductionMeter   : public juce::Component,
juce::Timer
{
public:
    GainReductionMeter(RMSMeters<float>& rmsMeters, int mRange) : rmsMeters (rmsMeters) {
        meterRange = mRange;
        startTimerHz(60);
    }
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void timerCallback() override;

protected:
    RMSMeters<float>& rmsMeters;

private:
    juce::dsp::IIR::Filter<float> filter;
    juce:: Image background;
    int padding = 8, meterRange;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainReductionMeter)
};


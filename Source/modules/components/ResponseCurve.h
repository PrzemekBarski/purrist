/*
  ==============================================================================

    ResponseCurve.h
    Created: 25 Feb 2024 5:09:03pm
    Author:  Przemysław Barski

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"

class ResponseCurve   : public juce::Component,
juce::Timer
{
public:
    ResponseCurve(PurristAudioProcessor& p) : audioProcessor (p) {
        startTimerHz(60);
    }
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    void timerCallback() override;

protected:
    PurristAudioProcessor& audioProcessor;

private:
    juce::dsp::IIR::Filter<float> filter;
    juce:: Image background;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResponseCurve)
};

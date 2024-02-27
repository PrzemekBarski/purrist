/*
  ==============================================================================

    GUI.h
    Created: 25 Feb 2024 4:55:51pm
    Author:  Przemysław Barski

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

static const juce::Font getFont()
{
    using namespace juce;
    static auto typeface = Typeface::createSystemTypefaceFor (BinaryData::WorkSansRegular_ttf, BinaryData::WorkSansRegular_ttfSize);
    return Font (typeface);
}

static const juce::Font getMediumFont()
{
    using namespace juce;
    static auto typeface = Typeface::createSystemTypefaceFor (BinaryData::WorkSansSemiBold_ttf, BinaryData::WorkSansSemiBold_ttfSize);
    return Font (typeface);
}

struct LookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider (juce::Graphics&,
                        int x, int y, int width, int height,
                        float sliderPosProportional,
                        float rotaryStartAngle,
                        float rotaryEndAngle,
                           juce::Slider&) override;
};

struct RotarySliderWithLabels : juce::Slider
{
    RotarySliderWithLabels(juce::RangedAudioParameter& param, juce::String suffix) :
        juce::Slider(juce::Slider::SliderStyle::Rotary, juce::Slider::TextEntryBoxPosition::NoTextBox),
    param(&param),
    suffix(suffix)
    {
        lnf.setColour (juce::Slider::thumbColourId, juce::Colours::white);
        setLookAndFeel(&lnf);
    }
    
    ~RotarySliderWithLabels()
    {
        setLookAndFeel(nullptr);
    }
    
    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const { return 14; }
    int getLabelTextHeight() const { return 18; }
    juce::String getDisplayString() const;
    
private:
    LookAndFeel lnf;
    juce::RangedAudioParameter* param;
    juce::String suffix;
};

/*
  ==============================================================================

    GUI.h
    Created: 25 Feb 2024 4:55:51pm
    Author:  Przemysław Barski

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../processors/RMSMeters.h"

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

struct PurristLookAndFeel : juce::LookAndFeel_V4, public juce::DeletedAtShutdown
{
    JUCE_DECLARE_SINGLETON(PurristLookAndFeel, false);
    
    void drawRotarySlider (juce::Graphics&,
                        int x, int y, int width, int height,
                        float sliderPosProportional,
                        float rotaryStartAngle,
                        float rotaryEndAngle,
                           juce::Slider&) override;
    
    void drawRMSSlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           juce::Slider::SliderStyle, juce::Slider&, float rms);
    
    void drawPointer (juce::Graphics& g, const float x, const float y, const float diameter,
                      const juce::Colour& colour, const int direction);
    
    int getSliderThumbRadius (juce::Slider&) override {return 16;};
    
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    
    void drawButtonText (juce::Graphics&, juce::TextButton&,
                         bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};

struct RotarySliderWithLabels : juce::Slider
{
    RotarySliderWithLabels(juce::RangedAudioParameter& param, juce::String suffix) :
        juce::Slider(juce::Slider::SliderStyle::Rotary, juce::Slider::TextEntryBoxPosition::NoTextBox),
    param(&param),
    suffix(suffix) {}
    
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
    PurristLookAndFeel lnf;
    juce::RangedAudioParameter* param;
    juce::String suffix;
};

struct RMSSlider : juce::Slider,
juce::Timer
{
    RMSSlider(juce::RangedAudioParameter& param, juce::Slider::SliderStyle sliderStyle, RMSMeters<float>& inputMeter) :
        juce::Slider(sliderStyle, juce::Slider::TextEntryBoxPosition::NoTextBox),
    inputRMSMeter(inputMeter),
    param(&param)
    {
        lnf.setColour (juce::Slider::thumbColourId, juce::Colours::black);
        setLookAndFeel(&lnf);
        startTimerHz(60);
    }
    
    ~RMSSlider()
    {
        setLookAndFeel(nullptr);
    }
    
    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const { return 14; }
    int getLabelTextHeight() const { return 18; }
    juce::String getDisplayString() const;
    void timerCallback() override;
    
private:
    RMSMeters<float>& inputRMSMeter;
    PurristLookAndFeel lnf;
    juce::RangedAudioParameter* param;
};

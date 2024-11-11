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

struct PurristLookAndFeelShared : juce::LookAndFeel_V4
{
    void drawButtonText (juce::Graphics&, juce::TextButton&,
                         bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    
    virtual juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override = 0;
    virtual juce::Font getTextButtonFont () = 0;
};

struct PurristLookAndFeel : PurristLookAndFeelShared, public juce::DeletedAtShutdown
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
    
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
    juce::Font getTextButtonFont () override;
    
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

private:
    int getSliderTextHeight() const { return 14; }
    int getSliderLabelTextHeight() const { return 18; }
};

struct PurristHelpButtonLNF : PurristLookAndFeelShared, public juce::DeletedAtShutdown
{
    JUCE_DECLARE_SINGLETON(PurristHelpButtonLNF, false);
    
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
    juce::Font getTextButtonFont () override;
};

struct RotarySliderWithLabels : juce::Component
{
    RotarySliderWithLabels(juce::RangedAudioParameter& param, juce::String label, juce::String suffix, int decimalPlaces) :
    slider(juce::Slider::SliderStyle::Rotary, juce::Slider::TextEntryBoxPosition::NoTextBox),
    param(&param),
    label(label),
    suffix(suffix),
    decimalPlaces(decimalPlaces)
    {
        slider.setScrollWheelEnabled(false);
        slider.setRotaryParameters(juce::degreesToRadians(270.f),
                                juce::degreesToRadians(90.f) + juce::MathConstants<float>::twoPi,
                                true);
    }
    
    ~RotarySliderWithLabels()
    {
        setLookAndFeel(nullptr);
    }
    
    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> calculateBounds(juce::Rectangle<int> inputBounds);
    juce::Slider& getSlider();
    juce::String getDisplayString() const;
    
private:
    juce::Slider slider;
    juce::RangedAudioParameter* param;
    juce::String label, suffix;
    int decimalPlaces;
};

struct RMSSlider : juce::Slider,
juce::Timer
{
    RMSSlider(juce::RangedAudioParameter& param, juce::Slider::SliderStyle sliderStyle, juce::String label, RMSMeters<float>& inputMeter) :
        juce::Slider(sliderStyle, juce::Slider::TextEntryBoxPosition::NoTextBox),
    inputRMSMeter(inputMeter),
    label(label),
    param(&param)
    {
        setScrollWheelEnabled(false);
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
    juce::String label;
    PurristLookAndFeel lnf;
    juce::RangedAudioParameter* param;
};

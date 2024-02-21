/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/

using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;

struct LookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider (juce::Graphics&,
                        int x, int y, int width, int height,
                        float sliderPosProportional,
                        float rotaryStartAngle,
                        float rotaryEndAngle,
                        juce::Slider&) override {}
};

struct RotarySliderWithLabels : juce::Slider
{
    RotarySliderWithLabels(juce::RangedAudioParameter& param, juce::String suffix) :
        juce::Slider(juce::Slider::SliderStyle::Rotary, juce::Slider::TextEntryBoxPosition::NoTextBox),
    param(&param),
    suffix(suffix)
    {
        setLookAndFeel(&lnf);
    }
    
    ~RotarySliderWithLabels()
    {
        setLookAndFeel(nullptr);
    }
    
//    void paint(juce::Graphics& g) override;
//    juce::Rectangle<int> getSliderBounds() const;
//    int getTextHeight() const { return 14; }
//    juce::String getDisplayString() const;
    
private:
    LookAndFeel lnf;
    juce::RangedAudioParameter* param;
    juce::String suffix;
};

class SectionComponent   : public juce::Component
{
public:
    SectionComponent(PurristAudioProcessor& p) : audioProcessor (p) {}

    void paint (juce::Graphics& g) override {
        
        auto area = getLocalBounds();
        auto shadowOffset = 20;
        
        area.removeFromRight(shadowOffset);
        area.removeFromBottom(shadowOffset);
        
        shadow.drawForRectangle(g, area);
        g.setColour (juce::Colours::white);
        g.fillRect(area.toFloat());
        
        auto sectionPaddingX = 16;
        auto sectionPaddingY = 32;
        
        area.removeFromTop(sectionPaddingY);
        area.removeFromRight(sectionPaddingX);
        area.removeFromBottom(sectionPaddingY);
        area.removeFromLeft(sectionPaddingX);
        
        paintSection(g, area);
    }
    
    virtual std::vector<juce::Component*> getComponents() = 0;
    
protected:
    PurristAudioProcessor& audioProcessor;

private:
    juce::DropShadow shadow = juce::DropShadow(juce::Colours::black, 1, juce::Point<int>(20, 20));
    
    virtual void paintSection(juce::Graphics& g, juce::Rectangle<int> area) {};
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SectionComponent)
};

class BuzzComponent   : public SectionComponent
{
public:
    BuzzComponent(PurristAudioProcessor& p)
        : SectionComponent(p),
    buzzThresholdSliderAttachment(audioProcessor.apvts, "buzz_threshold", buzzThresholdSlider),
    buzzRatioSliderAttachment(audioProcessor.apvts, "buzz_ratio", buzzRatioSlider),
    buzzFreqSliderAttachment(audioProcessor.apvts, "buzz_frequency", buzzFreqSlider),
    buzzThresholdSlider(*audioProcessor.apvts.getParameter("buzz_threshold"), "dB"),
    buzzRatioSlider(*audioProcessor.apvts.getParameter("buzz_ratio"), ":1"),
    buzzFreqSlider(*audioProcessor.apvts.getParameter("buzz_frequency"), "") {}
    
    std::vector<juce::Component*> getComponents() override;
private:
    void paintSection(juce::Graphics& g, juce::Rectangle<int> area) override;
    
    Attachment  buzzThresholdSliderAttachment, buzzRatioSliderAttachment,
                buzzFreqSliderAttachment;
    
    RotarySliderWithLabels  buzzThresholdSlider, buzzRatioSlider,
                            buzzFreqSlider;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BuzzComponent)
};

class HissComponent   : public SectionComponent,
juce::Timer
{
public:
    HissComponent(PurristAudioProcessor& p)
        : SectionComponent(p),
    hissThresholdSliderAttachment(audioProcessor.apvts, "hiss_threshold", hissThresholdSlider),
    hissRatioSliderAttachment(audioProcessor.apvts, "hiss_ratio", hissRatioSlider),
    hissCutoffSliderAttachment(audioProcessor.apvts, "hiss_cutoff", hissCutoffSlider),
    hissThresholdSlider(*audioProcessor.apvts.getParameter("hiss_threshold"), "dB"),
    hissRatioSlider(*audioProcessor.apvts.getParameter("hiss_ratio"), ":1"),
    hissCutoffSlider(*audioProcessor.apvts.getParameter("hiss_cutoff"), "Hz")
    {
        startTimerHz(60);
    }
    
    std::vector<juce::Component*> getComponents() override;
    void timerCallback() override;
private:
    void paintSection(juce::Graphics& g, juce::Rectangle<int> area) override;
    juce::dsp::IIR::Filter<float> filter;
    
    Attachment  hissThresholdSliderAttachment, hissRatioSliderAttachment,
                hissCutoffSliderAttachment;
    
    RotarySliderWithLabels  hissThresholdSlider, hissRatioSlider,
                            hissCutoffSlider;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HissComponent)
};

class NoiseComponent   : public SectionComponent
{
public:
    NoiseComponent(PurristAudioProcessor& p)
        : SectionComponent(p),
    noiseThresholdSliderAttachment(audioProcessor.apvts, "noise_threshold", noiseThresholdSlider),
    noiseRatioSliderAttachment(audioProcessor.apvts, "noise_ratio", noiseRatioSlider),
    noiseReleaseSliderAttachment(audioProcessor.apvts, "noise_release", noiseReleaseSlider),
    noiseThresholdSlider(*audioProcessor.apvts.getParameter("noise_threshold"), "dB"),
    noiseRatioSlider(*audioProcessor.apvts.getParameter("noise_ratio"), ":1"),
    noiseReleaseSlider(*audioProcessor.apvts.getParameter("noise_release"), "mS") {}
    
    std::vector<juce::Component*> getComponents() override;
private:
    void paintSection(juce::Graphics& g, juce::Rectangle<int> area) override {};
    juce::dsp::IIR::Filter<float> filter;
    
    Attachment  noiseThresholdSliderAttachment, noiseRatioSliderAttachment,
                noiseReleaseSliderAttachment;
    
    RotarySliderWithLabels  noiseThresholdSlider, noiseRatioSlider,
                            noiseReleaseSlider;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoiseComponent)
};

class PurristAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    PurristAudioProcessorEditor (PurristAudioProcessor&);
    ~PurristAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    juce::Atomic<bool> filterChanged { false };

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    
    PurristAudioProcessor& audioProcessor;
    
    BuzzComponent buzzSection;
    HissComponent hissSection;
    NoiseComponent noiseSection;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PurristAudioProcessorEditor)
};

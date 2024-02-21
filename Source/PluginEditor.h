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
    SectionComponent() {}

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

private:
    juce::DropShadow shadow = juce::DropShadow(juce::Colours::black, 1, juce::Point<int>(20, 20));
    
    virtual void paintSection(juce::Graphics& g, juce::Rectangle<int> area) {};
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SectionComponent)
};

class BuzzComponent   : public SectionComponent
{
public:
    BuzzComponent() {}
private:
    void paintSection(juce::Graphics& g, juce::Rectangle<int> area) override;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BuzzComponent)
};

class HissComponent   : public SectionComponent,
juce::Timer
{
public:
    HissComponent() {
        startTimerHz(60);
    }
    void timerCallback() override;
private:
    void paintSection(juce::Graphics& g, juce::Rectangle<int> area) override;
    juce::dsp::IIR::Filter<float> filter;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HissComponent)
};

class PurristAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    PurristAudioProcessorEditor (PurristAudioProcessor&);
    ~PurristAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    PurristAudioProcessor& audioProcessor;
    
    juce::Atomic<bool> filterChanged { false };
    
    RotarySliderWithLabels  buzzThresholdSlider, buzzRatioSlider,
                            buzzFreqSlider, hissThresholdSlider, hissRatioSlider,
                            hissCutoffSlider, noiseThresholdSlider, noiseRatioSlider,
                            noiseReleaseSlider;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    
    SectionComponent noiseSection;
    BuzzComponent buzzSection;
    HissComponent hissSection;
    
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    
    Attachment  buzzThresholdSliderAttachment, buzzRatioSliderAttachment,
                buzzFreqSliderAttachment, hissThresholdSliderAttachment, hissRatioSliderAttachment,
                hissCutoffSliderAttachment, noiseThresholdSliderAttachment, noiseRatioSliderAttachment,
                noiseReleaseSliderAttachment;
    
    std::vector<juce::Component*> getComponents();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PurristAudioProcessorEditor)
};

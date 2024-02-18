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

struct RotarySliderWithLabels : juce::Slider
{
    RotarySliderWithLabels() : juce::Slider(juce::Slider::SliderStyle::Rotary, juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
        
    }
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
        
        paintSection(area);
    }

private:
    juce::DropShadow shadow = juce::DropShadow(juce::Colours::black, 1, juce::Point<int>(20, 20));
    
    virtual void paintSection(juce::Rectangle<int> area) {};
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SectionComponent)
};

class BuzzComponent   : public SectionComponent
{
public:
    BuzzComponent() {}
private:
    void paintSection(juce::Rectangle<int>) override;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BuzzComponent)
};

class PurristAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    PurristAudioProcessorEditor (PurristAudioProcessor&);
    ~PurristAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    RotarySliderWithLabels  buzzThresholdSlider, buzzRatioSlider,
                            buzzFreqSlider, hissThresholdSlider, hissRatioSlider,
                            hissCutoffSlider, noiseThresholdSlider, noiseRatioSlider,
                            noiseReleaseSlider;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PurristAudioProcessor& audioProcessor;
    
    SectionComponent hissSection, noiseSection;
    BuzzComponent buzzSection;
    
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    
    Attachment  buzzThresholdSliderAttachment, buzzRatioSliderAttachment,
                buzzFreqSliderAttachment, hissThresholdSliderAttachment, hissRatioSliderAttachment,
                hissCutoffSliderAttachment, noiseThresholdSliderAttachment, noiseRatioSliderAttachment,
                noiseReleaseSliderAttachment;
    
    std::vector<juce::Component*> getComponents();
    
    MonoChain monoChain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PurristAudioProcessorEditor)
};

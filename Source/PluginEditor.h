/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI.h"

//==============================================================================
/**
*/

using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;

class SectionComponent   : public juce::Component
{
public:
    SectionComponent(PurristAudioProcessor& p) : audioProcessor (p) {}

    void paint (juce::Graphics& g) override {
        
        auto bounds = getLocalBounds();
        
        bounds.removeFromRight(shadowOffset);
        bounds.removeFromBottom(shadowOffset);
        
        shadow.drawForRectangle(g, bounds);
        g.setColour (juce::Colours::white);
        g.fillRect(bounds.toFloat());
        
        auto sectionPaddingX = 16;
        auto sectionPaddingY = 32;
        
        bounds.removeFromTop(sectionPaddingY);
        bounds.removeFromRight(sectionPaddingX);
        bounds.removeFromBottom(sectionPaddingY);
        bounds.removeFromLeft(sectionPaddingX);
        
        paintSection(g);
    }
    
    juce::Rectangle<int> getSectionArea()
    {
        auto area = getLocalBounds();
        
        area.removeFromRight(shadowOffset);
        area.removeFromBottom(shadowOffset);
        area.removeFromTop(sectionPaddingY);
        area.removeFromRight(sectionPaddingX);
        area.removeFromBottom(sectionPaddingY);
        area.removeFromLeft(sectionPaddingX);
        
        return area;
    }
    
    virtual std::vector<juce::Component*> getComponents()
    {
        DBG("parent getComponents");
        return {};
    }
    
protected:
    PurristAudioProcessor& audioProcessor;

private:
    int shadowOffset = 10, sectionPaddingX = 16, sectionPaddingY = 32;
    juce::DropShadow shadow = juce::DropShadow(juce::Colours::black, 1, juce::Point<int>(shadowOffset, shadowOffset));
    
    virtual void paintSection(juce::Graphics& g) {};
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SectionComponent)
};

class BuzzComponent   : public SectionComponent
{
public:
    BuzzComponent(PurristAudioProcessor& p)
        : SectionComponent(p),
    buzzThresholdSlider(*audioProcessor.apvts.getParameter("buzz_threshold"), "dB"),
    buzzRatioSlider(*audioProcessor.apvts.getParameter("buzz_ratio"), ":1"),
    buzzFreqSlider(*audioProcessor.apvts.getParameter("buzz_frequency"), ""),
    buzzThresholdSliderAttachment(audioProcessor.apvts, "buzz_threshold", buzzThresholdSlider),
    buzzRatioSliderAttachment(audioProcessor.apvts, "buzz_ratio", buzzRatioSlider),
    buzzFreqSliderAttachment(audioProcessor.apvts, "buzz_frequency", buzzFreqSlider)
    {
        for (auto* component : getComponents()) {
            addAndMakeVisible(component);
        }
    }
    
    std::vector<juce::Component*> getComponents() override;
private:
    void paintSection(juce::Graphics& g) override;
    
    RotarySliderWithLabels  buzzThresholdSlider, buzzRatioSlider,
                            buzzFreqSlider;
    
    Attachment  buzzThresholdSliderAttachment, buzzRatioSliderAttachment,
                buzzFreqSliderAttachment;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BuzzComponent)
};

class HissComponent   : public SectionComponent,
juce::Timer
{
public:
    HissComponent(PurristAudioProcessor& p)
        : SectionComponent(p),
    hissThresholdSlider(*audioProcessor.apvts.getParameter("hiss_threshold"), "dB"),
    hissRatioSlider(*audioProcessor.apvts.getParameter("hiss_ratio"), ":1"),
    hissCutoffSlider(*audioProcessor.apvts.getParameter("hiss_cutoff"), "Hz"),
    hissThresholdSliderAttachment(audioProcessor.apvts, "hiss_threshold", hissThresholdSlider),
    hissRatioSliderAttachment(audioProcessor.apvts, "hiss_ratio", hissRatioSlider),
    hissCutoffSliderAttachment(audioProcessor.apvts, "hiss_cutoff", hissCutoffSlider)
    {
        startTimerHz(60);
        for (auto* component : getComponents()) {
            addAndMakeVisible(component);
        }
    }
    
    void resized() override;
    std::vector<juce::Component*> getComponents() override;
    void timerCallback() override;
private:
    void paintSection(juce::Graphics& g) override;
    juce::dsp::IIR::Filter<float> filter;
    juce:: Image background;
    
    RotarySliderWithLabels  hissThresholdSlider, hissRatioSlider,
                            hissCutoffSlider;
    
    Attachment  hissThresholdSliderAttachment, hissRatioSliderAttachment,
                hissCutoffSliderAttachment;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HissComponent)
};

class NoiseComponent   : public SectionComponent
{
public:
    NoiseComponent(PurristAudioProcessor& p)
        : SectionComponent(p),
    noiseThresholdSlider(*audioProcessor.apvts.getParameter("noise_threshold"), "dB"),
    noiseRatioSlider(*audioProcessor.apvts.getParameter("noise_ratio"), ":1"),
    noiseReleaseSlider(*audioProcessor.apvts.getParameter("noise_release"), "mS"),
    noiseThresholdSliderAttachment(audioProcessor.apvts, "noise_threshold", noiseThresholdSlider),
    noiseRatioSliderAttachment(audioProcessor.apvts, "noise_ratio", noiseRatioSlider),
    noiseReleaseSliderAttachment(audioProcessor.apvts, "noise_release", noiseReleaseSlider)
    {
        for (auto* component : getComponents()) {
            addAndMakeVisible(component);
        }
    }
    
    std::vector<juce::Component*> getComponents() override;
private:
    void paintSection(juce::Graphics& g) override {};
    juce::dsp::IIR::Filter<float> filter;
    
    RotarySliderWithLabels  noiseThresholdSlider, noiseRatioSlider,
                            noiseReleaseSlider;
    
    Attachment  noiseThresholdSliderAttachment, noiseRatioSliderAttachment,
                noiseReleaseSliderAttachment;
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

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "modules/components/GUI.h"
#include "modules/components/ResponseCurve.h"
#include "modules/components/GainReductionMeter.h"

//==============================================================================
/**
*/

using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;

static const juce::Font getDisplayFont()
{
    using namespace juce;
    static auto typeface = Typeface::createSystemTypefaceFor (BinaryData::RighteousRegular_ttf, BinaryData::RighteousRegular_ttfSize);
    return Font (typeface);
}

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
        
        bounds.removeFromTop(sectionPaddingTop);
        bounds.removeFromRight(sectionPaddingX);
        bounds.removeFromBottom(sectionPaddingBottom);
        bounds.removeFromLeft(sectionPaddingX);
        
        auto titleField = bounds.removeFromTop(titleSize * 1.15f);
        
        g.setColour (juce::Colours::black);
        g.fillRect(bounds.removeFromTop(2));
        
//        g.setColour(juce::Colours::red);
//        g.drawRect(titleField);
        
        g.setFont(getDisplayFont());
        g.setFont(titleSize);
        g.setColour (juce::Colours::black);
        g.drawFittedText(title, titleField, juce::Justification::left, 1);
        
        paintSection(g);
    }
    
    juce::Rectangle<int> getSectionArea()
    {
        auto area = getLocalBounds();
        
        area.removeFromRight(shadowOffset);
        area.removeFromBottom(shadowOffset);
        area.removeFromTop(sectionPaddingTop);
        area.removeFromRight(sectionPaddingX);
        area.removeFromBottom(sectionPaddingBottom);
        area.removeFromLeft(sectionPaddingX);
        area.removeFromTop(titleSize * 1.15f + 16);
        
        return area;
    }
    
    virtual std::vector<juce::Component*> getComponents()
    {
        DBG("parent getComponents");
        return {};
    }
    
protected:
    PurristAudioProcessor& audioProcessor;
    juce::String title;

private:
    int shadowOffset = 5, sectionPaddingX = 24, sectionPaddingBottom= 32, sectionPaddingTop = 16, titleSize = 32;
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
    gainReductionMeter(p.chain[0].get<ChainPositions::buzzGate>(), meterRange::range24),
    ratioSlider(*audioProcessor.apvts.getParameter("buzz_ratio"), ":1"),
    freqSlider(*audioProcessor.apvts.getParameter("buzz_frequency"), ""),
    thresholdSlider(*audioProcessor.apvts.getParameter("buzz_threshold"),
                        juce::Slider::SliderStyle::LinearVertical,
                        p.chain[0].get<ChainPositions::buzzGate>()),
    thresholdSliderAttachment(audioProcessor.apvts, "buzz_threshold", thresholdSlider),
    ratioSliderAttachment(audioProcessor.apvts, "buzz_ratio", ratioSlider),
    freqSliderAttachment(audioProcessor.apvts, "buzz_frequency", freqSlider)
    {
        for (auto* component : getComponents()) {
            addAndMakeVisible(component);
        }
        title = "Buzz";
    }
    
    std::vector<juce::Component*> getComponents() override;
private:
    void paintSection(juce::Graphics& g) override;
    
    GainReductionMeter gainReductionMeter;
    
    RotarySliderWithLabels  ratioSlider, freqSlider;
    
    RMSSlider thresholdSlider;
    
    Attachment  thresholdSliderAttachment, ratioSliderAttachment,
                freqSliderAttachment;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BuzzComponent)
};

class HissComponent   : public SectionComponent
{
public:
    HissComponent(PurristAudioProcessor& p)
        : SectionComponent(p), responseCurve(p),
    ratioSlider(*audioProcessor.apvts.getParameter("hiss_ratio"), ": 1"),
    cutoffSlider(*audioProcessor.apvts.getParameter("hiss_cutoff"), "Hz"),
    thresholdSlider(*audioProcessor.apvts.getParameter("hiss_threshold"),
                        juce::Slider::SliderStyle::LinearHorizontal,
                        p.chain[0].get<ChainPositions::hissGate>()),
    thresholdSliderAttachment(audioProcessor.apvts, "hiss_threshold", thresholdSlider),
    ratioSliderAttachment(audioProcessor.apvts, "hiss_ratio", ratioSlider),
    cutoffSliderAttachment(audioProcessor.apvts, "hiss_cutoff", cutoffSlider)
    {
        for (auto* component : getComponents()) {
            addAndMakeVisible(component);
        }
        title = "Hiss";
    }
    
    void resized() override;
    std::vector<juce::Component*> getComponents() override;
private:
    void paintSection(juce::Graphics& g) override;
    
    ResponseCurve responseCurve;
    
    RotarySliderWithLabels  ratioSlider, cutoffSlider;
    
    RMSSlider thresholdSlider;
    
    Attachment  thresholdSliderAttachment, ratioSliderAttachment,
                cutoffSliderAttachment;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HissComponent)
};

class NoiseComponent   : public SectionComponent
{
public:
    NoiseComponent(PurristAudioProcessor& p)
        : SectionComponent(p),
    gainReductionMeter(p.chain[0].get<ChainPositions::noiseGate>(), meterRange::range48),
    ratioSlider(*audioProcessor.apvts.getParameter("noise_ratio"), ":1"),
    releaseSlider(*audioProcessor.apvts.getParameter("noise_release"), "mS"),
    thresholdSlider(*audioProcessor.apvts.getParameter("noise_threshold"),
                        juce::Slider::SliderStyle::LinearVertical,
                        p.chain[0].get<ChainPositions::noiseGate>()),
    thresholdSliderAttachment(audioProcessor.apvts, "noise_threshold", thresholdSlider),
    ratioSliderAttachment(audioProcessor.apvts, "noise_ratio", ratioSlider),
    releaseSliderAttachment(audioProcessor.apvts, "noise_release", releaseSlider)
    {
        for (auto* component : getComponents()) {
            addAndMakeVisible(component);
        }
        title = "Noise";
    }
    
    void resized() override;
    std::vector<juce::Component*> getComponents() override;
private:
    void paintSection(juce::Graphics& g) override {};
    juce::dsp::IIR::Filter<float> filter;
    
    GainReductionMeter gainReductionMeter;
    
    RotarySliderWithLabels  ratioSlider,
                            releaseSlider;
    
    RMSSlider thresholdSlider;
    
    Attachment  thresholdSliderAttachment, ratioSliderAttachment,
                releaseSliderAttachment;
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
    
    std::unique_ptr<juce::Drawable> logo, logoShadow, pluginIcon, pluginIconShadow;
    juce::DrawableText pluginLogo, pluginLogoShadow;
    
    juce::Rectangle<int> debugRect1, debugRect2;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PurristAudioProcessorEditor)
};

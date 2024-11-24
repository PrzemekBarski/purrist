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
using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

//==============================================================================
/** Returns the heading / title font
*/
static const juce::Font getDisplayFont()
{
    using namespace juce;
    static auto typeface = Typeface::createSystemTypefaceFor (BinaryData::RighteousRegular_ttf, BinaryData::RighteousRegular_ttfSize);
    return Font (typeface); // TODO: update with FontOptions constructor
}

//==============================================================================
/**
    A class for plugin section card, including the title and on / off button
*/
class SectionComponent   : public juce::Component
{
public:
    //==============================================================================
    /**
        @param p    Audio processor to use in the section
    */
    SectionComponent(PurristAudioProcessor& p) : audioProcessor (p) {
        PurristLookAndFeel* lnf = PurristLookAndFeel::getInstance();
        
        setLookAndFeel(lnf);
        addAndMakeVisible(onButton);
    }
    
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
        
        // Title underline
        g.setColour (juce::Colours::black);
        g.fillRect(bounds.removeFromTop(2));
        
        onButton.setBounds(titleField.removeFromRight(titleSize));
        
        g.setFont(getDisplayFont());
        g.setFont(titleSize);
        g.setColour (juce::Colours::black);
        g.drawFittedText(title, titleField, juce::Justification::left, 1);
        
        paintSection(g);
    }
    
    //==============================================================================
    /** Returns the editable section area rectangle
    */
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
    
    //==============================================================================
    /** Returns all child components to add to the section and make visible
    */
    virtual std::vector<juce::Component*> getComponents()
    {
        return {};
    }
    
protected:
    PurristAudioProcessor& audioProcessor;
    juce::String title;
    juce::ToggleButton onButton;

private:
    int shadowOffset = 5, sectionPaddingX = 24, sectionPaddingBottom= 32, sectionPaddingTop = 16, titleSize = 32;
    juce::DropShadow shadow = juce::DropShadow(juce::Colours::black, 1, juce::Point<int>(shadowOffset, shadowOffset));
    
    /** A method to paint the editable content of the section

        @param g    the graphics context that must be used to do the drawing operations.
    */
    virtual void paintSection(juce::Graphics& g) {};
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SectionComponent)
};

//==============================================================================
/**
    A class for the Buzz section of the plugin
*/
class BuzzComponent   : public SectionComponent
{
public:
    BuzzComponent(PurristAudioProcessor& p)
        : SectionComponent(p),
    gainReductionMeter(p.chain[0].get<ChainPositions::buzzGate>(), meterRange::range12),
    ratioSlider(*audioProcessor.apvts.getParameter("buzz_ratio"), "Ratio", ": 1", 1),
    thresholdSlider(*audioProcessor.apvts.getParameter("buzz_threshold"),
                        juce::Slider::SliderStyle::LinearVertical,
                        "Threshold",
                        p.chain[0].get<ChainPositions::buzzGate>()),
    thresholdSliderAttachment(audioProcessor.apvts, "buzz_threshold", thresholdSlider),
    ratioSliderAttachment(audioProcessor.apvts, "buzz_ratio", ratioSlider.getSlider()),
    freqButtonAttachment(audioProcessor.apvts, "buzz_frequency", freqButton[1]),
    onButtonAttachment(audioProcessor.apvts, "buzz_on", onButton)
    {
        float freqOption = audioProcessor.apvts.getRawParameterValue("buzz_frequency")->load();
        freqButton[0].setButtonText("50 Hz");
        freqButton[0].setConnectedEdges(juce::TextButton::ConnectedOnRight);
        
        freqButton[1].setButtonText("60 Hz");
        freqButton[1].setConnectedEdges(juce::TextButton::ConnectedOnLeft);
        
        for (int i = 0; i < 2; i++) {
            freqButton[i].setRadioGroupId (10);
            freqButton[i].setClickingTogglesState(true);
            
            if (freqOption == i)
                freqButton[i].setToggleState (true, juce::dontSendNotification);
        }
        
        for (auto* component : getComponents()) {
            addAndMakeVisible(component);
        }
        
        title = "Buzz";
    }
    
    std::vector<juce::Component*> getComponents() override;
private:
    void paintSection(juce::Graphics& g) override;
    
    GainReductionMeter gainReductionMeter;
    RotarySliderWithLabels  ratioSlider;
    RMSSlider thresholdSlider;
    juce::TextButton freqButton[2];
    
    Attachment  thresholdSliderAttachment, ratioSliderAttachment;
    
    ButtonAttachment freqButtonAttachment, onButtonAttachment;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BuzzComponent)
};

//==============================================================================
/**
    A class for the Hiss section of the plugin
*/
class HissComponent   : public SectionComponent
{
public:
    HissComponent(PurristAudioProcessor& p)
        : SectionComponent(p), responseCurve(p),
    ratioSlider(*audioProcessor.apvts.getParameter("hiss_ratio"), "Ratio", ": 1", 1),
    cutoffSlider(*audioProcessor.apvts.getParameter("hiss_cutoff"), "Cutoff", "Hz", 1),
    thresholdSlider(*audioProcessor.apvts.getParameter("hiss_threshold"),
                        juce::Slider::SliderStyle::LinearHorizontal,
                        "Threshold",
                        p.chain[0].get<ChainPositions::hissGate>()),
    thresholdSliderAttachment(audioProcessor.apvts, "hiss_threshold", thresholdSlider),
    ratioSliderAttachment(audioProcessor.apvts, "hiss_ratio", ratioSlider.getSlider()),
    cutoffSliderAttachment(audioProcessor.apvts, "hiss_cutoff", cutoffSlider.getSlider()),
    onButtonAttachment(audioProcessor.apvts, "hiss_on", onButton)
    {
        for (auto* component : getComponents()) {
            addAndMakeVisible(component);
        }
        title = "Fizz";
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
    ButtonAttachment onButtonAttachment;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HissComponent)
};

//==============================================================================
/**
    A class for the Noise section of the plugin
*/
class NoiseComponent   : public SectionComponent
{
public:
    NoiseComponent(PurristAudioProcessor& p)
        : SectionComponent(p),
    gainReductionMeter(p.chain[0].get<ChainPositions::noiseGate>(), meterRange::range48),
    ratioSlider(*audioProcessor.apvts.getParameter("noise_ratio"), "Ratio", ": 1", 1),
    releaseSlider(*audioProcessor.apvts.getParameter("noise_release"), "Release", "mS", 0),
    thresholdSlider(*audioProcessor.apvts.getParameter("noise_threshold"),
                        juce::Slider::SliderStyle::LinearVertical,
                        "Threshold",
                        p.chain[0].get<ChainPositions::noiseGate>()),
    thresholdSliderAttachment(audioProcessor.apvts, "noise_threshold", thresholdSlider),
    ratioSliderAttachment(audioProcessor.apvts, "noise_ratio", ratioSlider.getSlider()),
    releaseSliderAttachment(audioProcessor.apvts, "noise_release", releaseSlider.getSlider()),
    onButtonAttachment(audioProcessor.apvts, "noise_on", onButton)
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
    ButtonAttachment onButtonAttachment;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoiseComponent)
};

//==============================================================================
/**
    A class for the plugin editor
*/
class PurristAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    PurristAudioProcessorEditor (PurristAudioProcessor&);
    ~PurristAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    
    juce::Atomic<bool> filterChanged { false };

private:
    PurristAudioProcessor& audioProcessor;
    Component contentComponent;
    juce::Viewport mainViewport;
    
    BuzzComponent buzzSection;
    HissComponent hissSection;
    NoiseComponent noiseSection;
    
    std::unique_ptr<juce::Drawable> logo, logoShadow, pluginIcon, pluginIconShadow;
    juce::DrawableText pluginLogo, pluginLogoShadow;
    juce::TextButton helpButton;
    juce::URL helpURL{"https://straycataudio.netlify.app/purrist/user-manual/"};
    
    juce::Rectangle<int> debugRect1, debugRect2;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PurristAudioProcessorEditor)
};

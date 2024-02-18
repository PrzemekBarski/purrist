/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PurristAudioProcessorEditor::PurristAudioProcessorEditor (PurristAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
buzzThresholdSliderAttachment(audioProcessor.apvts, "buzz_threshold", buzzThresholdSlider),
buzzRatioSliderAttachment(audioProcessor.apvts, "buzz_ratio", buzzRatioSlider),
buzzFreqSliderAttachment(audioProcessor.apvts, "buzz_frequency", buzzFreqSlider),
hissThresholdSliderAttachment(audioProcessor.apvts, "hiss_threshold", hissThresholdSlider),
hissRatioSliderAttachment(audioProcessor.apvts, "hiss_ratio", hissRatioSlider),
hissCutoffSliderAttachment(audioProcessor.apvts, "hiss_cutoff", hissCutoffSlider),
noiseThresholdSliderAttachment(audioProcessor.apvts, "noise_threshold", noiseThresholdSlider),
noiseRatioSliderAttachment(audioProcessor.apvts, "noise_ratio", noiseRatioSlider),
noiseReleaseSliderAttachment(audioProcessor.apvts, "noise_release", noiseReleaseSlider)
//buzzThresholdSlider(*audioProcessor.apvts.getParameter("buzz_threshold"), "dB"),
//buzzRatioSlider(*audioProcessor.apvts.getParameter("buzz_ratio"), ":1"),
//buzzFreqSlider(*audioProcessor.apvts.getParameter("buzz_frequency"), ""),
//hissThresholdSlider(*audioProcessor.apvts.getParameter("hiss_threshold"), "dB"),
//hissRatioSlider(*audioProcessor.apvts.getParameter("hiss_ratio"), ":1"),
//hissCutoffSlider(*audioProcessor.apvts.getParameter("hiss_cutoff"), "Hz"),
//noiseThresholdSlider(*audioProcessor.apvts.getParameter("noise_threshold"), "dB"),
//noiseRatioSlider(*audioProcessor.apvts.getParameter("noise_ratio"), ":1"),
//noiseReleaseSlider(*audioProcessor.apvts.getParameter("noise_release"), "mS")
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    addAndMakeVisible (buzzSection);
    addAndMakeVisible (hissSection);
    addAndMakeVisible (noiseSection);
    
    for (auto* component : getComponents()) {
        addAndMakeVisible(component);
    }
    setSize (1024, 500);
    setResizable (true, true);
    setResizeLimits(1024, 500, 9999, 9999);
}

PurristAudioProcessorEditor::~PurristAudioProcessorEditor(){}

//==============================================================================

std::vector<juce::Component*> PurristAudioProcessorEditor::getComponents()
{
    return
    {
        &buzzThresholdSlider,
        &buzzRatioSlider,
        &buzzFreqSlider,
        &hissThresholdSlider,
        &hissRatioSlider,
        &hissCutoffSlider,
        &noiseThresholdSlider,
        &noiseRatioSlider,
        &noiseReleaseSlider
    };
}

void PurristAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(79, 85, 117));
}

void PurristAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    
    auto paddingX = 20;
    auto maxHeight = 400;
    auto paddingY = (area.getHeight() - maxHeight) / 2;
    
    area.removeFromTop(paddingY);
    area.removeFromRight(paddingX);
    area.removeFromBottom(paddingY);
    area.removeFromLeft(paddingX);
    
    auto gap = 6;
    auto sectionWidth = area.getWidth() / 3 - gap * 2 / 3;
    
    buzzSection.setBounds(area.removeFromLeft(sectionWidth));
    auto buzzSectionArea = buzzSection.getBounds();
    
    area.removeFromLeft(gap);
    
    hissSection.setBounds(area.removeFromLeft(sectionWidth));
    auto hissSectionArea =hissSection.getBounds();
    
    auto responseArea = hissSectionArea.removeFromBottom(hissSectionArea.getHeight() / 3);
    
    area.removeFromLeft(gap);
    noiseSection.setBounds(area.removeFromLeft(sectionWidth));
}

void BuzzComponent::paintSection(juce::Rectangle<int> area)
{
    PurristAudioProcessorEditor* editor = findParentComponentOfClass<PurristAudioProcessorEditor>();
    area.removeFromRight(area.getWidth() / 2);
    editor->buzzThresholdSlider.setBounds(area.removeFromBottom(area.getHeight() / 2));
}

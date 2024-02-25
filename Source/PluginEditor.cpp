/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PurristAudioProcessorEditor::PurristAudioProcessorEditor (PurristAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), buzzSection(p), hissSection(p), noiseSection(p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    addAndMakeVisible (buzzSection);
    addAndMakeVisible (hissSection);
    addAndMakeVisible (noiseSection);
    setResizable (true, true);
    setResizeLimits(1024, 500, 9999, 9999);
    setSize (1024, 500);
}

PurristAudioProcessorEditor::~PurristAudioProcessorEditor(){}

//==============================================================================

void PurristAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(79, 85, 117));
}

void PurristAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    
    int paddingX = 20;
    int maxHeight = 400;
    int paddingY = (area.getHeight() - maxHeight) / 2;
    
    area.removeFromTop(paddingY);
    area.removeFromRight(paddingX);
    area.removeFromBottom(paddingY);
    area.removeFromLeft(paddingX);
    
    auto gap = 6;
    auto sectionWidth = area.getWidth() / 3 - gap * 2 / 3;
    
    buzzSection.setBounds(area.removeFromLeft(sectionWidth));
    
    area.removeFromLeft(gap);
    
    hissSection.setBounds(area.removeFromLeft(sectionWidth));
    
    area.removeFromLeft(gap);
    
    noiseSection.setBounds(area.removeFromLeft(sectionWidth));
}

void BuzzComponent::paintSection(juce::Graphics& g)
{
    // area.removeFromRight(area.getWidth() / 2);
}

void HissComponent::paintSection(juce::Graphics& g)
{
    auto area = getSectionArea();
    
    auto responseArea = area.removeFromBottom(area.getHeight() / 4);
    responseCurve.setBounds(responseArea);
}

void HissComponent::resized()
{
    auto area = getSectionArea();
    area.removeFromBottom(area.getHeight() / 2);
    hissRatioSlider.setBounds(area.removeFromLeft(area.getWidth() / 2));
    hissCutoffSlider.setBounds(area);
    
}

std::vector<juce::Component*> BuzzComponent::getComponents()
{
    return
    {
        &buzzThresholdSlider,
        &buzzRatioSlider,
        &buzzFreqSlider
    };
}

std::vector<juce::Component*> HissComponent::getComponents()
{
    DBG("child getComponents");
    return
    {
        &hissThresholdSlider,
        &hissRatioSlider,
        &hissCutoffSlider,
        &responseCurve
    };
}

std::vector<juce::Component*> NoiseComponent::getComponents()
{
    return
    {
        &noiseThresholdSlider,
        &noiseRatioSlider,
        &noiseReleaseSlider
    };
}

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
noiseReleaseSliderAttachment(audioProcessor.apvts, "noise_release", noiseReleaseSlider),
buzzThresholdSlider(*audioProcessor.apvts.getParameter("buzz_threshold"), "dB"),
buzzRatioSlider(*audioProcessor.apvts.getParameter("buzz_ratio"), ":1"),
buzzFreqSlider(*audioProcessor.apvts.getParameter("buzz_frequency"), ""),
hissThresholdSlider(*audioProcessor.apvts.getParameter("hiss_threshold"), "dB"),
hissRatioSlider(*audioProcessor.apvts.getParameter("hiss_ratio"), ":1"),
hissCutoffSlider(*audioProcessor.apvts.getParameter("hiss_cutoff"), "Hz"),
noiseThresholdSlider(*audioProcessor.apvts.getParameter("noise_threshold"), "dB"),
noiseRatioSlider(*audioProcessor.apvts.getParameter("noise_ratio"), ":1"),
noiseReleaseSlider(*audioProcessor.apvts.getParameter("noise_release"), "mS")
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

void BuzzComponent::paintSection(juce::Graphics& g, juce::Rectangle<int> area)
{
    PurristAudioProcessorEditor* editor = findParentComponentOfClass<PurristAudioProcessorEditor>();
    area.removeFromRight(area.getWidth() / 2);
    editor->hissCutoffSlider.setBounds(area.removeFromBottom(area.getHeight() / 2));
}

void HissComponent::paintSection(juce::Graphics& g, juce::Rectangle<int> area)
{
    PurristAudioProcessorEditor* editor = findParentComponentOfClass<PurristAudioProcessorEditor>();
    auto responseArea = area.removeFromBottom(area.getHeight() / 2);
    auto legendX = responseArea.removeFromBottom(24);
    auto legendY = responseArea.removeFromRight(48);
    auto responseAreaWidth = responseArea.getWidth();
    
    auto& hissGate = editor->audioProcessor.chain[0].get<ChainPositions::hissGate>();
    float filterGain = hissGate.getCurrentGain(0);
    auto filterFrequency = editor->audioProcessor.apvts.getRawParameterValue("hiss_cutoff")->load();
    auto sampleRate = editor->audioProcessor.getSampleRate();
    *filter.coefficients = juce::dsp::IIR::ArrayCoefficients<float>::makeHighShelf(sampleRate, float(filterFrequency), 1, filterGain);
    
    std::vector<double> magnitudes;
    
    magnitudes.resize(responseAreaWidth);
    
    for (int i = 0; i < responseAreaWidth; i++) {
        float magnitude = 1.f;
        float frequency = juce::mapToLog10<float>(float(i) / float(responseAreaWidth), 200.0, 10000.0);
        magnitude = filter.coefficients->getMagnitudeForFrequency(frequency, sampleRate);
        magnitudes[i] = juce::Decibels::gainToDecibels(magnitude);
    }
    
    juce::Path responseCurve;
    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input)
    {
        return juce::jmap(input, -48.0, 0.0, outputMin, outputMax);
    };
    
    responseCurve.startNewSubPath(responseArea.getX(), map(magnitudes.front()));
    
    for( size_t i = 1; i < magnitudes.size(); ++i )
    {
        responseCurve.lineTo(responseArea.getX() + i, map(magnitudes[i]));
    }
    g.setColour(juce::Colours::black);
    g.strokePath(responseCurve, juce::PathStrokeType(2.f));
}

void HissComponent::timerCallback()
{
    repaint();
}

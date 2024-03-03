/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

//==============================================================================
PurristAudioProcessorEditor::PurristAudioProcessorEditor (PurristAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), buzzSection(p), hissSection(p), noiseSection(p)
{
    PurristLookAndFeel* lnf = PurristLookAndFeel::getInstance();
    
    lnf->setColour (juce::Slider::thumbColourId, juce::Colours::white);
    lnf->setColour (juce::Slider::rotarySliderFillColourId, juce::Colours::black);
    
    lnf->setColour (juce::ComboBox::outlineColourId,    juce::Colours::black);
    lnf->setColour (juce::TextButton::textColourOffId,  juce::Colours::black);
    lnf->setColour (juce::TextButton::textColourOnId,   juce::Colours::white);
    lnf->setColour (juce::TextButton::buttonColourId,   juce::Colours::white);
    lnf->setColour (juce::TextButton::buttonOnColourId, juce::Colours::black);
    
    juce::LookAndFeel::setDefaultLookAndFeel(lnf);
    
    logoShadow = juce::Drawable::createFromImageData (BinaryData::straycat_svg, BinaryData::straycat_svgSize);
    logo = juce::Drawable::createFromImageData (BinaryData::straycatwhite_svg, BinaryData::straycatwhite_svgSize);
    
    pluginIconShadow = juce::Drawable::createFromImageData (BinaryData::purristlogoshadow_svg, BinaryData::purristlogoshadow_svgSize);
    pluginIcon = juce::Drawable::createFromImageData (BinaryData::purristlogo_svg, BinaryData::purristlogo_svgSize);
    
    pluginLogo.setText("PURRIST");
    pluginLogo.setColour(juce::Colour(250, 219, 35));
    pluginLogo.setFont(getDisplayFont(), true);
    pluginLogo.setJustification(juce::Justification::bottomLeft);
    pluginLogo.setFontHeight(82);
    
    pluginLogoShadow.setText("PURRIST");
    pluginLogoShadow.setColour(juce::Colours::black);
    pluginLogoShadow.setFont(getDisplayFont(), true);
    pluginLogoShadow.setJustification(juce::Justification::bottomLeft);
    pluginLogoShadow.setFontHeight(82);
    
    addAndMakeVisible(pluginIconShadow.get());
    addAndMakeVisible(logoShadow.get());
    addAndMakeVisible(logo.get());
    addAndMakeVisible(pluginLogoShadow);
    addAndMakeVisible(pluginLogo);
    addAndMakeVisible (buzzSection);
    addAndMakeVisible (hissSection);
    addAndMakeVisible (noiseSection);
    addAndMakeVisible(pluginIcon.get());
    setResizable (true, true);
    setResizeLimits(920, 540, 9999, 9999);
    setSize (1024, 540);
}

PurristAudioProcessorEditor::~PurristAudioProcessorEditor(){}

//==============================================================================

void PurristAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour(79, 85, 117));
    
    g.setColour(juce::Colours::red);
    g.drawRect(debugRect1);
    
    g.setColour(juce::Colours::green);
    g.drawRect(debugRect2);
}

void PurristAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    
    int maxHeight = 480;
    int maxWidth = 1024;
    int paddingY = (area.getHeight() - maxHeight) / 2;
    int paddingX = juce::jmax((area.getWidth() - maxWidth) / 2, 20);
    
    area.removeFromTop(paddingY);
    area.removeFromRight(paddingX);
    area.removeFromBottom(paddingY);
    area.removeFromLeft(paddingX);
    
    using namespace juce;
    
    /*--------------------------------------*/

    auto header = area.removeFromTop(24);
    auto logoShadowArea = header.removeFromRight(300);
    logoShadowArea = logoShadowArea.withY(logoShadowArea.getY() + 5);
    auto logoArea = logoShadowArea.withPosition(logoShadowArea.getX() - 5, logoShadowArea.getY() - 5);
    
    RectanglePlacement logoPlacement {RectanglePlacement::xRight | RectanglePlacement::yBottom};
    
    logoShadow->setTransformToFit(logoShadowArea.toFloat(), logoPlacement);
    logo->setTransformToFit(logoArea.toFloat(), logoPlacement);
    
    /*--------------------------------------*/
    
    auto pluginLogoHeight = header.getHeight() * 1.75f;
    auto pluginLogoOffset = header.getHeight() * 0.4f;
    
    auto pluginLogoArea = header.removeFromLeft(300).toFloat();
    pluginLogoArea = pluginLogoArea.withHeight(pluginLogoHeight)
        .withY(pluginLogoArea.getY() - pluginLogoOffset);
    auto pluginLogoShadowArea = pluginLogoArea.withPosition(pluginLogoArea.getX() + 5, pluginLogoArea.getY() + 5);
    
    pluginLogoShadow.setBoundingBox(Parallelogram<float>(pluginLogoShadowArea));
    pluginLogo.setBoundingBox(Parallelogram<float>(pluginLogoArea));
    
    /*--------------------------------------*/
    
    auto pluginIconArea = header.withBottom(header.getBottom() + 40).withTop(header.getY() - 10);
    auto pluginIconShadowArea = pluginIconArea.withPosition(pluginIconArea.getX() + 5, pluginIconArea.getY() + 5);
    
    RectanglePlacement pluginIconPlacement {RectanglePlacement::xMid | RectanglePlacement::yBottom};
    
    pluginIcon->setTransformToFit(pluginIconArea.toFloat(), pluginIconPlacement);
    pluginIconShadow->setTransformToFit(pluginIconShadowArea.toFloat(), pluginIconPlacement);
    
    area.removeFromTop(16);
    
    auto gap = 6;
    auto narrowSectionWidth = area.getWidth() * 4 / 13 - gap * 2 / 3;
    auto wideSectionWidth = area.getWidth() * 5 / 13 - gap * 2 / 3;
    
    buzzSection.setBounds(area.removeFromLeft(narrowSectionWidth));
    
    area.removeFromLeft(gap);
    
    hissSection.setBounds(area.removeFromLeft(wideSectionWidth));
    
    area.removeFromLeft(gap);
    
    noiseSection.setBounds(area.removeFromLeft(narrowSectionWidth));
}

void BuzzComponent::paintSection(juce::Graphics& g)
{
    auto area = getSectionArea();
    auto thresholdSliderBounds = area.removeFromRight(100);
    
//    g.setColour(juce::Colours::red);
//    g.drawRect(thresholdSliderBounds);
    
    gainReductionMeter.setBounds(area.removeFromBottom(64));
    
    auto buttonsArea = area.removeFromBottom((area.getHeight()) / 2);
    buttonsArea.removeFromTop(24);
    buttonsArea.removeFromBottom(48);
    auto labelArea = buttonsArea.removeFromBottom(21);
    buttonsArea.removeFromBottom(8);
    buttonsArea = buttonsArea.removeFromBottom(40);
    
    g.setFont(getMediumFont());
    g.setFont(21);
    g.setColour(juce::Colours::black);
    g.drawFittedText("AC Frequency", labelArea, juce::Justification::topLeft, 1);
    
    freqButton[0].setBounds(buttonsArea.removeFromLeft(buttonsArea.getWidth() / 2));
    freqButton[1].setBounds(buttonsArea);
    
    area.removeFromBottom(24);
    ratioSlider.setBounds(area.withRight(area.getRight() + 8));
    
    thresholdSlider.setBounds(thresholdSliderBounds);
}

void HissComponent::paintSection(juce::Graphics& g)
{
    auto area = getSectionArea();
    
    auto responseArea = area.removeFromBottom(area.getHeight() / 3);
    responseCurve.setBounds(responseArea);
}

void HissComponent::resized()
{
    auto area = getSectionArea();
    area.removeFromBottom(area.getHeight() / 3 + 18);

    thresholdSlider.setBounds(area.removeFromBottom(80));
    
    area.removeFromBottom(18);
    
    ratioSlider.setBounds(area.removeFromLeft(area.getWidth() / 2));
    cutoffSlider.setBounds(area);
    
}

void NoiseComponent::resized()
{
    auto area = getSectionArea();
    auto thresholdSliderBounds = area.removeFromRight(100);
    
    gainReductionMeter.setBounds(area.removeFromBottom(64));
    
    releaseSlider.setBounds(area.removeFromBottom((area.getHeight() - 24) / 2).withRight(area.getRight() + 15));
    area.removeFromBottom(24);
    ratioSlider.setBounds(area.withRight(area.getRight() + 15));
    
    thresholdSlider.setBounds(thresholdSliderBounds);
}

std::vector<juce::Component*> BuzzComponent::getComponents()
{
    return
    {
        &thresholdSlider,
        &ratioSlider,
        &freqButton[0],
        &freqButton[1],
        &gainReductionMeter
    };
}

std::vector<juce::Component*> HissComponent::getComponents()
{
    return
    {
        &thresholdSlider,
        &ratioSlider,
        &cutoffSlider,
        &responseCurve
    };
}

std::vector<juce::Component*> NoiseComponent::getComponents()
{
    return
    {
        &thresholdSlider,
        &ratioSlider,
        &releaseSlider,
        &gainReductionMeter
    };
}

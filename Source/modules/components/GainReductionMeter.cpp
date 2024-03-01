/*
  ==============================================================================

    GainReductionMeter.cpp
    Created: 29 Feb 2024 10:26:19pm
    Author:  Przemysław Barski

  ==============================================================================
*/

#include "GainReductionMeter.h"
#include "GUI.h"

void GainReductionMeter::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();
    int textHeight = 21;
    auto labelArea = area.removeFromBottom(textHeight);
    area.removeFromBottom(8);
    auto gainReductionArea = area.reduced(padding);
    auto gainReduction = rmsMeters.getGainReduction();
    auto rmsReduction = juce::jmap(gainReduction, float(meterRange), 0.f, float(padding), float(gainReductionArea.getWidth() + padding));
    
    juce::Rectangle meter = gainReductionArea.withLeft(rmsReduction);
    g.setColour(juce::Colours::lightgrey);
    g.fillRect(meter);
    
//    g.drawImage(background, getLocalBounds().toFloat());
    
    using namespace juce;

    auto graphArea = area;
    
    g.setFont(getMediumFont());
    g.setFont(textHeight);
    g.setColour (Colours::black);
    g.drawFittedText("Reduction", labelArea, Justification::centredLeft, 1);
    
    g.setColour(juce::Colours::black);
    g.drawRect(graphArea, 2);
    
    int textWidth = 22;
    
    int scaleStep = 12;
    
    if ((gainReductionArea.getWidth() / (meterRange / -3)) >= textWidth)
    {
        scaleStep = 3;
    } else if ((gainReductionArea.getWidth() / (meterRange / -6)) >= textWidth)
    {
        scaleStep = 6;
    }
    
    for (int dbBar = meterRange + scaleStep; dbBar < 0; dbBar += scaleStep) {
        auto bar = jmap(float(dbBar), float(meterRange), 0.f, 0.f, float(gainReductionArea.getWidth()));
        bar = bar + padding;
        g.setColour(juce::Colours::black);
        
        Rectangle<int> label;
        
        g.drawVerticalLine(bar, 0, padding);
        g.drawVerticalLine(bar, graphArea.getHeight() - padding, graphArea.getHeight());
        
        label.setX(bar - textWidth / 2);
        label.setY(padding);
        label.setWidth(textWidth);
        label.setHeight(graphArea.getHeight() - padding * 2);
        
        g.setFont(getFont());
        g.drawFittedText(String(int(dbBar)), label, Justification::centred, 1);
    }
}

void GainReductionMeter::resized()
{
//    using namespace juce;
//    background = Image(Image::PixelFormat::RGB, getWidth(), getHeight(), true);
//    Graphics g(background);
//    
//    int textHeight = 21;
//    auto area = getLocalBounds();
//    auto labelArea = area.removeFromBottom(textHeight);
//    area.removeFromBottom(8);
//    auto graphArea = area;
//    auto gainReductionArea = graphArea.reduced(padding);
//    
//    g.setFont(getMediumFont());
//    g.setFont(textHeight);
//    g.setColour (Colours::black);
//    g.drawFittedText("Reduction", labelArea, Justification::centredLeft, 1);
//    
//    g.setColour(juce::Colours::black);
//    g.drawRect(graphArea, 2);
//    
//    int textWidth = 22;
//    int scaleStep = (gainReductionArea.getWidth() / (meterRange / -6)) < textWidth ? 12 : 6;
//    
//    for (int dbBar = meterRange + scaleStep; dbBar < 0; dbBar += scaleStep) {
//        auto bar = jmap(float(dbBar), float(meterRange), 0.f, 0.f, float(gainReductionArea.getWidth()));
//        bar = bar + padding;
//        g.setColour(juce::Colours::black);
//        
//        Rectangle<int> label;
//        
//        g.drawVerticalLine(bar, 0, padding);
//        g.drawVerticalLine(bar, graphArea.getHeight() - padding, graphArea.getHeight());
//        
//        label.setX(bar - textWidth / 2);
//        label.setY(padding);
//        label.setWidth(textWidth);
//        label.setHeight(graphArea.getHeight() - padding * 2);
//        
//        g.setFont(getFont());
//        g.drawFittedText(String(int(dbBar)), label, Justification::centred, 1);
//    }
}

void GainReductionMeter::timerCallback()
{
    repaint();
}

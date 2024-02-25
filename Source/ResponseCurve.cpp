/*
  ==============================================================================

    ResponseCurve.cpp
    Created: 25 Feb 2024 5:09:03pm
    Author:  Przemysław Barski

  ==============================================================================
*/

#include "ResponseCurve.h"

void ResponseCurve::paint(juce::Graphics& g)
{
    auto responseArea = getLocalBounds();
    auto responseAreaWidth = responseArea.getWidth();
    
    g.drawImage(background, responseArea.toFloat());
    
    auto& hissGate = audioProcessor.chain[0].get<ChainPositions::hissGate>();
    float filterGain = hissGate.getCurrentGain(0);
    auto filterFrequency = audioProcessor.apvts.getRawParameterValue("hiss_cutoff")->load();
    auto sampleRate = audioProcessor.getSampleRate();
    *filter.coefficients = juce::dsp::IIR::ArrayCoefficients<float>::makeHighShelf(sampleRate, float(filterFrequency), 1, filterGain);
    
//    g.setColour(juce::Colours::red);
//    g.drawRect(responseArea);
    
    std::vector<double> magnitudes;
    
    magnitudes.resize(responseAreaWidth);
    
    for (int i = 0; i < responseAreaWidth; i++) {
        float magnitude = 1.f;
        float frequency = juce::mapToLog10<float>(float(i) / float(responseAreaWidth), 500.0, 10000.0);
        magnitude = filter.coefficients->getMagnitudeForFrequency(frequency, sampleRate);
        magnitudes[i] = juce::Decibels::gainToDecibels(magnitude);
    }
    
    juce::Path responseCurve;
    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input)
    {
        return juce::jmap(input, -27.0, 3.0, outputMin, outputMax);
    };
    
    responseCurve.startNewSubPath(responseArea.getX(), map(magnitudes.front()));
    
    for( size_t i = 0; i < magnitudes.size(); ++i )
    {
        responseCurve.lineTo(responseArea.getX() + i, map(magnitudes[i]));
    }
    g.setColour(juce::Colours::black);
    g.strokePath(responseCurve, juce::PathStrokeType(2.f));
}

void ResponseCurve::resized()
{
    using namespace juce;
    background = Image(Image::PixelFormat::RGB, getWidth(), getHeight(), true);
    Graphics g(background);
    
    auto graphArea = getLocalBounds();
    
    g.setColour(juce::Colours::grey);
    g.drawRect(graphArea);
    
    Array<float> frequencies
    {
        600, 700, 800, 900, 1000,
        2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000
    };
    
    g.setColour(Colours::grey);
    for (auto f: frequencies )
    {
        auto normX = mapFromLog10(f, 500.f, 10000.f) ;
        g.drawVerticalLine(graphArea.getWidth() * normX,
                           graphArea.getY(),
                           graphArea.getHeight());
    }
    
    Array<float> gain
    {
        -24.f, -18.f, -12.f, -6.f, 0.f
    };
    
    for ( auto gDb : gain )
    {
        auto y = jmap(gDb, -27.f, 3.f, float(graphArea.getHeight()), 0.f);
        g.drawHorizontalLine(y, 0, graphArea.getWidth());
    }
}

void ResponseCurve::timerCallback()
{
    repaint();
}

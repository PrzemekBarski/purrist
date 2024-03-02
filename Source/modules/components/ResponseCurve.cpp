/*
  ==============================================================================

    ResponseCurve.cpp
    Created: 25 Feb 2024 5:09:03pm
    Author:  Przemysław Barski

  ==============================================================================
*/

#include "ResponseCurve.h"
#include "GUI.h"

void ResponseCurve::paint(juce::Graphics& g)
{
    auto responseArea = getLocalBounds();
    
    g.drawImage(background, responseArea.toFloat());
    
    responseArea.removeFromBottom(18);
    responseArea.removeFromRight(28);
    auto responseAreaWidth = responseArea.getWidth();
    
    auto& hissGate = audioProcessor.chain[0].get<ChainPositions::hissGate>();
    float filterGain = hissGate.getCurrentGain();
    auto filterFrequency = audioProcessor.apvts.getRawParameterValue("hiss_cutoff")->load();
    auto sampleRate = audioProcessor.getSampleRate();
    *filter.coefficients = juce::dsp::IIR::ArrayCoefficients<float>::makeHighShelf(sampleRate, float(filterFrequency), 1, filterGain);
    
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
    g.strokePath(responseCurve, juce::PathStrokeType(3.f));
}

void ResponseCurve::resized()
{
    using namespace juce;
    background = Image(Image::PixelFormat::RGB, getWidth() * 2, getHeight() * 2, true);
    Graphics g(background);
    
    auto graphArea = getLocalBounds().withSize(getWidth() * 2, getHeight() * 2);
    auto xLabels = graphArea.removeFromBottom(18 * 2);
    xLabels.removeFromRight(24 * 2);
    auto yLabels = graphArea.removeFromRight(28 * 2);
    
    Rectangle<int> startLabel, endLabel;
    
    startLabel.setX(0);
    startLabel.setY(xLabels.getY());
    startLabel.setWidth(30 * 2);
    startLabel.setHeight(18 * 2);
    
    endLabel.setX(xLabels.getWidth() - 30 * 2);
    endLabel.setY(xLabels.getY());
    endLabel.setWidth(30 * 2);
    endLabel.setHeight(18 * 2);
    
    g.setColour(Colour(80, 80, 80));
    g.setFont(getFont());
    g.setFont(14 * 2);
    g.drawFittedText("500", startLabel, Justification::left, 1);
    g.drawFittedText("10k", endLabel, Justification::right, 1);
    
    Array<float> frequencies
    {
        600, 700, 800, 900, 1000,
        2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000
    };
    
    Array<float> labels
    {
        1000, 5000
    };
    
    for (auto f: frequencies )
    {
        auto normX = mapFromLog10(f, 500.f, 10000.f) ;
        g.setColour(Colour(180, 180, 180));
        g.fillRect(graphArea.getWidth() * normX,
                   float(graphArea.getY()),
                   2.f,
                   float(graphArea.getHeight()));
//        g.drawVerticalLine(graphArea.getWidth() * normX,
//                           graphArea.getY(),
//                           graphArea.getHeight());
        if (labels.contains(f))
        {
            Rectangle<int> label;
            int formattedValue = f >= 1000 ? int(f / 1000) : int(f);
            String labelText = String(formattedValue);
            
            if (f >= 1000)
                labelText << "k";
            
            label.setX(graphArea.getWidth() * normX - 15 * 2);
            label.setY(xLabels.getY());
            label.setWidth(30 * 2);
            label.setHeight(18 * 2);
            
            g.setColour(Colour(80, 80, 80));
//            g.setFont(getFont());
            g.drawFittedText(labelText, label, Justification::centred, 1);
        }
    }
    
    Array<float> gain
    {
        -24.f, -18.f, -12.f, -6.f, 0.f
    };
    
    for ( auto gDb : gain )
    {
        auto y = jmap(gDb, -27.f, 3.f, float(graphArea.getHeight()), 0.f);
        g.setColour(Colour(180, 180, 180));
        g.fillRect(0.f,
                   y,
                   float(graphArea.getWidth()),
                   2.f);
//        g.drawHorizontalLine(y, 0, graphArea.getWidth());
        
        Rectangle<int> label;
        String labelText = String(int(gDb));
        
        label.setX(yLabels.getX() + 4 * 2 + (gDb >= 0 ? 5 * 2 : 0));
        label.setY(y - 9 * 2);
        label.setWidth(24 * 2);
        label.setHeight(18 * 2);
        
        g.setColour(Colour(80, 80, 80));
//        g.setFont(getFont());
        g.drawFittedText(labelText, label, Justification::left, 1);
    }
    
    g.setColour(juce::Colours::black);
    g.drawRect(graphArea, 1.5f * 2);
}

void ResponseCurve::timerCallback()
{
    repaint();
}

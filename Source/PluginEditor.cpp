/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void LookAndFeel::drawRotarySlider(juce::Graphics & g,
                                int x,
                                int y,
                                int width,
                                int height,
                                float sliderPosProportional,
                                float rotaryStartAngle,
                                float rotaryEndAngle,
                                juce::Slider & slider)
{
    using namespace juce;
    auto outline = slider.findColour (Slider::rotarySliderOutlineColourId);
    auto fill    = slider.findColour (Slider::rotarySliderFillColourId);

    auto bounds = Rectangle<int> (x, y, width, height).toFloat().reduced (10);
    
//    g.setColour(juce::Colours::black);
//    g.drawRect(bounds);
    
    if (auto* rswl = dynamic_cast<RotarySliderWithLabels*>(&slider))
    {
        auto center = bounds.getCentre();
        Path p;
        Rectangle<float> r;
        r.setLeft(center.getX());
        r.setRight(center.getX());
        r.setTop(bounds. getY());
        r.setBottom(center.getY() - rswl->getTextHeight());
        
        g.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        r.setSize(strWidth, rswl->getTextHeight());
        r.setCentre(bounds.getCentreX(), bounds.getCentreY() + rswl->getTextHeight());
        g. setColour (Colours::black);
        g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }

    auto radius = jmin (bounds.getWidth(), bounds.getHeight() * 2) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = jmin (8.0f, radius * 0.5f);
    auto arcRadius = radius - lineW * 0.5f;

    Path backgroundArc;
    backgroundArc.addCentredArc (bounds.getCentreX(),
                                 bounds.getBottom(),
                                 arcRadius,
                                 arcRadius,
                                 0.0f,
                                 rotaryStartAngle,
                                 rotaryEndAngle,
                                 true);

    g.setColour (outline);
    g.strokePath (backgroundArc, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));

    if (slider.isEnabled())
    {
        Path valueArc;
        valueArc.addCentredArc (bounds.getCentreX(),
                                bounds.getBottom(),
                                arcRadius,
                                arcRadius,
                                0.0f,
                                rotaryStartAngle,
                                toAngle,
                                true);

        g.setColour (fill);
        g.strokePath (valueArc, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));
    }

    auto thumbWidth = lineW * 2.0f;
    Point<float> thumbPoint (bounds.getCentreX() + arcRadius * std::cos (toAngle - MathConstants<float>::halfPi),
                             bounds.getBottom() + arcRadius * std::sin (toAngle - MathConstants<float>::halfPi));

    g.setColour (slider.findColour (Slider::thumbColourId));
    g.fillEllipse (Rectangle<float> (thumbWidth, thumbWidth).withCentre (thumbPoint));
    g.setColour (fill);
    g.drawEllipse(Rectangle<float> (thumbWidth, thumbWidth).withCentre (thumbPoint), 6);
}
void RotarySliderWithLabels::paint(juce::Graphics &g)
{
    using namespace juce;
    
    auto startAngle = degreesToRadians(270.f);
    auto endAngle = degreesToRadians(90.f) + MathConstants<float>::twoPi;
    auto range = getRange();
    auto sliderBounds = getSliderBounds();
    auto area = getLocalBounds();
    area.removeFromTop(sliderBounds.getHeight());
    auto label = area.removeFromTop(getLabelTextHeight() * 1.5);
    
    g.setFont(getLabelTextHeight());
    g.setColour (Colours::black);
    g.drawFittedText(param->getName(20), label, juce::Justification::centred, 1);
    
//    g.setColour(juce::Colours::red);
//    g.drawRect(getLocalBounds());
    
    
    getLookAndFeel().drawRotarySlider(g,
                                sliderBounds.getX(),
                                sliderBounds.getY(),
                                sliderBounds.getWidth(),
                                sliderBounds.getHeight(),
                                jmap(getValue(),
                                range.getStart(),
                                range.getEnd(), 0.0, 1.0),
                                startAngle,
                                endAngle,
                                *this);
}

juce::Rectangle<int> RotarySliderWithLabels::getSliderBounds() const
{
    auto bounds = getLocalBounds();
    auto size = bounds.getWidth() / 2;
    auto textFieldHeight = getLabelTextHeight() * 1.5;
    if (bounds.getHeight() - textFieldHeight < size) {
        size = bounds.getHeight() - textFieldHeight;
    }
    juce::Rectangle<int> r;
    r.setSize(size * 2, size);
    r.setCentre(bounds.getCentreX(), 0);
    r.setY(2);
    
    return r;
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
        return choiceParam->getCurrentChoiceName();
        
    juce::String str;
    bool addK = false;
    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
    {
        float val = getValue();
        if( val >= 1000.f)
        {
            val /= 1000.f;
            addK = true;
        }
        
        str = juce::String(val, 1, false);
    }
    else
    {
        jassertfalse;
    }
    
    if( suffix.isNotEmpty() )
    {
        str << " ";
        if( addK )
            str << "k";
        str << suffix;
    }
    
    return str;
}

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

void HissComponent::resized()
{
    auto area = getSectionArea();
    area.removeFromBottom(area.getHeight() / 2);
    hissRatioSlider.setBounds(area.removeFromLeft(area.getWidth() / 2));
    hissCutoffSlider.setBounds(area);
    
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

void HissComponent::timerCallback()
{
    repaint();
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
        &hissCutoffSlider
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

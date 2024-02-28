/*
  ==============================================================================

    GUI.cpp
    Created: 25 Feb 2024 4:55:51pm
    Author:  Przemysław Barski

  ==============================================================================
*/

#include "GUI.h"

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
        
        auto textHeight = width / 8;
        g.setFont(getFont());
        g.setFont(textHeight);
        auto text = rswl->getDisplayString();
        auto strWidth = g.getCurrentFont().getStringWidth(text);
        r.setSize(strWidth, textHeight);
        r.setCentre(bounds.getCentreX(), bounds.getCentreY() + textHeight * 0.75);
        g.setColour (Colours::black);
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
    
    auto textHeight = 21;
    auto label = area.removeFromTop(textHeight);
    
    g.setFont(getMediumFont());
    g.setFont(textHeight);
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

void RMSHorizontalSlider::timerCallback()
{
    repaint();
}

void RMSHorizontalSlider::paint(juce::Graphics &g)
{
    using namespace juce;
    
    auto bounds = getLocalBounds();
    auto range = getRange();;
    int thumbRadius = lnf.getSliderThumbRadius(*this);
    int trackWidth = 32;
    int thumbWidth = lnf.getSliderThumbRadius(*this);
    double start = bounds.getX() + thumbRadius * 0.5;
    double end = bounds.getWidth() - thumbRadius * 0.5;
    float rms = Decibels::gainToDecibels(inputRMSMeter.getInputRMS());
    String valueText = String(getValue(), 1, false);
    valueText << " dB";
    
    auto textHeight = 21;
    Rectangle<int> label, value;
    
    label.setX(bounds.getX());
    label.setY(bounds.getY() + trackWidth + thumbWidth + 4);
    label.setHeight(textHeight);
    label.setWidth(bounds.getWidth() / 2);
    value = label.withX(bounds.getX() + bounds.getWidth() / 2);
    
    g.setFont(getMediumFont());
    g.setFont(textHeight);
    g.setColour (Colours::black);
    g.drawFittedText(param->getName(20), label, juce::Justification::topLeft, 1);
    g.setFont(getFont());
    g.setFont(18);
    g.drawFittedText(valueText, value, juce::Justification::centredRight, 1);
    
//    g.setColour(juce::Colours::red);
//    g.drawRect(value);
    
    lnf.drawRMSSlider(g,
                    bounds.getX(),
                    bounds.getY(),
                    bounds.getWidth(),
                    trackWidth + thumbWidth,
                    jmap(getValue(),
                        range.getStart(),
                        range.getEnd(), start, end),
                    bounds.getX(),
                    bounds.getX() + bounds.getWidth(),
                    getSliderStyle(),
                    *this, rms);
}

void LookAndFeel::drawRMSSlider (juce::Graphics& g, int x, int y, int width, int height,
                                       float sliderPos,
                                       float minSliderPos,
                                       float maxSliderPos,
                                       const juce::Slider::SliderStyle style,
                                        juce::Slider& slider,
                                        float rms)
{
    using namespace juce;
    Rectangle<int> outline;
    Rectangle<int> levelMeter;
    int thumbRadius = getSliderThumbRadius(slider);
    float trackWidth = height - thumbRadius;
    float trackLength = slider.isHorizontal() ? width : height;
    auto range = slider.getNormalisableRange().getRange();
    auto rmsValue = jmap(rms,
                         float(range.getStart()),
                         float(range.getEnd()),
                         float(x + thumbRadius / 2),
                         float(width + thumbRadius / 2));
    
    rmsValue = rmsValue > 0 ? rmsValue : 0;
    
    auto pointerColour = slider.findColour (Slider::thumbColourId);
    
    if (slider.isHorizontal())
    {
        int trackY = thumbRadius;
        outline.setX(x);
        outline.setY(trackY);
        outline.setHeight(trackWidth);
        outline.setWidth(width);
        
        levelMeter.setX(x + thumbRadius / 2);
        levelMeter.setY(trackY + thumbRadius / 2);
        levelMeter.setHeight(trackWidth - thumbRadius);
        levelMeter.setWidth(rmsValue);
        
        drawPointer (g, sliderPos - thumbRadius / 2,
                     trackY - thumbRadius,
                     thumbRadius, pointerColour, 2);
    }
    else
    {
        drawPointer (g, jmax (0.0f, (float) x + (float) width * 0.5f - trackWidth),
                     sliderPos - thumbRadius,
                     thumbRadius, pointerColour, 1);
    }
    
    g.setColour(juce::Colours::lightgrey);
    g.fillRect(levelMeter);
    
    for (float dbBar = -84; dbBar <= 0; dbBar += 12) {
        auto bar = jmap(dbBar, -96.f, 12.f, trackLength - thumbRadius, 0.f);
        bar = width - thumbRadius / 2 - bar;
        g.setColour(juce::Colours::black);
        g.drawVerticalLine(bar, thumbRadius, thumbRadius * 1.5);
        g.drawVerticalLine(bar, trackWidth + thumbRadius / 2, trackWidth + thumbRadius);
        
        Rectangle<int> label;
        
        label.setX(bar - 15);
        label.setY(thumbRadius * 1.5);
        label.setWidth(30);
        label.setHeight(trackWidth - thumbRadius);
        
        g.setFont(getFont());
        g.drawFittedText(String(int(dbBar)), label, Justification::centred, 1);
    }
    
    g.setColour(juce::Colours::black);
    g.drawRect(outline, 2);
}

void LookAndFeel::drawPointer (juce::Graphics& g, const float x, const float y, const float diameter,
                                  const juce::Colour& colour, const int direction)
{
    using namespace juce;
    
    Path p;
    p.startNewSubPath (x + diameter * 0.5f, y);
    p.lineTo (x + diameter, y + diameter * 0.6f);
    p.lineTo (x + diameter, y + diameter);
    p.lineTo (x, y + diameter);
    p.lineTo (x, y + diameter * 0.6f);
    p.closeSubPath();

    p.applyTransform (AffineTransform::rotation ((float) direction * MathConstants<float>::halfPi,
                                                 x + diameter * 0.5f, y + diameter * 0.5f));
    g.setColour (colour);
    g.fillPath (p);
}

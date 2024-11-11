/*
  ==============================================================================

    GUI.cpp
    Created: 25 Feb 2024 4:55:51pm
    Author:  Przemysław Barski

  ==============================================================================
*/

#include "GUI.h"

JUCE_IMPLEMENT_SINGLETON (PurristLookAndFeel);

void PurristLookAndFeel::drawRotarySlider(juce::Graphics & g,
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

    auto bounds = Rectangle<int> (x, y, width, height).toFloat().reduced(0, 10);

    auto radius = jmin (bounds.getWidth(), bounds.getHeight() * 2) / 2.0f;
    auto toAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    auto lineW = jmin (8.0f, radius * 0.5f);
    auto arcRadius = radius - lineW * 0.5f;

    Path backgroundArc;
    int centreY = bounds.getBottom() - 2;
    backgroundArc.addCentredArc (bounds.getCentreX(),
                                 centreY,
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
                                centreY,
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
                             centreY + arcRadius * std::sin (toAngle - MathConstants<float>::halfPi));

    g.setColour (slider.findColour (Slider::thumbColourId));
    
    g.fillEllipse (Rectangle<float> (thumbWidth, thumbWidth).withCentre (thumbPoint));
    g.setColour (fill);
    g.drawEllipse(Rectangle<float> (thumbWidth, thumbWidth).withCentre (thumbPoint), 6);
}

juce::Rectangle<int> RotarySliderWithLabels::calculateBounds(juce::Rectangle<int> inputBounds)
{
    auto height = inputBounds.getWidth() / 2 + 10;
    juce::Rectangle<int> sliderBounds;
    sliderBounds.setSize(inputBounds.getWidth(), height);
    sliderBounds.setCentre(inputBounds.getCentreX(), 0);
    sliderBounds.setY(inputBounds.getY());
    
    return sliderBounds;
}

juce::Slider& RotarySliderWithLabels::getSlider()
{
    return slider;
}

void RotarySliderWithLabels::paint(juce::Graphics &g)
{
    using namespace juce;

    auto area = getLocalBounds();
    auto sliderBounds = calculateBounds(area);
    
    auto labelHeight = 21;
    
    area.removeFromTop(sliderBounds.getHeight());
    auto labelArea = area.removeFromTop(labelHeight);
    
    g.setFont(getMediumFont());
    g.setFont(labelHeight);
    g.setColour (Colours::black);
    g.drawFittedText(label, labelArea, juce::Justification::centred, 1);
    
    auto textHeight = sliderBounds.getWidth() / 8;
    auto center = sliderBounds.getCentre();
    
    Path p;
    Rectangle<float> r;
    r.setLeft(center.getX());
    r.setRight(center.getX());
    r.setTop(sliderBounds.getY() - 5);
    r.setBottom(center.getY() - textHeight - 5);
    
    g.setFont(getFont());
    g.setFont(textHeight);
    auto text = getDisplayString();
    auto strWidth = g.getCurrentFont().getStringWidth(text);
    r.setSize(strWidth, textHeight);
    r.setCentre(sliderBounds.getCentreX(), sliderBounds.getCentreY() + textHeight * 0.75);
    g.setColour (Colours::black);
    g.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    
    slider.setBounds(sliderBounds);
    addAndMakeVisible(slider);
}

juce::String RotarySliderWithLabels::getDisplayString() const
{
    if (auto* choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
        return choiceParam->getCurrentChoiceName();
        
    juce::String str;
    bool addK = false;
    if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
    {
        float val = slider.getValue();
        if( val >= 1000.f)
        {
            val /= 1000.f;
            addK = true;
        }
        
        if (decimalPlaces)
        {
            str = juce::String(val, decimalPlaces, false);
        } else {
            str = juce::String(juce::roundToInt(val));
        }
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

void RMSSlider::timerCallback()
{
    repaint();
}

void RMSSlider::paint(juce::Graphics &g)
{
    using namespace juce;
    
    auto bounds = getLocalBounds();
    auto range = getRange();
    int thumbRadius = getLookAndFeel().getSliderThumbRadius(*this);
    auto textHeight = 21;
    int trackWidth = isHorizontal() ? 32 : 44;
    double start = isHorizontal() ?
                    bounds.getX() + thumbRadius / 2 :
                    bounds.getHeight() - textHeight - 8 - thumbRadius / 2;
    double end = (isHorizontal() ?
                  bounds.getWidth() - thumbRadius / 2 :
                  bounds.getY() + textHeight + 8 + thumbRadius / 2);
    float rms = Decibels::gainToDecibels(inputRMSMeter.getInputRMS());
    
    String valueText = String(getValue(), 1, false);
    valueText << " dB";

    Rectangle<int> labelArea, valueArea;
    
    labelArea.setX(bounds.getX());
    labelArea.setY(isHorizontal() ?
               (bounds.getY() + trackWidth + thumbRadius + 4) :
               (bounds.getHeight() - textHeight));
    labelArea.setHeight(textHeight);
    labelArea.setWidth(isHorizontal() ?
                   (bounds.getWidth() / 2) :
                   bounds.getWidth());
    valueArea = isHorizontal() ?
        labelArea.withX(bounds.getX() + bounds.getWidth() / 2) :
        labelArea.withPosition(bounds.getWidth() - labelArea.getWidth(), 0);
    
    g.setFont(getMediumFont());
    g.setFont(textHeight);
    g.setColour (Colours::black);
    g.drawFittedText(label, labelArea, isHorizontal() ? Justification::topLeft : Justification::centredRight, 1);
    g.setFont(getFont());
    g.setFont(18);
    g.drawFittedText(valueText, valueArea, Justification::centredRight, 1);
    
//    g.setColour(juce::Colours::red);
//    g.drawRect(value);
    
    lnf.drawRMSSlider(g,
                    isHorizontal() ? bounds.getX() : bounds.getWidth() - trackWidth - thumbRadius,
                    isHorizontal() ? bounds.getY() : bounds.getY() + textHeight + 8,
                    isHorizontal() ? bounds.getWidth() : trackWidth + thumbRadius,
                    isHorizontal() ? trackWidth + thumbRadius : bounds.getHeight() - textHeight * 2 - 16,
                    jmap(getValue(),
                        range.getStart(),
                        range.getEnd(), start, end),
                    isHorizontal() ? bounds.getX() : bounds.getY() + bounds.getHeight(),
                    isHorizontal() ? bounds.getX() + bounds.getWidth() : bounds.getY(),
                    getSliderStyle(),
                    *this, rms);
}

void PurristLookAndFeel::drawRMSSlider (juce::Graphics& g, int x, int y, int width, int height,
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
    float trackWidth = slider.isHorizontal() ? height - thumbRadius : width - thumbRadius;
    float trackLength = slider.isHorizontal() ? width : height;
    auto range = slider.getNormalisableRange().getRange();
    auto rmsValue = jmap(rms,
                         float(range.getStart()),
                         float(range.getEnd()),
                         float(slider.isHorizontal() ? thumbRadius / 2 : thumbRadius / 2),
                         float(slider.isHorizontal() ? width + thumbRadius / 2 : trackLength - thumbRadius / 2));
        
    rmsValue = rmsValue > 0 ? rmsValue : 0;
    
    auto pointerColour = slider.findColour (Slider::thumbColourId);
    
    if (slider.isHorizontal())
    {
        outline.setX(x);
        outline.setY(thumbRadius);
        outline.setHeight(trackWidth);
        outline.setWidth(trackLength);
        
        levelMeter.setX(x + thumbRadius / 2);
        levelMeter.setY(thumbRadius + thumbRadius / 2);
        levelMeter.setHeight(trackWidth - thumbRadius);
        levelMeter.setWidth(rmsValue);
        
        drawPointer (g, sliderPos - thumbRadius / 2,
                     0,
                     thumbRadius, pointerColour, 2);
    }
    else
    {
        outline.setX(x + width - trackWidth);
        outline.setY(y);
        outline.setHeight(trackLength);
        outline.setWidth(trackWidth);
        
        levelMeter.setX(x + width - trackWidth + thumbRadius / 2);
        levelMeter.setY(y + trackLength - rmsValue - thumbRadius / 2);
        levelMeter.setHeight(rmsValue);
        levelMeter.setWidth(trackWidth - thumbRadius);
        
        drawPointer (g, x,
                     sliderPos - thumbRadius / 2,
                     thumbRadius, pointerColour, 1);
    }
    
    g.setColour(juce::Colours::lightgrey);
    g.fillRect(levelMeter);
    
    for (float dbBar = -84; dbBar <= -12; dbBar += 12) {
        auto bar = jmap(dbBar, -96.f, 0.f, trackLength - thumbRadius, 0.f);
        bar = slider.isHorizontal() ?
                trackLength - thumbRadius / 2 - bar :
                bar + y + thumbRadius / 2;
        g.setColour(juce::Colours::black);
        
        Rectangle<int> label;
        
        if (slider.isHorizontal())
        {
            g.drawVerticalLine(bar, thumbRadius, thumbRadius * 1.5);
            g.drawVerticalLine(bar, trackWidth + thumbRadius / 2, trackWidth + thumbRadius);
            
            label.setX(bar - 15);
            label.setY(thumbRadius * 1.5);
            label.setWidth(30);
            label.setHeight(trackWidth - thumbRadius);
        } else
        {
            g.drawHorizontalLine(bar, x + width - thumbRadius / 2 , x + width);
            g.drawHorizontalLine(bar, x + width - trackWidth, x + width - trackWidth + thumbRadius / 2);
            
            label.setX(x + width - trackWidth);
            label.setY(bar - (trackWidth - thumbRadius) / 2);
            label.setWidth(trackWidth);
            label.setHeight(trackWidth - thumbRadius);
        }
        
        g.setFont(getFont());
        g.drawFittedText(String(int(dbBar)), label, Justification::centred, 1);
    }
    
    g.setColour(juce::Colours::black);
    g.drawRect(outline, 2);
}

void PurristLookAndFeel::drawPointer (juce::Graphics& g, const float x, const float y, const float diameter,
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

void PurristLookAndFeel::drawButtonBackground (juce::Graphics& g,
                                               juce::Button& button,
                                               const juce::Colour& backgroundColour,
                                               bool shouldDrawButtonAsHighlighted,
                                               bool shouldDrawButtonAsDown)
{
    using namespace juce;
    
    auto cornerSize = 6.0f;
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f, 0.5f);

    auto baseColour = backgroundColour.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
                                      .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f);

    if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
        baseColour = baseColour.contrasting (shouldDrawButtonAsDown ? 0.2f : 0.05f);

    g.setColour (baseColour);

    auto flatOnLeft   = button.isConnectedOnLeft();
    auto flatOnRight  = button.isConnectedOnRight();
    auto flatOnTop    = button.isConnectedOnTop();
    auto flatOnBottom = button.isConnectedOnBottom();

    if (flatOnLeft || flatOnRight || flatOnTop || flatOnBottom)
    {
        Path path;
        path.addRoundedRectangle (bounds.getX(), bounds.getY(),
                                  bounds.getWidth(), bounds.getHeight(),
                                  cornerSize, cornerSize,
                                  ! (flatOnLeft  || flatOnTop),
                                  ! (flatOnRight || flatOnTop),
                                  ! (flatOnLeft  || flatOnBottom),
                                  ! (flatOnRight || flatOnBottom));

        g.fillPath (path);

        g.setColour (button.findColour (ComboBox::outlineColourId));
        g.strokePath (path, PathStrokeType (2.0f));
    }
    else
    {
        g.fillRoundedRectangle (bounds, cornerSize);

        g.setColour (button.findColour (ComboBox::outlineColourId));
        g.drawRoundedRectangle (bounds, cornerSize, 2.0f);
    }
}

juce::Font PurristLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return 18;
}

juce::Font PurristLookAndFeel::getTextButtonFont ()
{
    return getFont();
}

void PurristLookAndFeelShared::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                     bool /*shouldDrawButtonAsHighlighted*/, bool /*shouldDrawButtonAsDown*/)
{
    using namespace juce;
    
    Font fontSize = getTextButtonFont (button, button.getHeight());
    
    g.setFont (getTextButtonFont());
    g.setFont(fontSize.getHeight());
    g.setColour (button.findColour (button.getToggleState() ? TextButton::textColourOnId
                                                            : TextButton::textColourOffId)
                       .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));

    const int yIndent = jmin (4, button.proportionOfHeight (0.3f));
    const int cornerSize = jmin (button.getHeight(), button.getWidth()) / 2;

    const int fontHeight = roundToInt (fontSize.getHeight() * 0.6f);
    const int leftIndent  = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
    const int rightIndent = jmin (fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
    const int textWidth = button.getWidth() - leftIndent - rightIndent;

    if (textWidth > 0)
        g.drawFittedText (button.getButtonText(),
                          leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2,
                          Justification::centred, 2);
}

void PurristLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                       bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    using namespace juce;
    
    Path powerButton;
    
    auto bounds = button.getLocalBounds();
    auto size = jmin(bounds.getWidth(), bounds.getHeight()) - 4;
    auto r = bounds.withSizeKeepingCentre(size, size);
    float ang = 30.f;
    
    size -= 6;
    
    powerButton.addCentredArc(r.getCentreX(),
                            r. getCentreY(),
                            size * 0.5, size * 0.5,
                            0.f,
                            degreesToRadians (ang),
                            degreesToRadians (360.f - ang),
                            true);
    
    powerButton.startNewSubPath(r.getCentreX(), r.getY());
    powerButton.lineTo(r.getCentre().toFloat());
    
    PathStrokeType pst(3.5f, PathStrokeType::JointStyle::curved);
    auto color = button.getToggleState() ? Colours::black : Colours::grey;
    g.setColour(color);
    g.strokePath(powerButton, pst);
//    auto fontSize = jmin (15.0f, (float) button.getHeight() * 0.75f);
//    auto tickWidth = fontSize * 1.1f;
//
//    drawTickBox (g, button, 4.0f, ((float) button.getHeight() - tickWidth) * 0.5f,
//                 tickWidth, tickWidth,
//                 button.getToggleState(),
//                 button.isEnabled(),
//                 shouldDrawButtonAsHighlighted,
//                 shouldDrawButtonAsDown);
//
//    g.setColour (button.findColour (ToggleButton::textColourId));
//    g.setFont (fontSize);
//
//    if (! button.isEnabled())
//        g.setOpacity (0.5f);
//
//    g.drawFittedText (button.getButtonText(),
//                      button.getLocalBounds().withTrimmedLeft (roundToInt (tickWidth) + 10)
//                                             .withTrimmedRight (2),
//                      Justification::centredLeft, 10);
}

JUCE_IMPLEMENT_SINGLETON (PurristHelpButtonLNF);

juce::Font PurristHelpButtonLNF::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return 22;
}

juce::Font PurristHelpButtonLNF::getTextButtonFont ()
{
    return getMediumFont();
}

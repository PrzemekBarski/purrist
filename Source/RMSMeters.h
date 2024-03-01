/*
  ==============================================================================

    InputRMSMeter.h
    Created: 28 Feb 2024 1:09:33pm
    Author:  Przemysław Barski

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

template <typename SampleType>
class RMSMeters
{
public:
    //==============================================================================
    RMSMeters() {};
    float getInputRMS()
    {
        return inputRMS.get();
    }
    
    void setInputRMS(float RMS)
    {
        inputRMS.set(RMS);
    }
    
    float getGainReduction ()
    {
        return gainReduction.get();
    }
    
    void setGainReduction (float RMS)
    {
        gainReduction.set(RMS);
    }
    
private:
    juce::Atomic<float> inputRMS = 0.f;
    juce::Atomic<float> gainReduction = 0.f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RMSMeters)
};

template class RMSMeters<float>;
template class RMSMeters<double>;

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
class InputRMSMeter
{
public:
    //==============================================================================
    InputRMSMeter() {};
    float getInputRMS()
    {
        return inputRMS.get();
    }
    
    void setInputRMS(float RMS)
    {
        inputRMS.set(RMS);
    }
    
private:
    juce::Atomic<float> inputRMS = 0.f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InputRMSMeter)
};

template class InputRMSMeter<float>;
template class InputRMSMeter<double>;

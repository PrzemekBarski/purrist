/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "modules/processors/BuzzGate.h"
#include "modules/processors/HissGate.h"

struct ChainSettings
{
    float buzzThreshold { 1.f }, buzzRatio { 4.f }, buzzFrequency { 0 };
    float hissThreshold { 1.f }, hissRatio { 4.f }, hissCutoff { 0 };
    float noiseThreshold { 1.f }, noiseRatio { 4.f }, noiseRelease { 0 };
};

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts);

using NoiseGate = juce::dsp::NoiseGate<float>;
using MonoChain = juce::dsp::ProcessorChain<BuzzGate<float>, HissGate<float>, NoiseGate>;

enum ChainPositions
{
    buzzGate,
    hissGate,
    noiseGate
};

//==============================================================================
/**
*/
class PurristAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PurristAudioProcessor();
    ~PurristAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{*this, nullptr, "Parameters", createParameterLayout()};
    MonoChain chain[2];

private:
    void updateParameters();
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PurristAudioProcessor)
};

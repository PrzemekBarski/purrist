/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PurristAudioProcessor::PurristAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

PurristAudioProcessor::~PurristAudioProcessor()
{
}

//==============================================================================
const juce::String PurristAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PurristAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PurristAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PurristAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PurristAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PurristAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PurristAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PurristAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PurristAudioProcessor::getProgramName (int index)
{
    return {};
}

void PurristAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================

void PurristAudioProcessor::updateParameters ()
{
    auto chainSettings = getChainSettings(apvts);
    
    for (int channel = 0; channel < 2; channel++) {
        chain[channel].setBypassed<ChainPositions::buzzGate>(!chainSettings.buzzOn);
        chain[channel].get<ChainPositions::buzzGate>().setThreshold(chainSettings.buzzThreshold);
        chain[channel].get<ChainPositions::buzzGate>().setRatio(chainSettings.buzzRatio);
        chain[channel].get<ChainPositions::buzzGate>().setFrequencyID(chainSettings.buzzFrequency);
        
        chain[channel].setBypassed<ChainPositions::hissGate>(!chainSettings.hissOn);
        chain[channel].get<ChainPositions::hissGate>().setThreshold(chainSettings.hissThreshold);
        chain[channel].get<ChainPositions::hissGate>().setRatio(chainSettings.hissRatio);
        chain[channel].get<ChainPositions::hissGate>().setCutoff(chainSettings.hissCutoff);
        
        chain[channel].setBypassed<ChainPositions::noiseGate>(!chainSettings.noiseOn);
        chain[channel].get<ChainPositions::noiseGate>().setThreshold(chainSettings.noiseThreshold);
        chain[channel].get<ChainPositions::noiseGate>().setRatio(chainSettings.noiseRatio);
        chain[channel].get<ChainPositions::noiseGate>().setRelease(chainSettings.noiseRelease);
    }
}

void PurristAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;
    
    updateParameters();
    
    for (int channel = 0; channel < 2; channel++) {
        chain[channel].get<ChainPositions::buzzGate>().setAttack(5);
        chain[channel].get<ChainPositions::buzzGate>().setRelease(150);
        
        chain[channel].get<ChainPositions::hissGate>().setAttack(50);
        chain[channel].get<ChainPositions::hissGate>().setRelease(300);
        
        chain[channel].get<ChainPositions::noiseGate>().setAttack(5);
        
        chain[channel].prepare(spec);
    }
}

void PurristAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PurristAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PurristAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    updateParameters();

    juce::dsp::AudioBlock<float> block(buffer);
    
    auto leftBlock = block.getSingleChannelBlock(0);
    auto rightBlock = block.getSingleChannelBlock(1);
    
    juce::dsp::ProcessContextReplacing<float> leftContext(leftBlock);
    juce::dsp::ProcessContextReplacing<float> rightContext(rightBlock);
    
    chain[0].process(leftContext);
    chain[1].process(rightContext);
    
}

//==============================================================================
bool PurristAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PurristAudioProcessor::createEditor()
{
    return new PurristAudioProcessorEditor (*this);
//    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void PurristAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void PurristAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    
    if (tree.isValid()) {
        apvts.replaceState(tree);
        updateParameters();
    }
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;
    
    settings.buzzOn = apvts.getRawParameterValue("buzz_on")->load() > 0.5f;
    settings.buzzThreshold = apvts.getRawParameterValue("buzz_threshold")->load();
    settings.buzzRatio = apvts.getRawParameterValue("buzz_ratio")->load();
    settings.buzzFrequency = apvts.getRawParameterValue("buzz_frequency")->load();
    
    settings.hissOn = apvts.getRawParameterValue("hiss_on")->load() > 0.5f;
    settings.hissThreshold = apvts.getRawParameterValue("hiss_threshold")->load();
    settings.hissRatio = apvts.getRawParameterValue("hiss_ratio")->load();
    settings.hissCutoff = apvts.getRawParameterValue("hiss_cutoff")->load();
    
    settings.noiseOn = apvts.getRawParameterValue("noise_on")->load() > 0.5f;
    settings.noiseThreshold = apvts.getRawParameterValue("noise_threshold")->load();
    settings.noiseRatio = apvts.getRawParameterValue("noise_ratio")->load();
    settings.noiseRelease = apvts.getRawParameterValue("noise_release")->load();
    
    return settings ;
}

juce::AudioProcessorValueTreeState::ParameterLayout PurristAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("buzz_threshold", 1),
            "Buzz Threshold",
            juce::NormalisableRange<float>(-96.f, 0.f, 0.1f, 1.f),
            -42.f
        )
    );
    
    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("buzz_ratio", 1),
            "Buzz Ratio",
            juce::NormalisableRange<float>(1.5f, 4.f, 0.01f, 0.55f),
            2.f
        )
    );
    
    juce::StringArray buzzFreqOptions;
    buzzFreqOptions.add("50Hz");
    buzzFreqOptions.add("60Hz");
    
    layout.add(
        std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID("buzz_on", 1),
            "Buzz On",
            true
        )
    );
    
    layout.add(
        std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID("buzz_frequency", 1),
            "Buzz AC Frequency",
            buzzFreqOptions,
            0
        )
    );
    
    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("hiss_threshold", 1),
            "Fizz Threshold",
            juce::NormalisableRange<float>(-96.f, 0.f, 0.1f, 1.f),
            -48.f
        )
    );
    
    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("hiss_ratio", 1),
            "Fizz Ratio",
            juce::NormalisableRange<float>(1.5f, 4.f, 0.01f, 0.55f),
            2.f
        )
    );
    
    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("hiss_cutoff", 1),
            "Fizz Cutoff",
            juce::NormalisableRange<float>(1000.f, 4000.f, 0.5f, 0.55f),
            2000.f
        )
    );
    
    layout.add(
        std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID("hiss_on", 1),
            "Fizz On",
            true
        )
    );
    
    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("noise_threshold", 1),
            "Noise Threshold",
            juce::NormalisableRange<float>(-96.f, 0.f, 0.1f, 1.f),
            -54.f
        )
    );
    
    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("noise_ratio", 1),
            "Noise Ratio",
            juce::NormalisableRange<float>(1.5f, 4.f, 0.01f, 0.55f),
            3.f
        )
    );
    
    layout.add(
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("noise_release", 1),
            "Noise Release",
            juce::NormalisableRange<float>(0.f, 800.f, 1.f, 0.5f),
            200.f
        )
    );
    
    layout.add(
        std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID("noise_on", 1),
            "Noise On",
            true
        )
    );
    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PurristAudioProcessor();
}

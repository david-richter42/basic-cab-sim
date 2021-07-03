/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BasicAmpSimAudioProcessor::BasicAmpSimAudioProcessor()
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
    auto dir = juce::File::getCurrentWorkingDirectory();
    
    auto& convolution = processorChain.template get<convolutionIndex>();
    convolution.loadImpulseResponse(
            juce::File ("/Users/acanois/prog/audio/juce_projects/BasicAmpSim/Resources/guitar_amp.wav"),
            juce::dsp::Convolution::Stereo::yes,
            juce::dsp::Convolution::Trim::no,
            1024
    );}

BasicAmpSimAudioProcessor::~BasicAmpSimAudioProcessor()
{
}

//==============================================================================
const juce::String BasicAmpSimAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BasicAmpSimAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BasicAmpSimAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BasicAmpSimAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BasicAmpSimAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BasicAmpSimAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BasicAmpSimAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BasicAmpSimAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String BasicAmpSimAudioProcessor::getProgramName (int index)
{
    return {};
}

void BasicAmpSimAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void BasicAmpSimAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();
    
    auto& preGain = processorChain.template get<preGainIndex>();
    auto& postGain = processorChain.template get<postGainIndex>();
    
    preGain.setGainLinear(0.5f);
    postGain.setGainLinear(1.f);
    
    processorChain.prepare(spec);
}

void BasicAmpSimAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BasicAmpSimAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
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

void BasicAmpSimAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    juce::dsp::AudioBlock<float> processBlock (buffer);

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    processorChain.process (juce::dsp::ProcessContextReplacing<float> (processBlock));
}

//===============================s===============================================
bool BasicAmpSimAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* BasicAmpSimAudioProcessor::createEditor()
{
    return new BasicAmpSimAudioProcessorEditor (*this);
}

//==============================================================================
void BasicAmpSimAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BasicAmpSimAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BasicAmpSimAudioProcessor();
}
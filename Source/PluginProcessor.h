/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================

class HiSamplerAudioProcessor : public AudioProcessor,
                                public ValueTree::Listener
{
public:
    //==============================================================================
    HiSamplerAudioProcessor();
    ~HiSamplerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    void loadFile();
    void loadFile(const String& path);
    
    int getNumSamplerSounds() { return sampler.getNumSounds(); }
    AudioBuffer<float>& getWaveform() { return waveform; }
    
    void updateADSR();
    ADSR::Parameters& getADSRParams() { return ADSRParams; }
    
    AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    
    
private:
    
    Synthesiser sampler;
    const int numVoices { 3 };
    AudioBuffer<float> waveform;
    
    ADSR::Parameters ADSRParams;
    
    AudioFormatManager formatManager;
    AudioFormatReader* formatReader { nullptr };
    
    AudioProcessorValueTreeState apvts;
    AudioProcessorValueTreeState::ParameterLayout createParameters();
    
    std::atomic<bool> shouldUpdate { false };
    
    void valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const Identifier& property) override;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HiSamplerAudioProcessor)
};

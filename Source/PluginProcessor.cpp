/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HiSamplerAudioProcessor::HiSamplerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ), apvts (*this, nullptr, "PARAMETERS", createParameters())
#endif
{
    formatManager.registerBasicFormats();
    apvts.state.addListener(this);
    for (int i = 0; i < numVoices; i++) {
        sampler.addVoice (new SamplerVoice());
    }
}

HiSamplerAudioProcessor::~HiSamplerAudioProcessor() {
    formatReader = nullptr;
}

//==============================================================================
const String HiSamplerAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool HiSamplerAudioProcessor::acceptsMidi() const {
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool HiSamplerAudioProcessor::producesMidi() const {
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool HiSamplerAudioProcessor::isMidiEffect() const {
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double HiSamplerAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int HiSamplerAudioProcessor::getNumPrograms() {
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int HiSamplerAudioProcessor::getCurrentProgram() {
    return 0;
}

void HiSamplerAudioProcessor::setCurrentProgram (int index) {
}

const String HiSamplerAudioProcessor::getProgramName (int index) {
    return {};
}

void HiSamplerAudioProcessor::changeProgramName (int index, const String& newName) {
}

//==============================================================================
void HiSamplerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sampler.setCurrentPlaybackSampleRate(sampleRate);
    updateADSR();
}

void HiSamplerAudioProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool HiSamplerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const {
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
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

void HiSamplerAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) {
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        buffer.clear (i, 0, buffer.getNumSamples());
    }
    
    if (shouldUpdate) {
        updateADSR();
    }
    
    sampler.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

//==============================================================================
bool HiSamplerAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* HiSamplerAudioProcessor::createEditor() {
    return new HiSamplerAudioProcessorEditor (*this);
}

//==============================================================================
void HiSamplerAudioProcessor::getStateInformation (MemoryBlock& destData) {
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void HiSamplerAudioProcessor::setStateInformation (const void* data, int sizeInBytes) {
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void HiSamplerAudioProcessor::loadFile() {
    sampler.clearSounds();
    
    FileChooser chooser {"Please load a file!"};
    if (chooser.browseForFileToOpen()) {
        File file = chooser.getResult();
        formatReader = formatManager.createReaderFor(file);
    }
    BigInteger range;
    range.setRange(0, 128, true);
    sampler.addSound(new SamplerSound("Sample", // const String &name
                                      *formatReader, // AudioFormatReader &source
                                      range, // const BigInteger &midinotes
                                      60, // int midiNoteForNormalPitch
                                      0.001, // double attackTimeSecs
                                      0.001, // double releaseTimeSecs
                                      10.0)); // double maxSampleLengthSeconds
}

void HiSamplerAudioProcessor::loadFile(const String& path) {
    sampler.clearSounds();
    
    File file = File (path);
    formatReader = formatManager.createReaderFor(file);
    
    auto sampleLength = static_cast<int>(formatReader->lengthInSamples);
    
    waveform.setSize(1, sampleLength);
    formatReader->read(&waveform, 0, sampleLength, 0, true, false);
    
    BigInteger range;
    range.setRange(0, 128, true);
    sampler.addSound(new SamplerSound("Sample", // const String &name
                                      *formatReader, // AudioFormatReader &source
                                      range, // const BigInteger &midinotes
                                      60, // int midiNoteForNormalPitch
                                      0.001, // double attackTimeSecs
                                      0.001, // double releaseTimeSecs
                                      10.0)); // double maxSampleLengthSeconds
}

void HiSamplerAudioProcessor::updateADSR() {
    ADSRParams.attack = apvts.getRawParameterValue("ATTACK")->load(); // uses std::atomic
    ADSRParams.decay = apvts.getRawParameterValue("DECAY")->load();
    ADSRParams.sustain = apvts.getRawParameterValue("SUSTAIN")->load();
    ADSRParams.release = apvts.getRawParameterValue("RELEASE")->load();
    
    for (int i = 0; i < sampler.getNumSounds(); i++) {
        if (auto sound = dynamic_cast<SamplerSound*>(sampler.getSound(i).get())) { // dynamic casting to make sure we're working with a sampler sound, NOT a synthesizer sound
            sound->setEnvelopeParameters(ADSRParams);
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new HiSamplerAudioProcessor();
}

AudioProcessorValueTreeState::ParameterLayout HiSamplerAudioProcessor::createParameters() {
    std::vector<std::unique_ptr<RangedAudioParameter>> parameters;
    
    parameters.push_back (std::make_unique<AudioParameterFloat>("ATTACK", "Attack", 0.0f, 2.0f, 0.0f));
    parameters.push_back (std::make_unique<AudioParameterFloat>("DECAY", "Decay", 0.0f, 2.0f, 0.0f));
    parameters.push_back (std::make_unique<AudioParameterFloat>("SUSTAIN", "Sustain", 0.0f, 1.0f, 1.0f));
    parameters.push_back (std::make_unique<AudioParameterFloat>("RELEASE", "Release", 0.0f, 5.0f, 0.0f));

    return { parameters.begin(), parameters.end() };
}

void HiSamplerAudioProcessor::valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const Identifier& property) {
    shouldUpdate = true;
}

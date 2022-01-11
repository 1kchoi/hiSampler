/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class HiSamplerAudioProcessorEditor   : public juce::AudioProcessorEditor,
                                        public FileDragAndDropTarget
{
public:
    HiSamplerAudioProcessorEditor (HiSamplerAudioProcessor&);
    ~HiSamplerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    bool isInterestedInFileDrag (const StringArray& files) override;
    void filesDropped (const StringArray& files, int x, int y) override;
    
private:
    TextButton loadButton { "Load a sample!" };
    std::vector<float> audioPoints;
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    HiSamplerAudioProcessor& audioProcessor;
    //bool shouldBePainting { false };
    
    Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    Label attackLabel, decayLabel, sustainLabel, releaseLabel;
    
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> sustainAttachment;
    std::unique_ptr<AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HiSamplerAudioProcessorEditor)
};

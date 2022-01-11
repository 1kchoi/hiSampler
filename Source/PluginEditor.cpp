/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HiSamplerAudioProcessorEditor::HiSamplerAudioProcessorEditor (HiSamplerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    loadButton.onClick = [&] {
        audioProcessor.loadFile();
    };
    addAndMakeVisible(loadButton);
    
    Colour attackColour = Colours::cyan;
    attackSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    attackSlider.setTextBoxStyle(Slider::TextBoxBelow, true, 40, 20);
    attackSlider.setRange(0.0f, 2.0f, 0.01f);
    // attackSlider.setValue(0.0);
    attackSlider.setColour(Slider::ColourIds::thumbColourId, attackColour);
    addAndMakeVisible(attackSlider);
    attackLabel.setFont(10.f);
    attackLabel.setText("Attack", NotificationType::dontSendNotification);
    attackLabel.setColour(Label::ColourIds::textColourId, attackColour);
    attackLabel.setJustificationType(Justification::centredTop);
    attackLabel.attachToComponent(&attackSlider, false);
    
    Colour decayColour = Colours::yellow;
    decaySlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    decaySlider.setTextBoxStyle(Slider::TextBoxBelow, true, 40, 20);
    decaySlider.setRange(0.0f, 2.0f, 0.01f);
    // decaySlider.setValue(0.0);
    decaySlider.setColour(Slider::ColourIds::thumbColourId, decayColour);
    addAndMakeVisible(decaySlider);
    decayLabel.setFont(10.f);
    decayLabel.setText("Decay", NotificationType::dontSendNotification);
    decayLabel.setColour(Label::ColourIds::textColourId, decayColour);
    decayLabel.setJustificationType(Justification::centredTop);
    decayLabel.attachToComponent(&decaySlider, false);
    
    Colour sustainColour = Colours::magenta;
    sustainSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    sustainSlider.setTextBoxStyle(Slider::TextBoxBelow, true, 40, 20);
    sustainSlider.setRange(0.0f, 1.0f, 0.01f);
    // sustainSlider.setValue(1.0);
    sustainSlider.setColour(Slider::ColourIds::thumbColourId, sustainColour);
    addAndMakeVisible(sustainSlider);
    sustainLabel.setFont(10.f);
    sustainLabel.setText("Sustain", NotificationType::dontSendNotification);
    sustainLabel.setColour(Label::ColourIds::textColourId, sustainColour);
    sustainLabel.setJustificationType(Justification::centredTop);
    sustainLabel.attachToComponent(&sustainSlider, false);
    
    Colour releaseColour = Colours::white;
    releaseSlider.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
    releaseSlider.setTextBoxStyle(Slider::TextBoxBelow, true, 40, 20);
    releaseSlider.setRange(0.0f, 5.0f, 0.01f);
    // releaseSlider.setValue(0.0);
    releaseSlider.setColour(Slider::ColourIds::thumbColourId, releaseColour);
    addAndMakeVisible(releaseSlider);
    releaseLabel.setFont(10.f);
    releaseLabel.setText("Release", NotificationType::dontSendNotification);
    releaseLabel.setColour(Label::ColourIds::textColourId, releaseColour);
    releaseLabel.setJustificationType(Justification::centredTop);
    releaseLabel.attachToComponent(&releaseSlider, false);
    
    /*
    audioProcessor.getADSRParams().attack = 0.0;
    audioProcessor.getADSRParams().decay = 0.0;
    audioProcessor.getADSRParams().sustain = 1.0;
    audioProcessor.getADSRParams().release = 0.0;
    */
    
    attackAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getAPVTS(), "ATTACK", attackSlider);
    decayAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getAPVTS(), "DECAY", decaySlider);
    sustainAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getAPVTS(), "SUSTAIN", sustainSlider);
    releaseAttachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.getAPVTS(), "RELEASE", releaseSlider);
    
    
    setSize (600, 200);
}

HiSamplerAudioProcessorEditor::~HiSamplerAudioProcessorEditor() {}

//==============================================================================
void HiSamplerAudioProcessorEditor::paint (juce::Graphics& g) {
    g.fillAll(Colours::black);
    g.setColour(Colours::white);
    
    
    // if (shouldBePainting) {
        Path p;
        audioPoints.clear();
        
        AudioBuffer<float>& waveform = audioProcessor.getWaveform();
        auto ratio = waveform.getNumSamples() / getWidth();
        auto buffer = waveform.getReadPointer(0);
        
        // scale audio file to window on x axis
        for (int sample = 0; sample < waveform.getNumSamples(); sample += ratio) {
            audioPoints.push_back(buffer[sample]);
        }
        
        p.startNewSubPath(0, getHeight() / 2);
        
        for (int sample = 0; sample < audioPoints.size(); ++sample) {
            float point = jmap<float>(audioPoints[sample], -1.0f, 1.0f, 200, 0);
            p.lineTo(sample, point);
        }
        
        g.strokePath(p, PathStrokeType(2));
        
        // shouldBePainting = false;
    // }
}

void HiSamplerAudioProcessorEditor::resized() {
    const float startX = 0.6f;
    const float startY = 0.6f;
    const float dialWidth = 0.1f;
    const float dialHeight = 0.4f;
    
    attackSlider.setBoundsRelative(startX, startY, dialWidth, dialHeight);
    decaySlider.setBoundsRelative(startX + dialWidth, startY, dialWidth, dialHeight);
    sustainSlider.setBoundsRelative(startX + 2 * dialWidth, startY, dialWidth, dialHeight);
    releaseSlider.setBoundsRelative(startX + 3 * dialWidth, startY, dialWidth, dialHeight);
}

bool HiSamplerAudioProcessorEditor::isInterestedInFileDrag (const StringArray& files) {
    for (auto file : files) {
        if (file.contains(".wav") || file.contains(".mp3") || file.contains(".aif")) {
            return true;
        }
    }
    return false;
}

void HiSamplerAudioProcessorEditor::filesDropped (const StringArray& files, int x, int y) {
    for (auto file : files) {
        // shouldBePainting = true;
        if (isInterestedInFileDrag(file)) {
            audioProcessor.loadFile(file);
        }
    }
    audioProcessor.updateADSR();
    repaint();
}

#pragma once

#include "PluginProcessor.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final
    : public juce::AudioProcessorEditor {
public:
  explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &);
  ~AudioPluginAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics &) override;
  void resized() override;

private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  AudioPluginAudioProcessor &processorRef;

  using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

  juce::Slider gainSlider;
  juce::Slider numGrainsSlider;
  juce::Slider spawnIntervalSlider;
  juce::Slider grainDurationSlider;
  juce::Slider playheadPositionSlider;
  juce::Slider spraySlider;

  juce::TextButton selectSampleButton;
  void openButtonClicked();

  std::unique_ptr<SliderAttachment> gainSliderAttachment;
  std::unique_ptr<SliderAttachment> numGrainsSliderAttachment;
  std::unique_ptr<SliderAttachment> spawnIntervalSliderAttachment;
  std::unique_ptr<SliderAttachment> grainDurationSliderAttachment;
  std::unique_ptr<SliderAttachment> playheadPositionSliderAttachment;
  std::unique_ptr<SliderAttachment> spraySliderAttachment;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};

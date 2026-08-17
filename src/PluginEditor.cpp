#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor &p)
    : AudioProcessorEditor(&p), processorRef(p) {
  juce::ignoreUnused(processorRef);
  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  setSize(600, 300);

  // Set properties of gain slider
  gainSlider.setSliderStyle(juce::Slider::LinearVertical);
  gainSlider.setTextValueSuffix(" Gain");
  addAndMakeVisible(&gainSlider);
  gainSliderAttachment = std::make_unique<SliderAttachment>(processorRef.apvts,
                                                            "gain", gainSlider);

  // Set properties of Num Grains slider
  numGrainsSlider.setSliderStyle(juce::Slider::LinearVertical);
  numGrainsSlider.setTextValueSuffix("\nNum Grains");
  addAndMakeVisible(&numGrainsSlider);
  numGrainsSliderAttachment = std::make_unique<SliderAttachment>(
      processorRef.apvts, "numGrains", numGrainsSlider);

  // Set properties of Spawn Interval slider
  spawnIntervalSlider.setSliderStyle(juce::Slider::LinearVertical);
  spawnIntervalSlider.setTextValueSuffix("\nSpawn Interval");
  addAndMakeVisible(&spawnIntervalSlider);
  spawnIntervalSliderAttachment = std::make_unique<SliderAttachment>(
      processorRef.apvts, "spawnInt", spawnIntervalSlider);

  // Set properties of Grain Duration slider
  grainDurationSlider.setSliderStyle(juce::Slider::LinearVertical);
  grainDurationSlider.setTextValueSuffix("\nGrain Duration");
  addAndMakeVisible(&grainDurationSlider);
  grainDurationSliderAttachment = std::make_unique<SliderAttachment>(
      processorRef.apvts, "grainDur", grainDurationSlider);

  // Set properties of Playhead Position slider
  playheadPositionSlider.setSliderStyle(juce::Slider::LinearVertical);
  playheadPositionSlider.setTextValueSuffix("\nPlayhead Position");
  addAndMakeVisible(&playheadPositionSlider);
  playheadPositionSliderAttachment = std::make_unique<SliderAttachment>(
      processorRef.apvts, "playheadPos", playheadPositionSlider);

  // Set properties of Spray slider
  spraySlider.setSliderStyle(juce::Slider::LinearVertical);
  spraySlider.setTextValueSuffix("\nSpray");
  addAndMakeVisible(&spraySlider);
  spraySliderAttachment = std::make_unique<SliderAttachment>(
      processorRef.apvts, "spray", spraySlider);

  addAndMakeVisible(&selectSampleButton);
  selectSampleButton.setButtonText("Open...");
  selectSampleButton.onClick = [this] { openButtonClicked(); };
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics &g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

  g.setColour(juce::Colours::white);
  g.setFont(15.0f);
}

void AudioPluginAudioProcessorEditor::resized() {
  auto area = getLocalBounds().reduced(10);

  int numSliders = 6;
  int sliderWidth = area.getWidth() / numSliders;
  int sliderHeight = area.getHeight();

  gainSlider.setBounds(area.removeFromLeft(sliderWidth));
  gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                             static_cast<int>(sliderWidth * 0.8f),
                             static_cast<int>(sliderHeight * 0.15f));

  numGrainsSlider.setBounds(area.removeFromLeft(sliderWidth));
  numGrainsSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                  static_cast<int>(sliderWidth * 0.8f),
                                  static_cast<int>(sliderHeight * 0.15f));

  spawnIntervalSlider.setBounds(area.removeFromLeft(sliderWidth));
  spawnIntervalSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                      static_cast<int>(sliderWidth * 0.8f),
                                      static_cast<int>(sliderHeight * 0.15f));

  grainDurationSlider.setBounds(area.removeFromLeft(sliderWidth));
  grainDurationSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                                      static_cast<int>(sliderWidth * 0.8f),
                                      static_cast<int>(sliderHeight * 0.15f));

  playheadPositionSlider.setBounds(area.removeFromLeft(sliderWidth));
  playheadPositionSlider.setTextBoxStyle(
      juce::Slider::TextBoxBelow, false, static_cast<int>(sliderWidth * 0.8f),
      static_cast<int>(sliderHeight * 0.15f));

  spraySlider.setBounds(area.removeFromLeft(sliderWidth));
  spraySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false,
                              static_cast<int>(sliderWidth * 0.8f),
                              static_cast<int>(sliderHeight * 0.15f));
}

void AudioPluginAudioProcessorEditor::openButtonClicked() {}

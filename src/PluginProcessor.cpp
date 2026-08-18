#include "PluginProcessor.h"
#include "BinaryData.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
              ),
      apvts(*this, nullptr, "PARAMETERS", createParameterLayout()) {
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const {
  return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
  return true;
#else
  return false;
#endif
}

bool AudioPluginAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
  return true;
#else
  return false;
#endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
  return true;
#else
  return false;
#endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int AudioPluginAudioProcessor::getNumPrograms() {
  return 1; // NB: some hosts don't cope very well if you tell them there are 0
            // programs, so this should be at least 1, even if you're not really
            // implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram() { return 0; }

void AudioPluginAudioProcessor::setCurrentProgram(int index) {
  juce::ignoreUnused(index);
}

const juce::String AudioPluginAudioProcessor::getProgramName(int index) {
  juce::ignoreUnused(index);
  return {};
}

void AudioPluginAudioProcessor::changeProgramName(int index,
                                                  const juce::String &newName) {
  juce::ignoreUnused(index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay(double sampleRate,
                                              int samplesPerBlock) {
  if (audioReservoir.getNumSamples() == 0) {
    // 1. Tell the manager to register basic formats like WAV and AIFF
    formatManager.registerBasicFormats();

    // 2. Create a memory stream pointing directly to your embedded binary file
    auto memoryStream = std::make_unique<juce::MemoryInputStream>(
        BinaryData::OTH_128_Gm_Forward_Synth_wav,
        BinaryData::OTH_128_Gm_Forward_Synth_wavSize, false);

    // 3. Find a reader that can understand the stream data
    std::unique_ptr<juce::AudioFormatReader> reader(
        formatManager.createReaderFor(std::move(memoryStream)));

    if (reader != nullptr) {
      // 4. Resize our reservoir buffer to match the audio file channels and
      // length
      audioReservoir.setSize(reader->numChannels,
                             static_cast<int>(reader->lengthInSamples));

      // 5. Read the samples from the file stream right into our buffer
      reader->read(&audioReservoir,
                   0, // Start at sample 0 in reservoir
                   static_cast<int>(reader->lengthInSamples),
                   0,     // Start at sample 0 in file
                   true,  // Read left channel
                   true); // Read right channel

      std::cout << "Successfully loaded sample! Length: "
                << audioReservoir.getNumSamples() << " samples." << std::endl;
    } else {
      std::cout << "Failed to read binary audio data!" << std::endl;
    }

    granularEngine = std::make_unique<GranularEngine>(
        audioReservoir, reader->lengthInSamples,
        apvts.getRawParameterValue("grainDur"),
        apvts.getRawParameterValue("gain"),
        apvts.getRawParameterValue("numGrains"),
        apvts.getRawParameterValue("spawnInt"),
        apvts.getRawParameterValue("playheadPos"),
        apvts.getRawParameterValue("spray"));
  }
}

void AudioPluginAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(
    const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
  juce::ignoreUnused(layouts);
  return true;
#else
  // This is the place where you check if the layout is supported.
  // In this template code we only support mono or stereo.
  // Some plugin hosts, such as certain GarageBand versions, will only
  // load plugins that support stereo bus layouts.
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

  // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
#endif
}

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                             juce::MidiBuffer &midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  auto totalNumInputChannels = getTotalNumInputChannels();
  auto totalNumOutputChannels = getTotalNumOutputChannels();
  int bufferNumSamples = buffer.getNumSamples();

  for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
    buffer.clear(i, 0, bufferNumSamples);
  }

  const juce::ScopedLock lock(audioLock);
  granularEngine->processBlock(buffer);
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const {
  return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *AudioPluginAudioProcessor::createEditor() {
  return new AudioPluginAudioProcessorEditor(*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation(
    juce::MemoryBlock &destData) {
  // You should use this method to store your parameters in the memory block.
  // You could do that either as raw data, or use the XML or ValueTree classes
  // as intermediaries to make it easy to save and load complex data.
  juce::ignoreUnused(destData);
}

void AudioPluginAudioProcessor::setStateInformation(const void *data,
                                                    int sizeInBytes) {
  // You should use this method to restore your parameters from this memory
  // block, whose contents will have been created by the getStateInformation()
  // call.
  juce::ignoreUnused(data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new AudioPluginAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout
AudioPluginAudioProcessor::createParameterLayout() {
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

  auto stringFromValue = [](float value, int) {
    return juce::String(value, 2);
  };

  auto valueFromString = [](juce::String text) { return text.getFloatValue(); };

  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      "gain", "Gain", juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f, "",
      juce::AudioProcessorParameter::genericParameter, stringFromValue,
      valueFromString));
  params.push_back(std::make_unique<juce::AudioParameterInt>(
      "numGrains", "Number of Grains", 1, GranularEngine::maxGrains, 50));
  params.push_back(std::make_unique<juce::AudioParameterInt>(
      "spawnInt", "Spawn Interval", 1, 4410, 882));
  params.push_back(std::make_unique<juce::AudioParameterInt>(
      "grainDur", "Grain Duration", 100, 44100, 4410));
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      "playheadPos", "Playhead Position",
      juce::NormalisableRange<float>(0.0f, 1.0f), 0.5f, "",
      juce::AudioProcessorParameter::genericParameter, stringFromValue,
      valueFromString));
  params.push_back(std::make_unique<juce::AudioParameterFloat>(
      "spray", "Spray", juce::NormalisableRange<float>(0.0f, 1.0f), 0.1f, "",
      juce::AudioProcessorParameter::genericParameter, stringFromValue,
      valueFromString));

  return {params.begin(), params.end()};
}

void AudioPluginAudioProcessor::loadAudioFile(const juce::File &file) {
  std::unique_ptr<juce::AudioFormatReader> reader(
      formatManager.createReaderFor(file));

  juce::AudioBuffer<float> tempBuffer;
  tempBuffer.setSize(reader->numChannels,
                     static_cast<int>(reader->lengthInSamples));

  reader->read(&tempBuffer, 0, static_cast<int>(reader->lengthInSamples), 0,
               true, true);
  const juce::ScopedLock lock(audioLock);
  audioReservoir.makeCopyOf(tempBuffer);

  granularEngine = std::make_unique<GranularEngine>(
      audioReservoir, reader->lengthInSamples,
      apvts.getRawParameterValue("grainDur"),
      apvts.getRawParameterValue("gain"),
      apvts.getRawParameterValue("numGrains"),
      apvts.getRawParameterValue("spawnInt"),
      apvts.getRawParameterValue("playheadPos"),
      apvts.getRawParameterValue("spray"));
}
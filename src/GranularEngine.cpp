#include "GranularEngine.h"

GranularEngine::GranularEngine(
    juce::AudioBuffer<float> &audioRes, int audioSize,
    std::atomic<float> *grainSampleDur, std::atomic<float> *grainG,
    std::atomic<float> *nGrains, std::atomic<float> *spawnInt,
    std::atomic<float> *playheadPos, std::atomic<float> *spr)
    : audioReservoir(audioRes), audioSampleSize(audioSize), numGrains(nGrains),
      spawnInterval(spawnInt), grainSampleDuration(grainSampleDur),
      grainGain(grainG), playheadPosition(playheadPos), spray(spr) {
  grains.resize(maxGrains);
}

GranularEngine::~GranularEngine() {}

void GranularEngine::processBlock(juce::AudioBuffer<float> &buffer) {
  int bufferNumSamples = buffer.getNumSamples();
  int numChannelsOutput = buffer.getNumChannels();

  int grainSampleDur = static_cast<int>(grainSampleDuration->load());
  float grainG = grainGain->load();
  int spawnInt = static_cast<int>(spawnInterval->load());
  int nGrains = static_cast<int>(numGrains->load());

  // 1. Run the clock and spawn all new grains for this block
  for (size_t i = 0; i < bufferNumSamples; i++) {
    spawnSampleClock += 1;
    if (spawnSampleClock > spawnInt) {
      spawnSampleClock = 0;

      int activeCount = 0;
      for (const auto &g : grains) {
        if (g.isActive)
          activeCount++;
      }

      if (activeCount < nGrains) {
        for (auto &grain : grains) {
          if (!grain.isActive) {
            int startingSample = getGrainStartingSample();
            grain.spawn(i, startingSample, grainSampleDur, grainG);
            break;
          }
        }
      }
    }
  }

  // 2. Go over all grains and mix into output buffer
  for (auto &grain : grains) {
    if (!grain.isActive)
      continue;

    int samplesLeftInGrain =
        grain.durationInSamples - static_cast<int>(grain.offsetSample);
    int samplesLeftInBuffer = bufferNumSamples - grain.bufferStartIndex;
    int samplesToProcess = std::min(samplesLeftInGrain, samplesLeftInBuffer);
    size_t loopLimit = grain.bufferStartIndex + samplesToProcess;

    for (size_t sampleIndex = grain.bufferStartIndex; sampleIndex < loopLimit;
         sampleIndex++) {
      float envelope = grain.getWindowEnvelope();

      // Calculate exact reading frame
      int sourceReadIndex =
          grain.startSample + static_cast<int>(grain.offsetSample);

      // Safety fallback wrap-around if sample position exceeds total audio
      // bounds
      if (sourceReadIndex >= audioSampleSize) {
        sourceReadIndex = audioSampleSize - 1;
      }

      for (size_t channel = 0; channel < numChannelsOutput; channel++) {
        auto audioSample = audioReservoir.getSample(
            channel % audioReservoir.getNumChannels(), sourceReadIndex);

        float processedSample = audioSample * envelope * grain.grainGain;
        buffer.addSample(channel, sampleIndex, processedSample);
      }

      grain.offsetSample += 1.0f;
    }

    if (grain.offsetSample >= grain.durationInSamples) {
      grain.disable();
    }
    grain.bufferStartIndex = 0;
  }
}

int GranularEngine::getGrainStartingSample() {
  float playheadPos = playheadPosition->load();
  float spr = spray->load();
  float grainSampleDur = grainSampleDuration->load();

  int centerSample = static_cast<int>(playheadPos * audioSampleSize);
  int sparySamples = static_cast<int>(spr * audioSampleSize);

  int randomOffset = 0;
  if (sparySamples > 0) {
    randomOffset = juce::Random::getSystemRandom().nextInt(sparySamples * 2) -
                   sparySamples;
  }

  int startingSample = centerSample + randomOffset;
  int maxSafeSample =
      std::max(0, static_cast<int>(audioSampleSize - grainSampleDur));
  return std::clamp(startingSample, 0, maxSafeSample);
}
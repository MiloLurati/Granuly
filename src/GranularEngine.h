#pragma once

#include <cmath>
#include <juce_audio_basics/juce_audio_basics.h>

struct Grain {
  int startSample = 0; // where the grain starts inside the source audio file
  float offsetSample =
      0.0f; // the current offset we are at while the grain is playing
  bool isActive = false;
  int durationInSamples = 0;
  float grainGain = 0;
  int bufferStartIndex = 0;

  void spawn(int bufferStartingIndex, int startingSample, int duration,
             float gain) {
    startSample = startingSample;
    durationInSamples = duration;
    grainGain = gain;
    isActive = true;
    offsetSample = 0.0f;
    bufferStartIndex = bufferStartingIndex;
  }

  void disable() {
    startSample = 0;
    offsetSample = 0.0f;
    isActive = false;
    durationInSamples = 0;
    grainGain = 0;
    bufferStartIndex = 0;
  }

  float getWindowEnvelope() const {
    if (durationInSamples <= 0)
      return 0.0f;

    // Calculate a normalized position between 0.0 (start) and 1.0 (end)
    float normalizedProgress =
        offsetSample / static_cast<float>(durationInSamples);

    // Safety check: if we somehow overshot, clip the envelope to silence
    if (normalizedProgress >= 1.0f)
      return 0.0f;

    // Standard Hann window formula: 0.5 * (1.0 - cos(2 * PI * progress))
    // This generates a bell curve that starts at 0, peaks at 1, and ends at 0.
    return 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi *
                                   normalizedProgress));
  }
};

class GranularEngine {
public:
  static constexpr int maxGrains = 500;
  static constexpr int defaultSpawnInterval = 882;

  GranularEngine(juce::AudioBuffer<float> &audioReservoir, int audioSize,
                 std::atomic<float> *grainSampleDur, std::atomic<float> *grainG,
                 std::atomic<float> *nGrains, std::atomic<float> *spawnInt,
                 std::atomic<float> *playheadPos, std::atomic<float> *spr);
  ~GranularEngine();
  void processBlock(juce::AudioBuffer<float> &buffer);

private:
  int audioSampleSize;
  std::atomic<float> *numGrains;
  std::atomic<float> *spawnInterval;
  std::atomic<float> *grainSampleDuration;
  std::atomic<float> *grainGain;
  std::atomic<float> *playheadPosition;
  std::atomic<float> *spray;
  int spawnSampleClock = 0;
  std::vector<Grain> grains;
  juce::AudioBuffer<float> &audioReservoir;

  int getGrainStartingSample();
};
#pragma once

// A6: offline STFT spectrogram. Compute once per track → a flat
// row-major float buffer; render viewport-sized BGRA images on
// demand from it, so zoom / scroll just resample the existing data
// without touching the FFT path.

#include <cstdint>
#include <vector>

class AudioManager;

struct Spectrogram {
    // Output grid dimensions.
    int frames = 0;
    int bins = 0;         // frameSize / 2 + 1
    // Frame spacing in milliseconds.
    float hopMS = 0.0f;
    int sampleRate = 0;
    int frameSize = 0;
    // Row-major: magnitudes[frame * bins + bin].
    std::vector<float> magnitudes;
    // Maximum magnitude in the buffer — used by the renderer so dB
    // normalisation is consistent regardless of track loudness.
    float peakMag = 0.0f;
};

struct SpectrogramOptions {
    int frameSize = 1024;
    int hopSize = 512;
};

Spectrogram ComputeSpectrogram(AudioManager* audio,
                                const SpectrogramOptions& opts = SpectrogramOptions{});

// Resample the spectrogram into an `outWidth x outHeight` BGRA
// buffer covering [startMS, endMS]. Log-frequency y-axis, dB-scaled
// magnitudes clipped to `dBFloor..0`, magenta→yellow colormap. The
// caller owns `outBGRA`; it will be resized to `outWidth * outHeight
// * 4` bytes.
void RenderSpectrogramBGRA(const Spectrogram& sg,
                            long startMS, long endMS,
                            int outWidth, int outHeight,
                            std::vector<uint8_t>& outBGRA,
                            float dBFloor = -80.0f);

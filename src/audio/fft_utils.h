#pragma once

enum class FFTWindowType
{
    Rectangular,
    Hamming,
    Hann,
    Blackman
};

void GetWindow(FFTWindowType type, float* window, size_t count);

void fft(const float* in, float* out, size_t count);
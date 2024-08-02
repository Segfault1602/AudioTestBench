#include "fft_utils.h"

#include <cassert>
#include <cmath>
#include <cstring>

#include <pffft.h>

namespace
{
constexpr float k_two_pi = 2 * 3.14159265358979323846f;
}

void GetWindow(FFTWindowType type, float* window, size_t count)
{
    assert(window != nullptr);

    switch (type)
    {
    case FFTWindowType::Rectangular:
        for (size_t i = 0; i < count; ++i)
        {
            window[i] = 1.0f;
        }
        break;
    case FFTWindowType::Hamming:
    {
        constexpr float alpha = 0.54f;
        constexpr float beta = 1.0f - alpha;
        for (size_t i = 0; i < count; ++i)
        {
            window[i] = alpha - beta * std::cos(k_two_pi * i / (count - 1));
        }
        break;
    }
    case FFTWindowType::Hann:
    {
        for (size_t i = 0; i < count; ++i)
        {
            window[i] = 0.5f * (1.0f - std::cos(k_two_pi * i / (count - 1)));
        }
        break;
    }
    case FFTWindowType::Blackman:
    {
        for (size_t i = 0; i < count; ++i)
        {
            window[i] =
                0.42f - 0.5 * std::cos(k_two_pi * i / (count - 1)) + 0.08f * std::cos(2 * k_two_pi * i / (count - 1));
        }
        break;
    }
    }
}

void fft(const float* in, float* out, size_t count)
{
    const size_t kSize = count * sizeof(float);
    float* aligned_in = static_cast<float*>(pffft_aligned_malloc(kSize));
    float* aligned_out = static_cast<float*>(pffft_aligned_malloc(kSize));

    memcpy(static_cast<void*>(aligned_in), static_cast<const void*>(in), kSize);

    PFFFT_Setup* setup = pffft_new_setup(count, PFFFT_REAL);
    pffft_transform_ordered(setup, aligned_in, aligned_out, nullptr, PFFFT_FORWARD);
    pffft_destroy_setup(setup);

    memcpy(static_cast<void*>(out), static_cast<void*>(aligned_out), kSize);

    pffft_aligned_free(aligned_in);
    pffft_aligned_free(aligned_out);
}
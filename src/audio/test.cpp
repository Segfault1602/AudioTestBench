#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include <sndfile.h>

#include "ring_buffer.h"

constexpr char audio_file[] = "C:\\source\\libdsp\\waves\\piano.wav";

RingBuffer<float> g_ring_buffer;

SF_INFO input_info;
SNDFILE* input_file;

void producer_thread()
{
    if (!input_file)
    {
        std::cerr << "Failed to open audio file: " << audio_file << std::endl;
        return;
    }

    // Not perfect, but simulate an audio thread
    constexpr uint32_t block_size = 512;
    const uint32_t period_seconds = input_info.samplerate / block_size;
    const uint32_t period_milliseconds = 1000 / period_seconds;

    std::cout << "Period: " << period_milliseconds << "ms" << std::endl;

    std::vector<float> buffer(block_size * input_info.channels);
    for (;;)
    {
        const auto read = sf_readf_float(input_file, buffer.data(), block_size);
        if (read == 0)
        {
            break;
        }

        g_ring_buffer.Write(buffer.data(), read * input_info.channels);

        std::this_thread::sleep_for(std::chrono::milliseconds(period_milliseconds));
    }

    sf_close(input_file);
}

void consumer()
{
    SF_INFO info;
    info.channels = input_info.channels;
    info.samplerate = input_info.samplerate;
    info.format = input_info.format;
    SNDFILE* file = sf_open("out.wav", SFM_WRITE, &info);

    constexpr uint32_t block_size = 512;
    const uint32_t period_milliseconds = 1000 / 60;

    size_t total_size_read = 0;

    std::vector<float> buffer(block_size);
    for (;;)
    {
        size_t read_available = g_ring_buffer.GetReadAvailable();
        if (read_available == 0)
        {
            std::cout << "No data to read!" << std::endl;
            continue;
        }

        while (read_available > 0)
        {
            size_t read = block_size;
            g_ring_buffer.Read(buffer.data(), read);

            if (read == 0)
            {
                std::cout << "No data to read! That's weird" << std::endl;
            }

            if (read > 0)
            {
                total_size_read += read;
                read_available -= read;
                sf_writef_float(file, buffer.data(), read);
            }
        }

        if (total_size_read >= input_info.frames * input_info.channels)
        {
            std::cout << "Done reading" << std::endl;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(period_milliseconds));
    }

    sf_close(file);
}

void easy_test()
{
    if (!input_file)
    {
        std::cerr << "Failed to open audio file: " << audio_file << std::endl;
        return;
    }

    SF_INFO info;
    info.channels = input_info.channels;
    info.samplerate = input_info.samplerate;
    info.format = input_info.format;
    SNDFILE* file = sf_open("out.wav", SFM_WRITE, &info);

    constexpr uint32_t block_size = 512;

    std::vector<float> read_buffer(block_size * input_info.channels);
    std::vector<float> write_buffer(block_size);
    for (;;)
    {
        const auto read = sf_readf_float(input_file, read_buffer.data(), block_size);
        g_ring_buffer.Write(read_buffer.data(), read * input_info.channels);

        size_t read_from_buffer = block_size;
        g_ring_buffer.Read(write_buffer.data(), read_from_buffer);

        sf_writef_float(file, write_buffer.data(), read_from_buffer);

        if (read == 0)
        {
            break;
        }
    }

    sf_close(file);
    sf_close(input_file);
}

bool compare_files()
{
    SF_INFO original_info;
    SNDFILE* original_file = sf_open(audio_file, SFM_READ, &original_info);

    SF_INFO out_info;
    SNDFILE* out_file = sf_open("out.wav", SFM_READ, &out_info);

    if (original_info.channels != out_info.channels)
    {
        std::cout << "Channels mismatch" << std::endl;
        return false;
    }

    if (original_info.samplerate != out_info.samplerate)
    {
        std::cout << "Sample rate mismatch" << std::endl;
        return false;
    }

    if (original_info.frames != out_info.frames)
    {
        std::cout << "Frames mismatch" << std::endl;
        return false;
    }

    constexpr uint32_t block_size = 512;
    std::vector<float> original_buffer(block_size * original_info.channels);
    std::vector<float> out_buffer(block_size * out_info.channels);
    for (;;)
    {
        const auto original_read = sf_readf_float(original_file, original_buffer.data(), block_size);
        const auto out_read = sf_readf_float(out_file, out_buffer.data(), block_size);

        if (original_read != out_read)
        {
            std::cout << "Read size mismatch" << std::endl;
            return false;
        }

        if (original_read == 0)
        {
            break;
        }

        for (size_t i = 0; i < original_read; ++i)
        {
            if (original_buffer[i] != out_buffer[i])
            {
                std::cout << "Data mismatch" << std::endl;
                return false;
            }
        }
    }

    sf_close(original_file);
    sf_close(out_file);

    return true;
}

int main()
{
    g_ring_buffer.Resize(48000);

    input_file = sf_open(audio_file, SFM_READ, &input_info);
    std::cout << "Channels: " << input_info.channels << std::endl;
    std::cout << "Sample rate: " << input_info.samplerate << std::endl;
    std::cout << "Frames: " << input_info.frames << std::endl;
    std::cout << "Format:" << input_info.format << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    std::thread producer(producer_thread);
    std::thread consumer_thread(consumer);
    producer.join();
    consumer_thread.join();
    // easy_test();
    auto end = std::chrono::high_resolution_clock::now();

    bool result = compare_files();
    std::cout << "Test result: " << (result ? "PASSED" : "FAILED") << std::endl;

    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << "s" << std::endl;

    return 0;
}

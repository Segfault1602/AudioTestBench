#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include <sndfile.h>

#include "ring_buffer.h"

constexpr char audio_file[] = "C:\\source\\libdsp\\waves\\piano.wav";

RingBuffer g_ring_buffer;

SF_INFO input_info;
SNDFILE* input_file;

// PaUtilRingBuffer g_pa_ring_buffer;

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

        // ring_buffer_size_t write_available = PaUtil_GetRingBufferWriteAvailable(&g_pa_ring_buffer);
        // if (write_available < read)
        // {
        //     std::cout << "No space to write, dropping samples" << std::endl;
        //     continue;
        // }
        // else
        // {
        //     // PaUtil_WriteRingBuffer(&g_pa_ring_buffer, buffer.data(), read);
        // }

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
    const uint32_t period_seconds = 30; // 30 fps
    const uint32_t period_milliseconds = 1000 / 30;

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

int main()
{
    g_ring_buffer.Resize(32768);

    float* data = new float[1024];
    // PaUtil_InitializeRingBuffer(&g_pa_ring_buffer, sizeof(float), 1024, data);

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

    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << "s" << std::endl;

    return 0;
}

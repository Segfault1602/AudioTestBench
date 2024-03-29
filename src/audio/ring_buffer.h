#pragma once

#include <memory>
#include <atomic>

class RingBuffer
{
public:
    RingBuffer(size_t size = 32768);
    ~RingBuffer();

    void Resize(size_t size);

    size_t GetSize() const;
    size_t GetReadAvailable() const;
    size_t GetWriteAvailable() const;

    void Write(const float* data, size_t size);
    void Read(float* data, size_t& size);
    void Peek(float* data, size_t& size);

    void Reset();

private:
    size_t size_;
    std::atomic<size_t> read_index_;
    std::atomic<size_t> write_index_;
    std::unique_ptr<float[]> buffer_;

    size_t big_mask_;
    size_t small_mask_;
};
#pragma once

#include <vector>

class PlotRingBuffer
{
  public:
    PlotRingBuffer(size_t size = 32768);
    ~PlotRingBuffer();

    void Resize(size_t size);

    size_t GetSize() const;

    void Write(const float* data, size_t size);

    constexpr const float* GetBuffer() const
    {
        return buffer_.data();
    }

    size_t GetOffset() const;
    void Reset();

  private:
    size_t size_;
    int64_t write_index_;
    std::vector<float> buffer_;
};
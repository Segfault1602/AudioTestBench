#include "plotting_ringbuffer.h"

PlotRingBuffer::PlotRingBuffer(size_t size)
{
    Resize(size);
}

PlotRingBuffer::~PlotRingBuffer()
{
}

void PlotRingBuffer::Resize(size_t size)
{
    size_ = size;
    buffer_.resize(size_);
    write_index_ = 0;
}

size_t PlotRingBuffer::GetSize() const
{
    return size_;
}

size_t PlotRingBuffer::GetOffset() const
{
    return static_cast<size_t>(write_index_);
}

void PlotRingBuffer::Write(const float* data, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        buffer_[write_index_--] = data[i];
        if (write_index_ < 0)
        {
            write_index_ = size_ - 1;
        }
    }
}
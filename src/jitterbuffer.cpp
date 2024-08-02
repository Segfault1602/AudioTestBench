#include "jitterbuffer.h"

#include <cmath>

JitterBuffer::JitterBuffer(size_t size)
{
    Resize(size);
}

JitterBuffer::~JitterBuffer()
{
}

void JitterBuffer::Resize(size_t size)
{
    size_ = size;
    buffer_.resize(size_);
    write_index_ = 0;
}

size_t JitterBuffer::GetSize() const
{
    return size_;
}

size_t JitterBuffer::GetOffset() const
{
    return static_cast<size_t>(write_index_);
}

void JitterBuffer::Write(const float* data, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        buffer_[write_index_++] = data[i];
        write_index_ %= size_;
    }
}

void JitterBuffer::Peek(float* data, size_t size)
{
    size_t copy_size = std::min(size, size_);

    size_t offset = GetOffset();
    if (offset + copy_size < size_)
    {
        std::copy(buffer_.begin() + offset, buffer_.begin() + offset + copy_size, data);
    }
    else
    {
        size_t first_size = size_ - offset;
        std::copy(buffer_.begin() + offset, buffer_.begin() + offset + first_size, data);
        std::copy(buffer_.begin(), buffer_.begin() + copy_size - first_size, data + first_size);
    }

    if (copy_size < size)
    {
        std::fill(data + copy_size, data + size, 0.0f);
    }
}
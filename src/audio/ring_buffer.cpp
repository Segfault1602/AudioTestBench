#include "ring_buffer.h"

#include <cassert>
#include <iostream>

RingBuffer::RingBuffer(size_t size)
{
    Resize(size);
}

RingBuffer::~RingBuffer()
{
}

void RingBuffer::Resize(size_t size)
{
    if (((size-1) & size) != 0)
    {
        std::cerr << "RingBuffer::Resize: Size must be a power of 2" << std::endl;
        return;
    }

    size_ = size;
    buffer_ = std::make_unique<float[]>(size_);

    read_index_ = 0;
    write_index_ = 0;

    big_mask_ = (size*2)-1;
    small_mask_ = size - 1;
}

size_t RingBuffer::GetSize() const
{
    return size_;
}

size_t RingBuffer::GetReadAvailable() const
{
    return (write_index_ - read_index_) & big_mask_;
}

size_t RingBuffer::GetWriteAvailable() const
{
    return size_ - GetReadAvailable();
}

void RingBuffer::Write(const float* data, size_t size)
{
    size_t write_available = GetWriteAvailable();

    if (write_available == 0)
    {
        std::cerr << "RingBuffer::Write: No space to write, dropping samples" << std::endl;
        return;
    }

    if (size > write_available)
    {
        std::cerr << "RingBuffer::Write: No enough space to write, dropping samples" << std::endl;
        size = write_available;
    }

    // Check if we need to wrap around and write in two step
    size_t write_index = write_index_ & small_mask_;
    if (write_index + size > size_)
    {
        size_t first_chunk_size = size_ - write_index;
        std::copy(data, data + first_chunk_size, buffer_.get() + write_index);
        std::copy(data + first_chunk_size, data + size, buffer_.get());
    }
    else
    {
        std::copy(data, data + size, buffer_.get() + write_index);
    }

    write_index_.store((write_index_ + size) & big_mask_);
}

void RingBuffer::Read(float* data, size_t& size)
{
    size_t read_available = GetReadAvailable();

    if (read_available == 0)
    {
        // std::cerr << "RingBuffer::Read: No data to read" << std::endl;
        size = 0;
        return;
    }

    if (size > read_available)
    {
        std::cerr << "RingBuffer::Read: No enough data to read, reading only available data" << std::endl;
        size = read_available;
    }

    // Check if we need to wrap around and read in two step
    size_t read_index = read_index_ & small_mask_;
    if (read_index + size > size_)
    {
        size_t first_chunk_size = size_ - read_index;
        std::copy(buffer_.get() + read_index, buffer_.get() + read_index + first_chunk_size, data);
        std::copy(buffer_.get(), buffer_.get() + size - first_chunk_size, data + first_chunk_size);
    }
    else
    {
        std::copy(buffer_.get() + read_index, buffer_.get() + read_index + size, data);
    }

    read_index_.store((read_index_ + size) & big_mask_);
}

void RingBuffer::Peek(float* data, size_t& size)
{
    size_t read_available = GetReadAvailable();

    if (read_available == 0)
    {
        // std::cerr << "RingBuffer::Read: No data to read" << std::endl;
        size = 0;
        return;
    }

    if (size > read_available)
    {
        std::cerr << "RingBuffer::Read: No enough data to read, reading only available data" << std::endl;
        size = read_available;
    }

    // Check if we need to wrap around and read in two step
    size_t read_index = read_index_ & small_mask_;
    if (read_index + size > size_)
    {
        size_t first_chunk_size = size_ - read_index;
        std::copy(buffer_.get() + read_index, buffer_.get() + read_index + first_chunk_size, data);
        std::copy(buffer_.get(), buffer_.get() + size - first_chunk_size, data + first_chunk_size);
    }
    else
    {
        std::copy(buffer_.get() + read_index, buffer_.get() + read_index + size, data);
    }
}

void RingBuffer::Reset()
{
    read_index_ = 0;
    write_index_ = 0;
}
#include "buffer.h"
#include <fstream>
#include <iterator>
#include <stdexcept>

Buffer::Buffer(const std::string& path) : path_(path) 
{
    std::ifstream f(path, std::ios::binary);

    if (!f)
    {
        throw std::runtime_error("Cannot open file: " + path);
    }

    data_.assign(std::istreambuf_iterator<char>(f),
                 std::istreambuf_iterator<char>());
}

std::vector<uint8_t> Buffer::read(size_t offset, size_t len) const 
{
    if (offset >= data_.size())
    {
        return {};
    }

    size_t end = std::min(offset + len, data_.size());

    return {data_.begin() + offset, data_.begin() + end};
}

bool Buffer::write_byte(size_t offset, uint8_t value) 
{
    if (offset >= data_.size())
    {
        return false;
    }

    data_[offset] = value;
    dirty_ = true;

    return true;
}

bool Buffer::save() 
{
    std::ofstream f(path_, std::ios::binary | std::ios::trunc);

    if (!f)
    {
        return false;
    }

    f.write(reinterpret_cast<const char*>(data_.data()), data_.size());
    dirty_ = !f.good();

    return f.good();
}

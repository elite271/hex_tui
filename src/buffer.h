#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <optional>

class Buffer 
{
public:
    explicit Buffer(const std::string& path);

    std::vector<uint8_t> read(size_t offset, size_t len) const;

    bool write_byte(size_t offset, uint8_t value);

    size_t size() const { return data_.size(); }
    bool   dirty() const { return dirty_; }
    bool   save();

    const std::string& path() const { return path_; }

private:
    std::string path_;
    std::vector<uint8_t> data_;
    bool dirty_ = false;
};

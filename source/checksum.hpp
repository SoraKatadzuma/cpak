#pragma once
#include "cpakfile.hpp"

namespace cpak {


/// @brief   Structure for calculating a SHA-1 checksum.
/// @details Given a string, this structure will calculate a SHA-1 checksum
///          for that string. This checksum will be used to differentiate the
///          same project with different configurations.
struct Checksum {
    using digest_t = std::array<std::uint32_t,  5>;
    using chunk_t  = std::array<std::uint8_t,  64>;
    using block_t  = std::array<std::uint8_t,  20>;

public:
    /// @brief Creates a new checksum from the given data.
    /// @param data The data to create the checksum from.
    explicit Checksum(std::string_view data) {
        reset(*this);
        update(*this, data);
    }

    virtual ~Checksum() = default;

    /// @brief Resets the checksum to its initial state.
    /// @param checksum The checksum to reset.
    static void reset(Checksum& checksum) noexcept {
        constexpr std::uint32_t kDigest1Constant = 0x67452301;
        constexpr std::uint32_t kDigest2Constant = 0xEFCDAB89;
        constexpr std::uint32_t kDigest3Constant = 0x98BADCFE;
        constexpr std::uint32_t kDigest4Constant = 0x10325476;
        constexpr std::uint32_t kDigest5Constant = 0xC3D2E1F0;

        checksum.digest_ = {
            kDigest1Constant,
            kDigest2Constant,
            kDigest3Constant,
            kDigest4Constant,
            kDigest5Constant
        };

        checksum.blockOffset_ = 0;
        checksum.byteCount_   = 0;
        checksum.chunk_.fill(0);
    }

    /// @brief Updates the checksum with the given data.
    /// @param checksum The checksum to update.
    /// @param data The data to update the checksum with.
    static void update(Checksum& checksum, std::string_view data) noexcept {
        for (const auto& byte : data)
            update(checksum, byte);
    }

    /// @brief Finalizes the checksum and outputs the digest.
    /// @param checksum The checksum to finalize.
    /// @param result Where to store the data.
    static void finalize(Checksum& checksum, block_t& result) noexcept {
        digest_t    digest;
        std::size_t index = 0;
        finalize(checksum, digest);

        for (const auto& value : digest) {
            result[index++] = ((value >> 24) & 0xFF);
            result[index++] = ((value >> 16) & 0xFF);
            result[index++] = ((value >>  8) & 0xFF);
            result[index++] = ((value >>  0) & 0xFF);
        }
    }

private:
    static void finalize(Checksum& checksum, digest_t& result) noexcept {
        const auto& bitCount = checksum.byteCount_ * 8;
        update(checksum, 0x80);
        if (checksum.blockOffset_ > 56) {
            while (checksum.blockOffset_ != 0) update(checksum, 0);
            while (checksum.blockOffset_ < 56) update(checksum, 0);
        } else {
            while (checksum.blockOffset_ < 56) update(checksum, 0);
        }

        update(checksum, 0);
        update(checksum, 0);
        update(checksum, 0);
        update(checksum, 0);
        update(checksum, static_cast<std::uint8_t>((bitCount >> 24) & 0xFF));
        update(checksum, static_cast<std::uint8_t>((bitCount >> 16) & 0xFF));
        update(checksum, static_cast<std::uint8_t>((bitCount >>  8) & 0xFF));
        update(checksum, static_cast<std::uint8_t>((bitCount >>  0) & 0xFF));
        std::memcpy(result.data(), checksum.digest_.data(), 5 * sizeof(std::uint32_t));
    }

    static void update(Checksum& checksum, std::uint8_t byte) noexcept {
        constexpr std::uint32_t kChunkSize = 64;

        checksum.byteCount_++;
        checksum.chunk_[checksum.blockOffset_++] = byte;
        if (checksum.blockOffset_ != kChunkSize)
            return;
    
        checksum.blockOffset_ = 0;
        processChunk(checksum);
    }

    static std::uint32_t leftRotate(std::uint32_t value, std::uint32_t count) noexcept {
        return (value << count) | (value >> (32 - count));
    }

    static void processChunk(Checksum& checksum) noexcept {
        constexpr std::uint32_t kPrimeA = 0x5A827999;
        constexpr std::uint32_t kPrimeB = 0x6ED9EBA1;
        constexpr std::uint32_t kPrimeC = 0x8F1BBCDC;
        constexpr std::uint32_t kPrimeD = 0xCA62C1D6;

        std::array<std::uint32_t, 80> w;
        for (auto i = 0u; i < 16; ++i)
            w[i] = (checksum.chunk_[i * 4 + 0] << 24) |
                   (checksum.chunk_[i * 4 + 1] << 16) |
                   (checksum.chunk_[i * 4 + 2] <<  8) |
                   (checksum.chunk_[i * 4 + 3] <<  0);
        
        for (auto i = 16u; i < 80; ++i)
            w[i] = leftRotate(w[i -  3] ^
                              w[i -  8] ^
                              w[i - 14] ^
                              w[i - 16], 1);
        
        auto t = 0u;
        auto f = 0u;
        auto k = 0u;
        auto a = checksum.digest_[0];
        auto b = checksum.digest_[1];
        auto c = checksum.digest_[2];
        auto d = checksum.digest_[3];
        auto e = checksum.digest_[4];
        for (auto i = 0u; i < 80; ++i) {
            if (i < 20) {
                f = (b & c) | (~b & d);
                k = kPrimeA;
            } else if (i < 40) {
                f = b ^ c ^ d;
                k = kPrimeB;
            } else if (i < 60) {
                f = (b & c) | (b & d) | (c & d);
                k = kPrimeC;
            } else {
                f = b ^ c ^ d;
                k = kPrimeD;
            }

            t = leftRotate(a, 5) + f + e + k + w[i];
            e = d;
            d = c;
            c = leftRotate(b, 30);
            b = a;
            a = t;
        }

        checksum.digest_[0] += a;
        checksum.digest_[1] += b;
        checksum.digest_[2] += c;
        checksum.digest_[3] += d;
        checksum.digest_[4] += e;
    }

private:
    chunk_t     chunk_;
    digest_t    digest_;
    std::size_t blockOffset_;
    std::size_t byteCount_;
};


/// @brief  Generates a checksum for the given CPakFile.
/// @param  cpakfile The CPakFile to generate the checksum for.
/// @return The generated checksum.
std::string checksum(const CPakFile& cpakfile) noexcept {
    std::ostringstream oss;
    Checksum::block_t  block;

    // TODO: support options.
    oss << cpakfile.project.name;
    Checksum checksum(oss.str());
    Checksum::finalize(checksum, block);

    oss.str("");
    oss.clear();
    for (const auto& byte : block)
        oss << std::hex << std::setfill('0')
            << std::setw(2) << static_cast<int>(byte);

    return oss.str();
}


}
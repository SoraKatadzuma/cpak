#include <sstream>
#include "utilities/checksum.hpp"
#include "gtest/gtest.h"

TEST(ChecksumTests, canReplicateChecksum) {
    cpak::utilities::Checksum actual("cpaktest");
    cpak::utilities::Checksum::block_t block;
    cpak::utilities::Checksum::finalize(actual, block);

    std::ostringstream oss;
    for (const auto& byte : block)
        oss << std::hex << std::setfill('0') << std::setw(2)
            << static_cast<int>(byte);

    ASSERT_EQ("0616799c9f15a2672f71be3772e6a7d2a06289d1", oss.str());
}
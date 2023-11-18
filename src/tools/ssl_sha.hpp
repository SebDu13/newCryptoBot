#pragma once

#include <array>
#include <climits>
#include <string_view>

#include "cct_string.hpp"

namespace cct {
namespace ssl {

//------------------------------------------------------------------------------
// helper function to compute SHA256:
using Sha256 = std::array<char, 256 / CHAR_BIT>;

Sha256 ComputeSha256(std::string_view data);

enum class ShaType { kSha256, kSha512 };

string ShaBin(ShaType shaType, std::string_view data, std::string_view secret);


}  // namespace ssl
}  // namespace cct
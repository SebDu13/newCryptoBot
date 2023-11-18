#pragma once

#include <string_view>

#include "cct_string.hpp"

namespace cct {

string B64Encode(std::string_view bindata);

string B64Decode(std::string_view ascdata);
}  // namespace cct
﻿#include "ssl_sha.hpp"

#include <openssl/hmac.h>
#include <openssl/sha.h>

#include <cassert>
#include <cstring>
#include <memory>

#include "cct_codec.hpp"

namespace cct {
namespace ssl {
namespace {
using HMACCtxUniquePtr = std::unique_ptr<HMAC_CTX, decltype([](HMAC_CTX* ptr) { HMAC_CTX_free(ptr); })>;

unsigned int ShaDigestLen(ShaType shaType) { return (shaType == ShaType::kSha256 ? 256 : 512) / CHAR_BIT; }
}  // namespace

//------------------------------------------------------------------------------
// helper function to compute SHA256:
Sha256 ComputeSha256(std::string_view data) {
  Sha256 ret;

  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  SHA256_Update(&ctx, data.data(), data.length());
  SHA256_Final(reinterpret_cast<unsigned char*>(ret.data()), &ctx);

  return ret;
}

string ShaBin(ShaType shaType, std::string_view data, std::string_view secret) {
  HMACCtxUniquePtr ctx(HMAC_CTX_new());

  HMAC_Init_ex(ctx.get(), secret.data(), static_cast<int>(secret.size()),
               shaType == ShaType::kSha256 ? EVP_sha256() : EVP_sha512(), nullptr);
  HMAC_Update(ctx.get(), reinterpret_cast<const unsigned char*>(data.data()), data.size());

  unsigned int len = ShaDigestLen(shaType);
  string binData(len, 0);
  HMAC_Final(ctx.get(), reinterpret_cast<unsigned char*>(binData.data()), &len);
  assert(len == binData.size());
  return binData;
}

}  // namespace ssl
}  // namespace cct

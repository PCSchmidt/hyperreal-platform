#include <doctest/doctest.h>

#include <string>

#include "hr/sha256.hpp"

TEST_CASE("SHA-256 FIPS 180-4 test vectors") {
  CHECK(hr::Sha256::hash_hex("") == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
  CHECK(hr::Sha256::hash_hex("abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
  CHECK(hr::Sha256::hash_hex("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") ==
        "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
}

TEST_CASE("SHA-256 of one million a characters") {
  hr::Sha256 h;
  const std::string chunk(1000, 'a');
  for (int i = 0; i < 1000; ++i) h.update(chunk);
  CHECK(hr::Sha256::hex(h.finish()) == "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
}

TEST_CASE("Incremental updates equal one-shot hashing at all split points") {
  const std::string msg(200, 'x');
  const auto whole = hr::Sha256::hash_hex(msg);
  for (std::size_t split = 0; split <= msg.size(); ++split) {
    hr::Sha256 h;
    h.update(msg.data(), split);
    h.update(msg.data() + split, msg.size() - split);
    CHECK(hr::Sha256::hex(h.finish()) == whole);
  }
}

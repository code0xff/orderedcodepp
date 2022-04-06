#include <libs/Catch2/src/catch2/catch_all.hpp>
#include <map>
#include <orderedcode.h>
#include <utils/str_const.h>
#include <vector>

using namespace std;
using namespace orderedcode;

TEST_CASE("orderedcode: append uint64", "[noir][codec]") {
  map<uint64_t, bytes> testcase;
  testcase.emplace(make_pair<uint64_t, bytes>(uint64_t(0), {0x00}));
  testcase.emplace(make_pair<uint64_t, bytes>(uint64_t(1), {0x01, 0x01}));
  testcase.emplace(make_pair<uint64_t, bytes>(uint64_t(255), {0x01, 0xff}));
  testcase.emplace(make_pair<uint64_t, bytes>(uint64_t(256), {0x02, 0x01, 0x00}));
  testcase.emplace(make_pair<uint64_t, bytes>(uint64_t(1025), {0x02, 0x04, 0x01}));
  testcase.emplace(make_pair<uint64_t, bytes>(uint64_t(0x0a0b0c0d), {0x04, 0x0a, 0x0b, 0x0c, 0x0d}));
  testcase.emplace(
    make_pair<uint64_t, bytes>(uint64_t(0x0102030405060708), {0x08, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}));
  testcase.emplace(make_pair<uint64_t, bytes>(
    numeric_limits<uint64_t>::max(), {0x08, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}));

  bytes b;
  for (auto& t : testcase) {
    orderedcode::append(b, t.first);
    CHECK(b == t.second);
    b.clear();
  }

  orderedcode::uint64_t i;
  for (auto& t : testcase) {
    i = 0;
    span<byte_t> s(t.second);
    orderedcode::parse(s, i);
    CHECK(i == t.first);
  }

  bytes b2;
  for (auto& t : testcase) {
    orderedcode::append(b, t.first);
    b2.insert(b2.end(), t.second.begin(), t.second.end());
  }
  CHECK(b == b2);
}

TEST_CASE("orderedcode: append string", "[noir][codec]") {
  map<string, bytes> testcase;
  testcase.emplace(make_pair<string, bytes>(str_const(""), {0x00, 0x01}));
  testcase.emplace(make_pair<string, bytes>(str_const("\x00"), {0x00, 0xff, 0x00, 0x01}));
  testcase.emplace(make_pair<string, bytes>(str_const("\x00\x00"), {0x00, 0xff, 0x00, 0xff, 0x00, 0x01}));
  testcase.emplace(make_pair<string, bytes>(str_const("\x01"), {0x01, 0x00, 0x01}));
  testcase.emplace(make_pair<string, bytes>(str_const("foo"), {'f', 'o', 'o', 0x00, 0x01}));
  testcase.emplace(make_pair<string, bytes>(str_const("foo\x00"), {'f', 'o', 'o', 0x00, 0xff, 0x00, 0x01}));
  testcase.emplace(make_pair<string, bytes>(str_const("foo\x00\x01"), {'f', 'o', 'o', 0x00, 0xff, 0x01, 0x00, 0x01}));
  testcase.emplace(make_pair<string, bytes>(str_const("foo\x01"), {'f', 'o', 'o', 0x01, 0x00, 0x01}));
  testcase.emplace(make_pair<string, bytes>(str_const("foo\x01\x00"), {'f', 'o', 'o', 0x01, 0x00, 0xff, 0x00, 0x01}));
  testcase.emplace(make_pair<string, bytes>(str_const("foo\xfe"), {'f', 'o', 'o', 0xfe, 0x00, 0x01}));
  testcase.emplace(make_pair<string, bytes>(str_const("foo\xff"), {'f', 'o', 'o', 0xff, 0x00, 0x00, 0x01}));
  testcase.emplace(make_pair<string, bytes>(str_const("\xff"), {0xff, 0x00, 0x00, 0x01}));
  testcase.emplace(make_pair<string, bytes>(str_const("\xff\xff"), {0xff, 0x00, 0xff, 0x00, 0x00, 0x01}));

  bytes b;

  for (auto& t : testcase) {
    orderedcode::append(b, t.first);
    CHECK(b == t.second);
    b.clear();
  }

  orderedcode::string s;
  for (auto& t : testcase) {
    span<byte_t> sp(t.second);
    orderedcode::parse(sp, s);
    CHECK(s == t.first);
  }

  bytes b2;
  for (auto& t : testcase) {
    orderedcode::append(b, t.first);
    b2.insert(b2.end(), t.second.begin(), t.second.end());
  }
  CHECK(b == b2);
}

TEST_CASE("orderedcode: append infinity", "[noir][codec]") {
  infinity inf1;
  bytes b;
  append(b, inf1);

  bytes c = {0xff, 0xff};
  CHECK(b == c);

  bytes d = {0xff, 0xff, 0xff, 0xff};
  infinity inf2;
  span<byte_t> s(d);
  parse(s, inf1, inf2);
  CHECK(inf1 == infinity{});
  CHECK(inf2 == infinity{});
}

TEST_CASE("orderedcode: append float64", "[noir][codec]") {
  map<orderedcode::float64_t, bytes> testcase;

  testcase.emplace(make_pair<orderedcode::float64_t, bytes>(-std::numeric_limits<orderedcode::float64_t>::infinity(),
                                                            {0x00, 0x3f, 0x80, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}));
  testcase.emplace(make_pair<orderedcode::float64_t, bytes>(
    -std::numeric_limits<orderedcode::float64_t>::max(), {0x00, 0x3f, 0x80, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}));
  testcase.emplace(
    make_pair<orderedcode::float64_t, bytes>(-2.71828, {0x00, 0x3f, 0xbf, 0xfa, 0x40, 0xf6, 0x6a, 0x55, 0x08, 0x70}));
  testcase.emplace(
    make_pair<orderedcode::float64_t, bytes>(-1.0, {0x00, 0x40, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}));
  testcase.emplace(make_pair<orderedcode::float64_t, bytes>(0.0, {0x80}));
  testcase.emplace(
    make_pair<orderedcode::float64_t, bytes>(0.333333333, {0xff, 0xbf, 0xd5, 0x55, 0x55, 0x54, 0xf9, 0xb5, 0x16}));
  testcase.emplace(
    make_pair<orderedcode::float64_t, bytes>(1.0, {0xff, 0xbf, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}));
  testcase.emplace(
    make_pair<orderedcode::float64_t, bytes>(1.41421, {0xff, 0xbf, 0xf6, 0xa0, 0x9a, 0xaa, 0x3a, 0xd1, 0x8d}));
  testcase.emplace(
    make_pair<orderedcode::float64_t, bytes>(1.5, {0xff, 0xbf, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}));
  testcase.emplace(
    make_pair<orderedcode::float64_t, bytes>(2.0, {0xff, 0xc0, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}));
  testcase.emplace(
    make_pair<orderedcode::float64_t, bytes>(3.14159, {0xff, 0xc0, 0x40, 0x09, 0x21, 0xf9, 0xf0, 0x1b, 0x86, 0x6e}));
  testcase.emplace(
    make_pair<orderedcode::float64_t, bytes>(6.022e23, {0xff, 0xc0, 0x44, 0xdf, 0xe1, 0x54, 0xf4, 0x57, 0xea, 0x13}));
  testcase.emplace(make_pair<orderedcode::float64_t, bytes>(
    std::numeric_limits<orderedcode::float64_t>::max(), {0xff, 0xc0, 0x7f, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}));
  testcase.emplace(make_pair<orderedcode::float64_t, bytes>(std::numeric_limits<orderedcode::float64_t>::infinity(),
                                                            {0xff, 0xc0, 0x7f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}));

  bytes b;
  for (auto& t : testcase) {
    orderedcode::append(b, t.first);
    CHECK(b == t.second);
    b.clear();
  }

  orderedcode::float64_t f;
  for (auto& t : testcase) {
    span<byte_t> s(t.second);
    orderedcode::parse(s, f);
    CHECK(f == t.first);
    b.clear();
  }

  bytes b2;
  for (auto& t : testcase) {
    orderedcode::append(b, t.first);
    b2.insert(b2.end(), t.second.begin(), t.second.end());
  }
  CHECK(b == b2);

  span<byte_t> s2(b2);
  for (auto& t : testcase) {
    orderedcode::parse(s2, f);
    CHECK(f == t.first);
  }
  b.clear();

  uint64_t i = 0x8000000000000000;
  orderedcode::float64_t f2 = 0;
  memcpy(&f, &i, sizeof(i));
  orderedcode::append(b, f2);

  bytes b3 = {0x80};
  CHECK(b == b3);
  b.clear();

  orderedcode::float64_t f3 = -4.940656458412465441765687928682213723651e-324;
  orderedcode::append(b, f3);
  bytes b4 = {0x7f};
  CHECK(b == b4);
  b.clear();

  orderedcode::float64_t f4 = 4.940656458412465441765687928682213723651e-324;
  orderedcode::append(b, f4);
  bytes b5 = {0x81};
  CHECK(b == b5);
}

TEST_CASE("orderedcode: append int64", "[noir][codec]") {
  map<int64_t, bytes> testcase;

  testcase.emplace(make_pair<int64_t, bytes>(-8193, {0x1f, 0xdf, 0xff}));
  testcase.emplace(make_pair<int64_t, bytes>(-8192, {0x20, 0x00}));
  testcase.emplace(make_pair<int64_t, bytes>(-4097, {0x2f, 0xff}));
  testcase.emplace(make_pair<int64_t, bytes>(-257, {0x3e, 0xff}));
  testcase.emplace(make_pair<int64_t, bytes>(-256, {0x3f, 0x00}));
  testcase.emplace(make_pair<int64_t, bytes>(-66, {0x3f, 0xbe}));
  testcase.emplace(make_pair<int64_t, bytes>(-65, {0x3f, 0xbf}));
  testcase.emplace(make_pair<int64_t, bytes>(-64, {0x40}));
  testcase.emplace(make_pair<int64_t, bytes>(-63, {0x41}));
  testcase.emplace(make_pair<int64_t, bytes>(-3, {0x7d}));
  testcase.emplace(make_pair<int64_t, bytes>(-2, {0x7e}));
  testcase.emplace(make_pair<int64_t, bytes>(-1, {0x7f}));
  testcase.emplace(make_pair<int64_t, bytes>(0, {0x80}));
  testcase.emplace(make_pair<int64_t, bytes>(1, {0x81}));
  testcase.emplace(make_pair<int64_t, bytes>(2, {0x82}));
  testcase.emplace(make_pair<int64_t, bytes>(62, {0xbe}));
  testcase.emplace(make_pair<int64_t, bytes>(63, {0xbf}));
  testcase.emplace(make_pair<int64_t, bytes>(64, {0xc0, 0x40}));
  testcase.emplace(make_pair<int64_t, bytes>(65, {0xc0, 0x41}));
  testcase.emplace(make_pair<int64_t, bytes>(255, {0xc0, 0xff}));
  testcase.emplace(make_pair<int64_t, bytes>(256, {0xc1, 0x00}));
  testcase.emplace(make_pair<int64_t, bytes>(4096, {0xd0, 0x00}));
  testcase.emplace(make_pair<int64_t, bytes>(8191, {0xdf, 0xff}));
  testcase.emplace(make_pair<int64_t, bytes>(8192, {0xe0, 0x20, 0x00}));

  testcase.emplace(make_pair<int64_t, bytes>(-0x800, {0x38, 0x00}));
  testcase.emplace(make_pair<int64_t, bytes>(0x424242, {0xf0, 0x42, 0x42, 0x42}));
  testcase.emplace(make_pair<int64_t, bytes>(0x23, {0xa3}));
  testcase.emplace(make_pair<int64_t, bytes>(0x10e, {0xc1, 0x0e}));
  testcase.emplace(make_pair<int64_t, bytes>(-0x10f, {0x3e, 0xf1}));
  testcase.emplace(make_pair<int64_t, bytes>(0x020b0c0d, {0xf2, 0x0b, 0x0c, 0x0d}));
  testcase.emplace(make_pair<int64_t, bytes>(0x0a0b0c0d, {0xf8, 0x0a, 0x0b, 0x0c, 0x0d}));
  testcase.emplace(
    make_pair<int64_t, bytes>(0x0102030405060708, {0xff, 0x81, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}));

  testcase.emplace(
    make_pair<int64_t, bytes>(((int64_t)-1 << 63) - 0, {0x00, 0x3f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}));
  testcase.emplace(
    make_pair<int64_t, bytes>(((int64_t)-1 << 62) - 1, {0x00, 0x3f, 0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}));
  testcase.emplace(
    make_pair<int64_t, bytes>(((int64_t)-1 << 62) - 0, {0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}));
  testcase.emplace(
    make_pair<int64_t, bytes>(((int64_t)-1 << 55) - 1, {0x00, 0x7f, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}));
  testcase.emplace(
    make_pair<int64_t, bytes>(((int64_t)-1 << 55) - 0, {0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}));
  testcase.emplace(
    make_pair<int64_t, bytes>(((int64_t)-1 << 48) - 1, {0x00, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)-1 << 48) - 0, {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)-1 << 41) - 1, {0x01, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)-1 << 41) - 0, {0x02, 0x00, 0x00, 0x00, 0x00, 0x00}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)-1 << 34) - 1, {0x03, 0xfb, 0xff, 0xff, 0xff, 0xff}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)-1 << 34) - 0, {0x04, 0x00, 0x00, 0x00, 0x00}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)-1 << 27) - 1, {0x07, 0xf7, 0xff, 0xff, 0xff}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)-1 << 27) - 0, {0x08, 0x00, 0x00, 0x00}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)-1 << 20) - 1, {0x0f, 0xef, 0xff, 0xff}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)-1 << 20) - 0, {0x10, 0x00, 0x00}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)-1 << 13) - 1, {0x1f, 0xdf, 0xff}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)-1 << 13) - 0, {0x20, 0x00}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)-1 << 6) - 1, {0x3f, 0xbf}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)-1 << 6) - 0, {0x40}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)1 << 6) - 1, {0xbf}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)1 << 6) - 0, {0xc0, 0x40}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)1 << 13) - 1, {0xdf, 0xff}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)1 << 13) - 0, {0xe0, 0x20, 0x00}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)1 << 20) - 1, {0xef, 0xff, 0xff}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)1 << 20) - 0, {0xf0, 0x10, 0x00, 0x00}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)1 << 27) - 1, {0xf7, 0xff, 0xff, 0xff}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)1 << 27) - 0, {0xf8, 0x08, 0x00, 0x00, 0x00}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)1 << 34) - 1, {0xfb, 0xff, 0xff, 0xff, 0xff}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)1 << 34) - 0, {0xfc, 0x04, 0x00, 0x00, 0x00, 0x00}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)1 << 41) - 1, {0xfd, 0xff, 0xff, 0xff, 0xff, 0xff}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)1 << 41) - 0, {0xfe, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)1 << 48) - 1, {0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)1 << 48) - 0, {0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}));
  testcase.emplace(make_pair<int64_t, bytes>(((int64_t)1 << 55) - 1, {0xff, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}));
  testcase.emplace(
    make_pair<int64_t, bytes>(((int64_t)1 << 55) - 0, {0xff, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}));
  testcase.emplace(
    make_pair<int64_t, bytes>(((int64_t)1 << 62) - 1, {0xff, 0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}));
  testcase.emplace(
    make_pair<int64_t, bytes>(((int64_t)1 << 62) - 0, {0xff, 0xc0, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}));
  testcase.emplace(
    make_pair<int64_t, bytes>(((int64_t)1 << 63) - 1, {0xff, 0xc0, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}));

  bytes b;

  for (auto& t : testcase) {
    orderedcode::append(b, t.first);
    CHECK(b == t.second);
    b.clear();
  }

  int64_t i;
  for (auto& t : testcase) {
    span<byte_t> s(t.second);
    orderedcode::parse(s, i);
    CHECK(i == t.first);
    b.clear();
  }
}

TEST_CASE("orderedcode: increase decrease", "[noir][codec]") {
  bytes b;
  orderedcode::append(
    b, uint64_t(0), decr<uint64_t>{1}, uint64_t(2), decr<uint64_t>{516}, uint64_t(517), decr<uint64_t>{0});

  bytes c = {0x00, 0xfe, 0xfe, 0x01, 0x02, 0xfd, 0xfd, 0xfb, 0x02, 0x02, 0x05, 0xff};
  CHECK(b == c);
}

TEST_CASE("orderedcode: round trip", "[noir][codec]") {
  bytes b;
  append(b, string("foo"), decr<string>{"bar"});

  string s1;
  decr<string> s2;

  span<byte_t> sp(b);
  parse(sp, s1, s2);
  CHECK(s1 == "foo");
  CHECK(s2 == decr<string>{"bar"});
}

TEST_CASE("orderedcode: trailing string", "[noir][codec]") {
  vector<string> testcase = {str_const(""), str_const("\x00"), str_const("\x00\x01"), str_const("a"), str_const("bcd"),
                             str_const("foo\x00"), str_const("foo\x00bar"), str_const("foo\x00bar\x00"), str_const("\xff"),
                             str_const("\xff\x00"), str_const("\xff\xfe"), str_const("\xff\xff")};

  bytes b;
  bytes b2;
  for (auto& t : testcase) {
    orderedcode::append(b, trailing_string{t});
    b2.insert(b2.end(), t.begin(), t.end());

    CHECK(b == b2);
    b.clear();
    b2.clear();
  }

  bytes b3;
  bytes b4;
  for (auto& t : testcase) {
    orderedcode::append(b3, decr<trailing_string>{t});
    b4.insert(b4.end(), t.begin(), t.end());

    span<byte_t> s(b4);
    invert(s);
    CHECK(b3 == b4);
    b3.clear();
    b4.clear();
  }
}

TEST_CASE("orderedcode: corrupt string or infinity", "[noir][codec]") {
  string_or_infinity dst0, dst1, dst2;

  bytes b = {0x00};
  span<byte_t> s(b);
  CHECK_THROWS(parse(s, dst0));

  bytes b2 = {'f', 'o', 'o', 0x00, 0x01, 0xff, 0xff, 0x00};
  span<byte_t> s2(b2);
  CHECK_THROWS(parse(s2, dst0, dst1, dst2));
}

TEST_CASE("orderedcode: each and pack", "[noir][codec]") {
  uint64_t i = 1;
  int64_t i2 = 1024;
  float64_t f = 0.333333333;
  string s = "foo";
  decr<string> ds{string("bar")};

  bytes b;
  orderedcode::append(b, i);
  orderedcode::append(b, i2);
  orderedcode::append(b, f);
  orderedcode::append(b, s);
  orderedcode::append(b, ds);
  orderedcode::append(b, infinity{});

  bytes b2;
  orderedcode::append(b2, i, i2, f, s, ds, infinity{});

  CHECK(b == b2);

  uint64_t ei;
  int64_t ei2;
  float64_t ef;
  string es;
  decr<string> eds;
  infinity einf;

  span<byte_t> sp(b);
  parse(sp, ei);
  parse(sp, ei2);
  parse(sp, ef);
  parse(sp, es);
  parse(sp, eds);
  parse(sp, einf);

  uint64_t pi;
  int64_t pi2;
  float64_t pf;
  string ps;
  decr<string> pds;
  infinity pinf;

  span<byte_t> sp2(b2);
  parse(sp2, pi, pi2, pf, ps, pds, pinf);

  CHECK(ei == pi);
  CHECK(ei2 == pi2);
  CHECK(ef == pf);
  CHECK(eds == pds);
  CHECK(einf == pinf);
}

TEST_CASE("orderedcode: NaN", "[noir][codec]") {
  uint64_t i = 0x7FF8000000000001;
  float64_t f;
  memcpy(&f, &i, sizeof(i));
  bytes b;

  CHECK_THROWS(append(b, f));

  span<byte_t> s(b);
  float64_t f2;
  CHECK_THROWS(parse(s, f2));

  CHECK(isnan(f));
}

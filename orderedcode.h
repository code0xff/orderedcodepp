// Copyright 2022 Jungyong Um
//
//   Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#pragma once
#include <cmath>
#include <iostream>
#include <span>
#include <string>
#include <vector>

namespace orderedcode {

using namespace std;

using byte_t = unsigned char;
using bytes = std::vector<byte_t>;
using float64_t = double_t;

const unsigned char term[] = {0x00, 0x01};
const unsigned char lit00[] = {0x00, 0xff};
const unsigned char litff[] = {0xff, 0x00};
const unsigned char inf[] = {0xff, 0xff};
const unsigned char msb[] = {0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe};

const byte_t increasing = 0x00;
const byte_t decreasing = 0xff;

struct infinity {
  bool operator==(const infinity& i) const {
    return true;
  }

  bool operator==(infinity&& i) {
    return true;
  }
};

template<typename T>
struct decr {
  T val;

  bool operator==(const decr<T>& o) const {
    return val == o.val;
  }

  bool operator==(decr<T> o) {
    return val == o.val;
  }
};

struct string_or_infinity {
  string s;
  bool inf;
};

struct trailing_string : string {};

void invert(span<byte_t>& s) {
  std::for_each(s.begin(), s.end(), [](byte_t& c) { c ^= 0xff; });
}

void append(bytes& s, uint64_t x) {
  vector<byte_t> buf(9);
  auto i = 8;
  for (; x > 0; x >>= 8) {
    buf[i--] = static_cast<byte_t>(x);
  }
  buf[i] = static_cast<byte_t>(8 - i);
  s.insert(s.end(), buf.begin() + i, buf.end());
}

void append(bytes& s, int64_t x) {
  if (x >= -64 && x < 64) {
    s.insert(s.end(), static_cast<byte_t>(x ^ 0x80));
    return;
  }
  bool neg = x < 0;
  if (neg) {
    x = ~x;
  }
  auto n = 1;
  bytes buf(10);
  auto i = 9;
  for (; x > 0; x >>= 8) {
    buf[i--] = static_cast<byte_t>(x);
    n++;
  }
  bool lfb = n > 7;
  if (lfb) {
    n -= 7;
  }
  if (buf[i + 1] < 1 << (8 - n)) {
    n--;
    i++;
  }
  buf[i] |= msb[n];
  if (lfb) {
    buf[--i] = 0xff;
  }
  if (neg) {
    span<byte_t> sp(&buf[i], buf.size() - i);
    invert(sp);
  }
  s.insert(s.end(), buf.begin() + i, buf.end());
}

void append(bytes& s, float64_t x) {
  if (isnan(x)) {
    throw runtime_error("append: NaN");
  }
  uint64_t b;
  memcpy(&b, &x, sizeof(x));
  auto i = int64_t(b);
  if (i < 0) {
    i = std::numeric_limits<int64_t>::min() - i;
  }
  append(s, i);
}

void append(bytes& s, const std::string& x) {
  auto last = x.begin();
  for (auto i = x.begin(); i < x.end(); i++) {
    switch (byte_t(*i)) {
      case 0x00:
        s.insert(s.end(), last, i);
        s.insert(s.end(), &lit00[0], &lit00[0] + 2);
        last = i + 1;
        break;
      case 0xff:
        s.insert(s.end(), last, i);
        s.insert(s.end(), &litff[0], &litff[0] + 2);
        last = i + 1;
    }
  }
  s.insert(s.end(), last, x.end());
  s.insert(s.end(), &term[0], &term[0] + 2);
}

void append(bytes& s, const trailing_string& x) {
  s.insert(s.end(), x.begin(), x.end());
}

void append(bytes& s) {
  s.insert(s.end(), &inf[0], &inf[0] + 2);
}

void append(bytes& s, infinity& _) {
  append(s);
}

void append(bytes& s, const string_or_infinity& x) {
  if (x.inf) {
    if (x.s.empty()) {
      throw runtime_error("orderedcode: string_or_infinity has non-zero string and non-zero infinity");
    }
    append(s);
  } else {
    append(s, x.s);
  }
}

template<typename T>
void append(bytes& buf, decr<T> d) {
  size_t n = buf.size();
  if (is_same_v<T, infinity>) {
    append(buf);
  } else {
    append(buf, d.val);
  }
  span<byte_t> s(&buf[n], buf.size() - n);
  invert(s);
}

template<typename It, typename... Its>
void append(bytes& buf, It it, Its... its) {
  append(buf, it);
  append(buf, its...);
}

void parse(span<byte_t>& s, byte_t dir, int64_t& dst) {
  if (s.empty()) {
    throw runtime_error("orderedcode: corrupt input");
  }
  byte_t c = s[0] ^ dir;
  if (c >= 0x40 && c < 0xc0) {
    dst = int64_t(int8_t(c ^ 0x80));
    s = s.subspan(1);
    return;
  }
  bool neg = (c & 0x80) == 0;
  if (neg) {
    c = ~c;
    dir = ~dir;
  }
  size_t n = 0;
  if (c == 0xff) {
    if (s.size() == 1) {
      throw runtime_error("orderedcode: corrupt input");
    }
    s = s.subspan(1);
    c = s[0] ^ dir;
    if (c > 0xc0) {
      throw runtime_error("orderedcode: corrupt input");
    }
    n = 7;
  }
  for (byte_t mask = 0x80; (c & mask) != 0; mask >>= 1) {
    c &= ~mask;
    n++;
  }
  if (s.size() < n) {
    throw runtime_error("orderedcode: corrupt input");
  }
  int64_t x = c;
  for (size_t i = 1; i < n; i++) {
    c = s[i] ^ dir;
    x = x << 8 | c;
  }
  if (neg) {
    x = ~x;
  }
  dst = x;
  s = s.subspan(n);
}

void parse(span<byte_t>& s, byte_t dir, uint64_t& dst) {
  if (s.empty()) {
    throw runtime_error("orderedcode: corrupt input");
  }
  byte_t n = s[0] ^ dir;
  if (n > 8 || s.size() < 1 + n) {
    throw runtime_error("orderedcode: corrupt input");
  }
  uint64_t x = 0;
  for (size_t i = 0; i < n; i++) {
    x = x << 8 | (s[1 + i] ^ dir);
  }
  dst = x;
  s = s.subspan(1 + n);
}

void parse(span<byte_t>& s, byte_t dir) {
  if (s.size() < 2) {
    throw runtime_error("orderedcode: corrupt input");
  }
  if ((s[0] ^ dir) != inf[0] || (s[1] ^ dir) != inf[1]) {
    throw runtime_error("orderedcode: corrupt input");
  }
  s = s.subspan(2);
}

void parse(span<byte_t>& s, byte_t dir, string& dst) {
  bytes buf;
  for (auto l = 0, i = 0; i < s.size();) {
    switch (s[i] ^ dir) {
      case 0x00:
        if (i + 1 >= s.size()) {
          throw runtime_error("orderedcode: corrupt input");
        }
        switch (s[i + 1] ^ dir) {
          case 0x01:
            dst.clear();
            if (l == 0 && dir == increasing) {
              dst.insert(dst.end(), s.begin(), s.begin() + i);
              s = s.subspan(i + 2);
              return;
            }
            buf.insert(buf.end(), s.begin() + l, s.begin() + i);
            if (dir == decreasing) {
              span<byte_t> s2(buf);
              invert(s2);
            }
            dst.insert(dst.end(), buf.begin(), buf.end());
            s = s.subspan(i + 2);
            return;
          case 0xff:
            buf.insert(buf.end(), s.begin() + l, s.begin() + i);
            buf.insert(buf.end(), static_cast<byte_t>(0x00 ^ dir));
            i += 2;
            l = i;
            break;
          default:
            throw runtime_error("orderedcode: corrupt input");
        }
        break;
      case 0xff:
        if (i + 1 >= s.size() || ((s[i + 1] ^ dir) != 0x00)) {
          throw runtime_error("orderedcode: corrupt input");
        }
        buf.insert(buf.end(), s.begin() + l, s.begin() + i);
        buf.insert(buf.end(), static_cast<byte_t>(0xff ^ dir));
        i += 2;
        l = i;
        break;
      default:
        i++;
    }
  }
  throw runtime_error("orderedcode: corrupt input4");
}

void parse(span<byte_t>& s, byte_t dir, float64_t& dst) {
  int64_t i = 0;
  parse(s, dir, i);
  if (i < 0) {
    i = ((int64_t)-1 << 63) - i;
  }
  memcpy(&dst, &i, sizeof(i));
  if (isnan(dst)) {
    throw runtime_error("parse: NaN");
  }
}

void parse(span<byte_t>& s, size_t& pos, byte_t dir, string_or_infinity& dst) {
  try {
    parse(s, dir);
    dst.inf = true;
    return;
  } catch (...) {
    parse(s, dir, dst.s);
  }
}

void parse(span<byte_t>& s, byte_t dir, trailing_string& dst) {
  if (dir == increasing) {
    dst.insert(dst.end(), s.begin(), s.end());
  } else {
    invert(s);
    dst.insert(dst.end(), s.begin(), s.end());
  }
}

template<typename T>
void parse(span<byte_t>& s, decr<T>& dst) {
  if (is_same_v<T, infinity>) {
    parse(s, decreasing);
  } else {
    parse(s, decreasing, dst.val);
  }
}

template<typename It>
void parse(span<byte_t>& s, It& it) {
  if (is_same_v<It, infinity>) {
    parse(s, increasing);
  } else {
    parse(s, increasing, it);
  }
}

template<typename It, typename... Its>
void parse(span<byte_t>& s, It& it, Its&... its) {
  parse(s, it);
  parse(s, its...);
}

}// namespace orderedcode

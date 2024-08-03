#include <chrono>

using namespace std;

class RNG {
  uint64_t res0 = 0, res1 = 0;
  uint32_t nBitsInRes = 0;
public:
  virtual uint64_t next64() = 0;
  virtual uint32_t next32() { return (uint32_t)next(32); }

  uint64_t next(uint32_t bits) {
    if (bits == 64) return next64();
    if (bits > nBitsInRes) {
      uint64_t u = next64();
      res0 |= u << nBitsInRes;
      res1 = nBitsInRes == 0 ? 0 : (u >> (64 - nBitsInRes));
      nBitsInRes += 64;
    }
    uint64_t ret = res0 & ((uint64_t(1) << bits) - 1);
    res0 >>= bits;
    res0 |= res1 << (64 - bits);
    res1 >>= bits;
    nBitsInRes -= bits;
    return ret;
  }

  void nextBytes(unsigned char *dst, size_t len) {
    while(len >= 8) {
      uint64_t u = next64();
      for(int i=0;i<8;i++) dst[i] = (u >> (i * 8)) & 0xff;
      dst += 8;
      len -= 8;
    }
    uint64_t u = next(uint32_t(len * 8));
    for(int i=0;i<(int)len;i++) dst[i] = (u >> (i * 8)) & 0xff;
  }

  unsigned clz64(uint64_t u) {
    unsigned z = 0;
    if (u & 0xffffffff00000000ULL) u >>= 32; else z += 32;
    if (u & 0x00000000ffff0000ULL) u >>= 16; else z += 16;
    if (u & 0x000000000000ff00ULL) u >>=  8; else z +=  8;
    if (u & 0x00000000000000f0ULL) u >>=  4; else z +=  4;
    if (u & 0x000000000000000cULL) u >>=  2; else z +=  2;
    if (u & 0x0000000000000002ULL) u >>=  1; else z +=  1;
    if (!u) z++;
    return z;
  }

  uint64_t nextLT(uint64_t bound) {
    if (bound == 0) return 0;

    unsigned b = sizeof(uint64_t)*8 - clz64(bound - 1);
    uint64_t r = next(b), u = uint64_t(1) << b;

    while(r >= bound) {
      r -= bound;
      u -= bound;
      while(u < bound) {
	r = (r << 1) | next(1);
	u *= 2;
      }
    }
    return r;
  }

  double nextDouble() {
    union {
      uint64_t u;
      double d;
    } c;
    c.u = next(52);
    c.u |= 0x3fe0000000000000ULL;
    return c.d;
  }
  bool nextBool() { return next(1); }
  virtual ~RNG() {}
};

class TLCG64 : public RNG {
  uint64_t state = 1;
public:
  TLCG64() {
    state = chrono::high_resolution_clock::now().time_since_epoch().count();
    for(int i=0;i<10;i++) next32();
  }

  uint32_t next32() {
    uint64_t t = chrono::high_resolution_clock::now().time_since_epoch().count();
    state = state * 6364136223846793005ULL + (t << 1) + 1;
    return uint32_t(state >> 32);
  }
  uint64_t next64() {
    uint32_t u = next32();
    return u | (uint64_t(next32()) << 32);
  }
};

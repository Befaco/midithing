#ifndef __BITFIELD128_H__
#define __BITFIELD128_H__

#include <limits.h>

typedef byte BitField128[16]; // 128 bits

static inline void setbit128(BitField128 field, size_t idx)
{
  field[idx / CHAR_BIT] |= 1u << (idx % CHAR_BIT);
}

static inline void unsetbit128(BitField128 field, size_t idx)
{
  field[idx / CHAR_BIT] &= ~(1u << (idx % CHAR_BIT));
}

static inline void togglebit128(BitField128 field, size_t idx)
{
  field[idx / CHAR_BIT] ^= 1u << (idx % CHAR_BIT);
}

static inline bool isbitset128(BitField128 field, size_t idx)
{
  return field[idx / CHAR_BIT] & (1u << (idx % CHAR_BIT));
}

static inline void clearbitset128(BitField128 field)
{
  memset(field, 0, sizeof(BitField128));
}

#endif // __BITFIELD128_H__


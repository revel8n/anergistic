#ifndef REG_T__
#define REG_T__

#include "types.h"

#pragma pack(push, 1)

struct reg_t
{
    union
    {
        u64 _u64[2]; s64 _s64[2];
        u32 _u32[4]; s32 _s32[4];
        u16 _u16[8]; s16 _s16[8];
        u8  _u8[16]; s8  _s8[16];

        f64 _f64[2];
        f32 _f32[4];
    };

    template<typename T>
    inline T get(u32 i) const;

    template<typename T>
    inline void set(u32 i, T v);

    inline u8 get_bit(u32 i) const;
    inline void set_bit(u32 i, u8 v);
};

#pragma pack(pop)

// unsigned types
template<>
inline u8 reg_t::get<u8>(u32 i) const
{
    i %= 16;

    return _u8[(i & ~3) + (3 - (i & 3))];
}
template<>
inline void reg_t::set<u8>(u32 i, u8 v)
{
    i %= 16;

    _u8[(i & ~3) + (3 - (i & 3))] = v;
}

template<>
inline u16 reg_t::get<u16>(u32 i) const
{
    i %= 8;

    return _u16[(i & ~1) + (2 - (i & 1))];
}
template<>
inline void reg_t::set<u16>(u32 i, u16 v)
{
    i %= 8;

    _u16[(i & ~1) + (2 - (i & 1))] = v;
}

template<>
inline u32 reg_t::get<u32>(u32 i) const
{
    i %= 4;

    return _u32[i];
}
template<>
inline void reg_t::set<u32>(u32 i, u32 v)
{
    i %= 4;

    _u32[i] = v;
}

template<>
inline u64 reg_t::get<u64>(u32 i) const
{
    i %= 2;

    return ((_u64[i] & 0xFFFFFFFF) << 32) | ((_u64[i] >> 32) & 0xFFFFFFFF);
}
template<>
inline void reg_t::set<u64>(u32 i, u64 v)
{
    i %= 2;

    _u64[i] = ((v & 0xFFFFFFFF) << 32) | ((v >> 32) & 0xFFFFFFFF);
}

// signed types
template<>
inline s8 reg_t::get<s8>(u32 i) const
{
    i %= 16;

    return _s8[(i & ~3) + (3 - (i & 3))];
}
template<>
inline void reg_t::set<s8>(u32 i, s8 v)
{
    i %= 16;

    _s8[(i & ~3) + (3 - (i & 3))] = v;
}

template<>
inline s16 reg_t::get<s16>(u32 i) const
{
    i %= 8;

    return _s16[(i & ~1) + (2 - (i & 1))];
}
template<>
inline void reg_t::set<s16>(u32 i, s16 v)
{
    i %= 8;

    _s16[(i & ~1) + (2 - (i & 1))] = v;
}

template<>
inline s32 reg_t::get<s32>(u32 i) const
{
    i %= 4;

    return _s32[i];
}
template<>
inline void reg_t::set<s32>(u32 i, s32 v)
{
    i %= 4;

    _s32[i] = v;
}

template<>
inline s64 reg_t::get<s64>(u32 i) const
{
    i %= 2;

    return ((_s64[i] & 0xFFFFFFFF) << 32) | ((_s64[i] >> 32) & 0xFFFFFFFF);
}
template<>
inline void reg_t::set<s64>(u32 i, s64 v)
{
    i %= 2;

    _s64[i] = ((v & 0xFFFFFFFF) << 32) | ((v >> 32) & 0xFFFFFFFF);
}

// floating point types
template<>
inline f32 reg_t::get<f32>(u32 i) const
{
    i %= 4;

    return _f32[i];
}
template<>
inline void reg_t::set<f32>(u32 i, f32 v)
{
    i %= 4;

    _f32[i] = v;
}

template<>
inline f64 reg_t::get<f64>(u32 i) const
{
    i %= 2;

    u64 v = ((_u64[i] & 0xFFFFFFFF) << 32) | ((_u64[i] >> 32) & 0xFFFFFFFF);

    return *((f64*)&v);
}
template<>
inline void reg_t::set<f64>(u32 i, f64 v)
{
    i %= 2;

    _u64[i] = ((*((u64*)&v) & 0xFFFFFFFF) << 32) | ((*((u64*)&v) >> 32) & 0xFFFFFFFF);
}

// bit handling
inline u8 reg_t::get_bit(u32 i) const
{
    i %= 128;

    return (_u32[(i / 32)] >> (31 - (i & 0x1F))) & 1;
}
inline void reg_t::set_bit(u32 i, u8 v)
{
    i %= 128;

    u32 r = i / 32;
    u32 m = 1 << (31 - (i & 0x1F));

    _u32[r] = (_u32[r] & ~m) | ((v & 1) << (31 - (i & 0x1F)));
}

#endif

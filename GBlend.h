#ifndef blending_DEFINED
#define blending_DEFINED

#include "include/GPaint.h"
#include "include/GPixel.h"
#include "include/GBlendMode.h"

typedef GPixel (*BlendProc)(GPixel, GPixel);

// static inline uint32_t absvalue(uint32_t x) {
//     uint32_t m = x >> 31;
//     return (x ^ m) - m;
// }

static inline uint8_t GDiv255(unsigned prod)
{
    return (prod + 128) * 257 >> 16;
}

/* 0 */
static inline GPixel clear_mode(GPixel src, GPixel dst)
{
    return GPixel_PackARGB(0, 0, 0, 0);
}
/* S */
static inline GPixel src_mode(GPixel src, GPixel dst)
{
    return src;
}
/* D */
static inline GPixel dst_mode(GPixel src, GPixel dst)
{
    return dst;
}
/* S + (1 - Sa)*D */
static inline GPixel src_over_mode(GPixel src, GPixel dst)
{
    int dst_a = GPixel_GetA(dst);
    // if (dst_a == 0) return src;
    int src_a = GPixel_GetA(src);
    int src_r = GPixel_GetR(src);
    int src_g = GPixel_GetG(src);
    int src_b = GPixel_GetB(src);
    int a = src_a + GDiv255((255 - src_a) * dst_a);
    int r = src_r + GDiv255((255 - src_a) * GPixel_GetR(dst));
    int g = src_g + GDiv255((255 - src_a) * GPixel_GetG(dst));
    int b = src_b + GDiv255((255 - src_a) * GPixel_GetB(dst));
    return GPixel_PackARGB(a, r, g, b);
}
/* D + (1 - Da)*S */
static inline GPixel dst_over_mode(GPixel src, GPixel dst)
{
    int dst_a = GPixel_GetA(dst);
    // if (dst_a == 0)
    //     return src;
    // if (dst_a == 255)
    // {
    //     return dst;
    // }
    int dst_r = GPixel_GetR(dst);
    int dst_g = GPixel_GetG(dst);
    int dst_b = GPixel_GetB(dst);
    int a = dst_a + GDiv255((255 - dst_a) * GPixel_GetA(src));
    int r = dst_r + GDiv255((255 - dst_a) * GPixel_GetR(src));
    int g = dst_g + GDiv255((255 - dst_a) * GPixel_GetG(src));
    int b = dst_b + GDiv255((255 - dst_a) * GPixel_GetB(src));
    return GPixel_PackARGB(a, r, g, b);
}
/* Da * S */
static inline GPixel src_in_mode(GPixel src, GPixel dst)
{
    int dst_a = GPixel_GetA(dst);
    // if (dst_a == 0)
    //     return GPixel_PackARGB(0, 0, 0, 0);
    // if (dst_a == 255)
    //     return src;
    int a = GDiv255(dst_a * GPixel_GetA(src));
    int r = GDiv255(dst_a * GPixel_GetR(src));
    int g = GDiv255(dst_a * GPixel_GetG(src));
    int b = GDiv255(dst_a * GPixel_GetB(src));
    return GPixel_PackARGB(a, r, g, b);
}
/* Sa * D */
static inline GPixel dst_in_mode(GPixel src, GPixel dst)
{
    int src_a = GPixel_GetA(src);
    int dst_a = GPixel_GetA(dst);
    // if (dst_a == 0) return GPixel_PackARGB(0,0,0,0);
    int a = GDiv255(src_a * dst_a);
    int r = GDiv255(src_a * GPixel_GetR(dst));
    int g = GDiv255(src_a * GPixel_GetG(dst));
    int b = GDiv255(src_a * GPixel_GetB(dst));
    return GPixel_PackARGB(a, r, g, b);
}
/* (1 - Da)*S */
static inline GPixel src_out_mode(GPixel src, GPixel dst)
{
    int dst_a = GPixel_GetA(dst);
    // if (dst_a == 0)
    //     return src;
    // if (dst_a == 255)
    //     return GPixel_PackARGB(0, 0, 0, 0);
    int a = GDiv255((255 - dst_a) * GPixel_GetA(src));
    int r = GDiv255((255 - dst_a) * GPixel_GetR(src));
    int g = GDiv255((255 - dst_a) * GPixel_GetG(src));
    int b = GDiv255((255 - dst_a) * GPixel_GetB(src));
    return GPixel_PackARGB(a, r, g, b);
}
/* (1 - Sa)*D */
static inline GPixel dst_out_mode(GPixel src, GPixel dst)
{
    int src_a = GPixel_GetA(src);
    int dst_a = GPixel_GetA(dst);
    // if (dst_a == 0) return GPixel_PackARGB(0,0,0,0);
    int a = GDiv255((255 - src_a) * dst_a);
    int r = GDiv255((255 - src_a) * GPixel_GetR(dst));
    int g = GDiv255((255 - src_a) * GPixel_GetG(dst));
    int b = GDiv255((255 - src_a) * GPixel_GetB(dst));
    return GPixel_PackARGB(a, r, g, b);
}
/* Da*S + (1 - Sa)*D */
static inline GPixel src_a_top_mode(GPixel src, GPixel dst)
{
    int src_a = GPixel_GetA(src);   // maybe add
    int dst_a = GPixel_GetA(dst);
    // if (dst_a == 0) return GPixel_PackARGB(0, 0, 0, 0);
    int src_r = GPixel_GetR(src);
    int src_g = GPixel_GetG(src);
    int src_b = GPixel_GetB(src);
    int dst_r = GPixel_GetR(dst);
    int dst_g = GPixel_GetG(dst);
    int dst_b = GPixel_GetB(dst);
    int a = GDiv255(dst_a * src_a + (255 - src_a) * dst_a);
    int r = GDiv255(dst_a * src_r + (255 - src_a) * dst_r);
    int g = GDiv255(dst_a * src_g + (255 - src_a) * dst_g);
    int b = GDiv255(dst_a * src_b + (255 - src_a) * dst_b);
    return GPixel_PackARGB(a, r, g, b);
}
/* Sa*D + (1 - Da)*S */
static inline GPixel dst_a_top_mode(GPixel src, GPixel dst)
{
    int src_a = GPixel_GetA(src);
    int dst_a = GPixel_GetA(dst);
    // if (dst_a == 0)
    //     return src;
    // if (dst_a == 255) return dst_in_mode(src, dst);
    int src_r = GPixel_GetR(src);
    int src_g = GPixel_GetG(src);
    int src_b = GPixel_GetB(src);
    int dst_r = GPixel_GetR(dst);
    int dst_g = GPixel_GetG(dst);
    int dst_b = GPixel_GetB(dst);
    int a = GDiv255(src_a * dst_a + (255 - dst_a) * src_a);
    int r = GDiv255(src_a * dst_r + (255 - dst_a) * src_r);
    int g = GDiv255(src_a * dst_g + (255 - dst_a) * src_g);
    int b = GDiv255(src_a * dst_b + (255 - dst_a) * src_b);
    return GPixel_PackARGB(a, r, g, b);
}
/* (1 - Sa)*D + (1 - Da)*S */
static inline GPixel xor_mode(GPixel src, GPixel dst)
{
    int src_a = GPixel_GetA(src);
    int dst_a = GPixel_GetA(dst);
    // if (dst_a == 0)
    //     return src;
    // if (dst_a == 255 && src_a == 255)
    //     return GPixel_PackARGB(0, 0, 0, 0);
    int src_r = GPixel_GetR(src);
    int src_g = GPixel_GetG(src);
    int src_b = GPixel_GetB(src);
    int dst_r = GPixel_GetR(dst);
    int dst_g = GPixel_GetG(dst);
    int dst_b = GPixel_GetB(dst);
    int a = GDiv255((255 - src_a) * dst_a + (255 - dst_a) * src_a);
    int r = GDiv255((255 - src_a) * dst_r + (255 - dst_a) * src_r);
    int g = GDiv255((255 - src_a) * dst_g + (255 - dst_a) * src_g);
    int b = GDiv255((255 - src_a) * dst_b + (255 - dst_a) * src_b);
    return GPixel_PackARGB(a, r, g, b);
}

static inline GBlendMode check255(GBlendMode mode) {
    switch(mode) {
        case GBlendMode::kSrcOver:
            return GBlendMode::kSrc;
        case GBlendMode::kDstIn:
            return GBlendMode::kDst;
        case GBlendMode::kDstOut:
            return GBlendMode::kClear;
        case GBlendMode::kSrcATop:
            return GBlendMode::kSrcIn;
        case GBlendMode::kXor:
            return GBlendMode::kSrcOut;
        default:
            return mode;
    }
}

static inline GBlendMode check0(GBlendMode mode) {
    switch(mode) {
        case GBlendMode::kSrc: 
            return GBlendMode::kClear;
        case GBlendMode::kSrcOver:
            return GBlendMode::kDst;
        case GBlendMode::kDstOver:
            return GBlendMode::kDst;
        case GBlendMode::kSrcIn:
            return GBlendMode::kClear;
        case GBlendMode::kDstIn:
            return GBlendMode::kClear;
        case GBlendMode::kSrcOut:
            return GBlendMode::kClear;
        case GBlendMode::kDstOut:
            return GBlendMode::kDst;
        case GBlendMode::kSrcATop:
            return GBlendMode::kDst;
        case GBlendMode::kDstATop:
            return GBlendMode::kClear;
        case GBlendMode::kXor:
            return GBlendMode::kDst;
        default:
            return mode;
    }
}

const BlendProc gProcs[] = {
    clear_mode, src_mode, dst_mode, src_over_mode, dst_over_mode, src_in_mode,
    dst_in_mode, src_out_mode, dst_out_mode, src_a_top_mode, dst_a_top_mode, xor_mode
};

#endif
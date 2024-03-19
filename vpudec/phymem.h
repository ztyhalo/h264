#ifndef PHYMEM_H
#define PHYMEM_H

#ifdef ARM
#include <stdint.h>
//#include <stdlib.h>
//#include "imx-mm/vpu/vpu_wrapper.h"
//#include "zprint/zprint.h"
//#include <list>
//#include "g2d.h"

typedef enum {
  GST_VIDEO_FORMAT_UNKNOWN,
  GST_VIDEO_FORMAT_ENCODED,
  GST_VIDEO_FORMAT_I420,
  GST_VIDEO_FORMAT_YV12,
  GST_VIDEO_FORMAT_YUY2,
  GST_VIDEO_FORMAT_UYVY,
  GST_VIDEO_FORMAT_AYUV,
  GST_VIDEO_FORMAT_RGBx,
  GST_VIDEO_FORMAT_BGRx,
  GST_VIDEO_FORMAT_xRGB,
  GST_VIDEO_FORMAT_xBGR,
  GST_VIDEO_FORMAT_RGBA,
  GST_VIDEO_FORMAT_BGRA,
  GST_VIDEO_FORMAT_ARGB,
  GST_VIDEO_FORMAT_ABGR,
  GST_VIDEO_FORMAT_RGB,
  GST_VIDEO_FORMAT_BGR,
  GST_VIDEO_FORMAT_Y41B,
  GST_VIDEO_FORMAT_Y42B,
  GST_VIDEO_FORMAT_YVYU,
  GST_VIDEO_FORMAT_Y444,
  GST_VIDEO_FORMAT_v210,
  GST_VIDEO_FORMAT_v216,
  GST_VIDEO_FORMAT_NV12,
  GST_VIDEO_FORMAT_NV21,
  GST_VIDEO_FORMAT_GRAY8,
  GST_VIDEO_FORMAT_GRAY16_BE,
  GST_VIDEO_FORMAT_GRAY16_LE,
  GST_VIDEO_FORMAT_v308,
  GST_VIDEO_FORMAT_RGB16,
  GST_VIDEO_FORMAT_BGR16,
  GST_VIDEO_FORMAT_RGB15,
  GST_VIDEO_FORMAT_BGR15,
  GST_VIDEO_FORMAT_UYVP,
  GST_VIDEO_FORMAT_A420,
  GST_VIDEO_FORMAT_RGB8P,
  GST_VIDEO_FORMAT_YUV9,
  GST_VIDEO_FORMAT_YVU9,
  GST_VIDEO_FORMAT_IYU1,
  GST_VIDEO_FORMAT_ARGB64,
  GST_VIDEO_FORMAT_AYUV64,
  GST_VIDEO_FORMAT_r210,
  GST_VIDEO_FORMAT_I420_10BE,
  GST_VIDEO_FORMAT_I420_10LE,
  GST_VIDEO_FORMAT_I422_10BE,
  GST_VIDEO_FORMAT_I422_10LE,
  GST_VIDEO_FORMAT_Y444_10BE,
  GST_VIDEO_FORMAT_Y444_10LE,
  GST_VIDEO_FORMAT_GBR,
  GST_VIDEO_FORMAT_GBR_10BE,
  GST_VIDEO_FORMAT_GBR_10LE,
  GST_VIDEO_FORMAT_NV16,
  GST_VIDEO_FORMAT_NV24,
  GST_VIDEO_FORMAT_NV12_64Z32,
  GST_VIDEO_FORMAT_A420_10BE,
  GST_VIDEO_FORMAT_A420_10LE,
  GST_VIDEO_FORMAT_A422_10BE,
  GST_VIDEO_FORMAT_A422_10LE,
  GST_VIDEO_FORMAT_A444_10BE,
  GST_VIDEO_FORMAT_A444_10LE,
  GST_VIDEO_FORMAT_NV61,
  GST_VIDEO_FORMAT_P010_10BE,
  GST_VIDEO_FORMAT_P010_10LE,
  GST_VIDEO_FORMAT_IYU2,
  GST_VIDEO_FORMAT_VYUY,
  GST_VIDEO_FORMAT_GBRA,
  GST_VIDEO_FORMAT_GBRA_10BE,
  GST_VIDEO_FORMAT_GBRA_10LE,
  GST_VIDEO_FORMAT_GBR_12BE,
  GST_VIDEO_FORMAT_GBR_12LE,
  GST_VIDEO_FORMAT_GBRA_12BE,
  GST_VIDEO_FORMAT_GBRA_12LE,
  GST_VIDEO_FORMAT_I420_12BE,
  GST_VIDEO_FORMAT_I420_12LE,
  GST_VIDEO_FORMAT_I422_12BE,
  GST_VIDEO_FORMAT_I422_12LE,
  GST_VIDEO_FORMAT_Y444_12BE,
  GST_VIDEO_FORMAT_Y444_12LE,
  GST_VIDEO_FORMAT_GRAY10_LE32,
  GST_VIDEO_FORMAT_NV12_10LE32,
  GST_VIDEO_FORMAT_NV16_10LE32,
  GST_VIDEO_FORMAT_NV12_10LE40,
  GST_VIDEO_FORMAT_Y210,
  GST_VIDEO_FORMAT_Y410,
  GST_VIDEO_FORMAT_VUYA,
  GST_VIDEO_FORMAT_BGR10A2_LE,
  GST_VIDEO_FORMAT_RGB10A2_LE,
  GST_VIDEO_FORMAT_Y444_16BE,
  GST_VIDEO_FORMAT_Y444_16LE,
  GST_VIDEO_FORMAT_P016_BE,
  GST_VIDEO_FORMAT_P016_LE,
  GST_VIDEO_FORMAT_P012_BE,
  GST_VIDEO_FORMAT_P012_LE,
  GST_VIDEO_FORMAT_Y212_BE,
  GST_VIDEO_FORMAT_Y212_LE,
  GST_VIDEO_FORMAT_Y412_BE,
  GST_VIDEO_FORMAT_Y412_LE,
  /**
   * GST_VIDEO_FORMAT_NV12_4L4:
   *
   * NV12 with 4x4 tiles in linear order.
   *
   * Since: 1.18
   */
  GST_VIDEO_FORMAT_NV12_4L4,
  /**
   * GST_VIDEO_FORMAT_NV12_32L32:
   *
   * NV12 with 32x32 tiles in linear order.
   *
   * Since: 1.18
   */
  GST_VIDEO_FORMAT_NV12_32L32,

  /**
   * GST_VIDEO_FORMAT_RGBP:
   *
   * Planar 4:4:4 RGB, R-G-B order
   *
   * Since: 1.20
   */
  GST_VIDEO_FORMAT_RGBP,

  /**
   * GST_VIDEO_FORMAT_BGRP:
   *
   * Planar 4:4:4 RGB, B-G-R order
   *
   * Since: 1.20
   */
  GST_VIDEO_FORMAT_BGRP,

  /**
   * GST_VIDEO_FORMAT_AV12:
   *
   * Planar 4:2:0 YUV with interleaved UV plane with alpha as
   * 3rd plane.
   *
   * Since: 1.20
   */
  GST_VIDEO_FORMAT_AV12,

  /**
   * GST_VIDEO_FORMAT_ARGB64_LE:
   *
   * RGB with alpha channel first, 16 bits (little endian)
   * per channel.
   *
   * Since: 1.20
   */
  GST_VIDEO_FORMAT_ARGB64_LE,

  /**
   * GST_VIDEO_FORMAT_ARGB64_BE:
   *
   * RGB with alpha channel first, 16 bits (big endian)
   * per channel.
   *
   * Since: 1.20
   */
  GST_VIDEO_FORMAT_ARGB64_BE,

  /**
   * GST_VIDEO_FORMAT_RGBA64_LE:
   *
   * RGB with alpha channel last, 16 bits (little endian)
   * per channel.
   *
   * Since: 1.20
   */
  GST_VIDEO_FORMAT_RGBA64_LE,

  /**
   * GST_VIDEO_FORMAT_RGBA64_BE:
   *
   * RGB with alpha channel last, 16 bits (big endian)
   * per channel.
   *
   * Since: 1.20
   */
  GST_VIDEO_FORMAT_RGBA64_BE,

  /**
   * GST_VIDEO_FORMAT_BGRA64_LE:
   *
   * Reverse RGB with alpha channel last, 16 bits (little endian)
   * per channel.
   *
   * Since: 1.20
   */
  GST_VIDEO_FORMAT_BGRA64_LE,

  /**
   * GST_VIDEO_FORMAT_BGRA64_BE:
   *
   * Reverse RGB with alpha channel last, 16 bits (big endian)
   * per channel.
   *
   * Since: 1.20
   */
  GST_VIDEO_FORMAT_BGRA64_BE,

  /**
   * GST_VIDEO_FORMAT_ABGR64_LE:
   *
   * Reverse RGB with alpha channel first, 16 bits (little endian)
   * per channel.
   *
   * Since: 1.20
   */
  GST_VIDEO_FORMAT_ABGR64_LE,

  /**
   * GST_VIDEO_FORMAT_ABGR64_BE:
   *
   * Reverse RGB with alpha channel first, 16 bits (big endian)
   * per channel.
   *
   * Since: 1.20
   */
  GST_VIDEO_FORMAT_ABGR64_BE,
} GstVideoFormat;


typedef struct {
  uint8_t *vaddr;
  uint8_t *paddr;
  uint8_t *caddr;
  uint32_t size;
  void *user_data;
} PhyMemBlock;


typedef struct {
  char *name;
  int fmt;
  uint32_t width;
  uint32_t height;
} DisplayInfo;

typedef struct {
  int left;
  int top;
  int right;
  int bottom;
  int width;
  int height;
} SurfaceRect;

typedef struct {
  int left;
  int top;
  int right;
  int bottom;
} DestRect;

typedef struct {
  int fmt;
  int rot;
  int alpha;
  bool keep_ratio;
  int zorder;
  SurfaceRect src;
  DestRect dst;
} SurfaceInfo;

typedef PhyMemBlock SurfaceBuffer;

#endif //ARM
#endif // VPUDEC_H

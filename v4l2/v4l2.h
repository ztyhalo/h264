#ifndef V4L2_H
#define V4L2_H

#include <iostream>
#include <linux/fb.h>
#include <string.h>
#include <stdint.h>
#include <uapi/linux/mxc_v4l2.h>
#include <uapi/linux/mxcfb.h>
#include <uapi/linux/videodev2.h>
#include "vpudec/phymem.h"
#include "g2d.h"

using namespace std;

#define RGB888TORGB565(rgb)\
    ((((rgb)<<8)>>27<<11)|(((rgb)<<18)>>26<<5)|(((rgb)<<27)>>27))

#define RGB565TOCOLORKEY(rgb)                              \
      ( ((rgb & 0xf800)<<8)  |  ((rgb & 0xe000)<<3)  |     \
        ((rgb & 0x07e0)<<5)  |  ((rgb & 0x0600)>>1)  |     \
        ((rgb & 0x001f)<<3)  |  ((rgb & 0x001c)>>2)  )

typedef struct v4l2_rect IMXV4l2Rect;

#define UPALIGNTO8(a) ((a + 7) & (~7))
#define DOWNALIGNTO8(a) ((a) & (~7))

#define DISPLAY_NUM_BUFFERS (3)

/**
 * GST_MAKE_FOURCC:
 * @a: the first character
 * @b: the second character
 * @c: the third character
 * @d: the fourth character
 *
 * Transform four characters into a #guint32 fourcc value with host
 * endianness.
 * <informalexample>
 * <programlisting>
 * guint32 fourcc = GST_MAKE_FOURCC ('M', 'J', 'P', 'G');
 * </programlisting>
 * </informalexample>
 */
#define GST_MAKE_FOURCC(a,b,c,d)        ((uint32_t)((a)|(b)<<8|(c)<<16|(d)<<24))

typedef struct {
  const char *name;
  bool bg;
  const char *bg_fb_name;
} IMXV4l2DeviceMap;

//typedef struct {
//  char *name;
//  int fmt;
//  uint32_t width;
//  uint32_t height;
//} DisplayInfo;


class DisplayDev
{
public:
    DisplayDev(string dev);
    ~DisplayDev();
    void get_display_resolution (char * device);
    int open_display_dev(int ty);

public:
    string name;
    int width;
    int height;
    int type;
    int fd;
    int device_map_id;

};

typedef struct {
  int fmt;
  int width;
  int height;
} G2DHandle;

typedef struct {
  struct g2d_surface src;
  struct g2d_surface dst;
} G2DSurface;

class G2dDevice
{
public:
    G2dDevice();
    ~G2dDevice();
    int g2d_device_open(int fmt, int width, int height);
    int g2d_device_update_surface_info (SurfaceInfo *info, void * surface);
    int g2d_device_update_surface_info (SurfaceInfo *info);
    G2DSurface * g2d_device_create_surface (SurfaceInfo *info);
    int g2d_device_blit_surface(SurfaceBuffer *buffer, SurfaceBuffer *dest);
public:
    G2DHandle handle;
    G2DSurface in_surface;
};


class V4L2
{
public:
    V4L2();
    ~V4L2();
    int v4l2_open_display_dev(int ty);
    int v4l2out_config_output(struct v4l2_crop * crop);
    int v4l2_enum_fmt(void);
    int v4l2out_config_alpha(int alpha);
    int imx_ipu_v4l2_config_colorkey (bool enable, uint32_t color_key);
    int imx_v4l2out_config_input (uint32_t fmt, IMXV4l2Rect *crop);
    int imx_v4l2_reset_device(void);
    int imx_v4l2_set_buffer_count (uint32_t count, uint32_t memory_mode);
    int imx_v4l2_allocate_buffer (PhyMemBlock *mem);
    int v4l2_display_init(string devname);
    void imx_v4l2_free_buffer(void);
    int imx_v4l2_dequeue_v4l2memblk(PhyMemBlock **mem);
    int get_next_display_buffer (SurfaceBuffer *buffer);
    int imx_v4l2_queue_v4l2memblk (PhyMemBlock *mem);
    v4l2_buffer * imx_v4l2_find_buffer(PhyMemBlock *mem);
    int imx_v4l2_do_queue_buffer(struct v4l2_buffer *v4l2buf);
public:
    DisplayDev * dev;
    G2dDevice  * g2d;
    int v4l2_fd;
    bool streamon;
    int type;
    int queued_count;
    int buffer_count;
    IMXV4l2Rect icrop;
    uint32_t ifmt;
    PhyMemBlock memblk[DISPLAY_NUM_BUFFERS];
    int allocated;
    v4l2_buffer v4l2buffer[DISPLAY_NUM_BUFFERS];
    struct v4l2_buffer * v4lbuf_queued_before_streamon[DISPLAY_NUM_BUFFERS];
    int first_request;
    int streamon_count;

};

#endif // V4L2_H

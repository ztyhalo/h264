#include "v4l2.h"
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>

static IMXV4l2DeviceMap g_device_maps[] = {
  {"/dev/video0", false, "/dev/fb0"},
  {"/dev/video16", true, "/dev/fb0"},
  {"/dev/video17", false, "/dev/fb0"},
  {"/dev/video18", true, "/dev/fb2"},
  {"/dev/video19", false, "/dev/fb2"},
  {"/dev/video20", true, "/dev/fb4"}
};

void print_crop_info(struct v4l2_crop *crop)
{
    printf("zty printf crop type %d!\n", crop->type);
    printf("zty crop info (%d, %d) -> (%d, %d).\n", crop->c.left, crop->c.top, crop->c.width, crop->c.height);
}
static uint32_t string_to_fmt (char *value)
{
  uint32_t fmt, a, b, c, d;

  a = value[0];
  b = value[1];
  c = value[2];
  d = value[3];

  fmt = (((a) << 0) | ((b) << 8) | ((c) << 16) | ((d) << 24));

  return fmt;
}

static uint32_t display_fmt_to_v4l2_fmt (uint32_t display_fmt)
{
  uint32_t fmt = 0;
  if (display_fmt == GST_MAKE_FOURCC('R', 'G', 'B', 'P'))
    fmt = V4L2_PIX_FMT_RGB565;
  else if (display_fmt == GST_MAKE_FOURCC('R', 'G', 'B', 'x'))
    fmt = V4L2_PIX_FMT_RGB32;

  return fmt;
}

DisplayDev::DisplayDev(string dev)
{
    name  = dev;
}

DisplayDev::~DisplayDev()
{
    ;
}
#define DEFAULTW (320)
#define DEFAULTH (240)
void DisplayDev::get_display_resolution (char * device)
{
    struct fb_var_screeninfo fb_var;
     int i;
//     int device_map_id;
     int fd;

     width = DEFAULTW;
     height = DEFAULTH;

     for (i=0; i<sizeof(g_device_maps)/sizeof(IMXV4l2DeviceMap); i++)
     {
       if (!strcmp (device, g_device_maps[i].name)) {
         device_map_id = i;
         break;
       }
     }

     fd = open (g_device_maps[device_map_id].bg_fb_name, O_RDWR, 0);
     if (fd < 0) {
       printf ("ERROR: Can't open %s.\n", g_device_maps[device_map_id].bg_fb_name);
       return;
     }

     if (ioctl (fd, FBIOGET_VSCREENINFO, &fb_var) < 0)
     {
       printf ("ERROR: Can't get display resolution, use default (%dx%d).\n", DEFAULTW, DEFAULTH);
       close (fd);
       return;
     }

     width = fb_var.xres;
     height = fb_var.yres;
     printf ("display(%s) resolution is (%dx%d).\n", g_device_maps[device_map_id].bg_fb_name, fb_var.xres, fb_var.yres);

     close (fd);

     return;
}

G2dDevice::G2dDevice()
{

    memset(&in_surface, 0 , sizeof(in_surface));

    memset(&handle, 0 , sizeof(handle));


}
G2dDevice::~G2dDevice()
{
    ;
}

int G2dDevice::g2d_device_open(int fmt, int width, int height)
{
    int g2d_fmt;

    SurfaceInfo info;

    if(fmt == GST_MAKE_FOURCC('R', 'G', 'B', 'P'))
      g2d_fmt = G2D_RGB565;
    else if(fmt == GST_MAKE_FOURCC('R', 'G', 'B', 'x'))
      g2d_fmt = G2D_RGBX8888;
    else {
      printf ("dst format (%x) is not supported.", fmt);
      return -1;
    }
    memset(&info, 0 , sizeof(info));
    handle.fmt = g2d_fmt;
    handle.width = width;
    handle.height = height;

    info.fmt = GST_VIDEO_FORMAT_NV12;
    info.src.width = 1920;
    info.src.height = 1088;
    info.src.left = 0;
    info.src.top = 0;
    info.src.right = 1920;
    info.src.bottom = 1080;
    info.dst.right = 1920;
    info.dst.bottom = 1080;

    g2d_device_update_surface_info(&info);

    return 0;
}

int G2dDevice::g2d_device_update_surface_info (SurfaceInfo *info, void * surface)
{
    G2DHandle *hdevice = &handle;
    G2DSurface *hsurface = (G2DSurface*) surface;

    if(info->fmt == GST_VIDEO_FORMAT_I420)
        hsurface->src.format = G2D_I420;
      else if(info->fmt == GST_VIDEO_FORMAT_NV12)
        hsurface->src.format = G2D_NV12;
      else if(info->fmt == GST_VIDEO_FORMAT_YV12)
        hsurface->src.format = G2D_YV12;
      else if(info->fmt == GST_VIDEO_FORMAT_NV16)
        hsurface->src.format = G2D_NV16;
      else if(info->fmt == GST_VIDEO_FORMAT_YUY2)
        hsurface->src.format = G2D_YUYV;
      else if(info->fmt == GST_VIDEO_FORMAT_UYVY)
        hsurface->src.format = G2D_UYVY;
      else if(info->fmt == GST_VIDEO_FORMAT_RGB16)
        hsurface->src.format = G2D_RGB565;
      else if(info->fmt == GST_VIDEO_FORMAT_RGBx)
        hsurface->src.format = G2D_RGBX8888;
      else {
        printf ("source format (%x) is not supported.!\n", info->fmt);
        return -1;
      }

    hsurface->src.width = info->src.width;
    hsurface->src.height = info->src.height;
    hsurface->src.stride = info->src.width;
    hsurface->src.left = info->src.left;
    hsurface->src.top = info->src.top;
    hsurface->src.right = info->src.right;
    hsurface->src.bottom = info->src.bottom;

    printf ("zty source, format (%x), res (%d,%d), crop (%d,%d) --> (%d,%d)!\n",
     hsurface->src.format, hsurface->src.width, hsurface->src.height,
     hsurface->src.left, hsurface->src.top, hsurface->src.right, hsurface->src.bottom);

    hsurface->dst.format = (g2d_format) hdevice->fmt;
      hsurface->dst.width = hdevice->width;
      hsurface->dst.height = hdevice->height;
      hsurface->dst.stride = hdevice->width;
      hsurface->dst.left = info->dst.left;
      hsurface->dst.top = info->dst.top;
      hsurface->dst.right = info->dst.right;
      hsurface->dst.bottom = info->dst.bottom;
      switch (info->rot) {
        case 0:
          hsurface->dst.rot = G2D_ROTATION_0;
          break;
        case 90:
          hsurface->dst.rot = G2D_ROTATION_90;
          break;
        case 180:
          hsurface->dst.rot = G2D_ROTATION_180;
          break;
        case 270:
          hsurface->dst.rot = G2D_ROTATION_270;
          break;
        default:
          hsurface->dst.rot = G2D_ROTATION_0;
          break;
    }

      printf ("zty dest, format (%x), res (%d,%d), crop (%d,%d) --> (%d,%d)!\n",
       hsurface->dst.format, hsurface->dst.width, hsurface->dst.height,
       hsurface->dst.left, hsurface->dst.top, hsurface->dst.right, hsurface->dst.bottom);
      return 0;

}


int G2dDevice::g2d_device_update_surface_info (SurfaceInfo *info)
{
    return g2d_device_update_surface_info (info, &in_surface);
}

G2DSurface * G2dDevice::g2d_device_create_surface (SurfaceInfo *info)
{
    G2DSurface *surface;
    printf("zty g2d_device_create_surface!\n");
    surface = (G2DSurface *)malloc (sizeof(G2DSurface));
    if (!surface) {
      printf ("failed allocate G2DSurface.!\n");
      return NULL;
    }

    memset(surface, 0, sizeof(G2DSurface));

    if (g2d_device_update_surface_info (info, surface) < 0)
    {
      free (surface);
      return NULL;
    }

    return surface;
}

int G2dDevice::g2d_device_blit_surface(SurfaceBuffer *buffer, SurfaceBuffer *dest)
{
//  G2DHandle *hdevice = (G2DHandle*) &handle;
  G2DSurface *hsurface = (G2DSurface *)&in_surface;
  void *g2d_handle = NULL;

  if(g2d_open(&g2d_handle) == -1 || g2d_handle == NULL) {
    printf ("Failed to open g2d device.!\n");
    return -1;
  }

  switch(hsurface->src.format) {
    case G2D_I420:
      hsurface->src.planes[0] = (int)buffer->paddr;
      hsurface->src.planes[1] = (int)(buffer->paddr + hsurface->src.width * hsurface->src.height);
      hsurface->src.planes[2] = hsurface->src.planes[1]  + hsurface->src.width * hsurface->src.height / 4;
      //GST_DEBUG ("YUV address: %p, %p, %p", hsurface->src.planes[0], hsurface->src.planes[1], hsurface->src.planes[2]);
      break;
    case G2D_YV12:
      hsurface->src.planes[0] = (int)(buffer->paddr);
      hsurface->src.planes[2] = (int)(buffer->paddr + hsurface->src.width * hsurface->src.height);
      hsurface->src.planes[1] = hsurface->src.planes[2]  + hsurface->src.width * hsurface->src.height / 4;
      //GST_DEBUG ("YUV address: %p, %p, %p", hsurface->src.planes[0], hsurface->src.planes[1], hsurface->src.planes[2]);
      break;
    case G2D_NV12:
      hsurface->src.planes[0] = (int)(buffer->paddr);
      hsurface->src.planes[1] = (int)(buffer->paddr + hsurface->src.width * hsurface->src.height);
      break;
    case G2D_NV16:
      hsurface->src.planes[0] = (int)(buffer->paddr);
      hsurface->src.planes[1] = (int)(buffer->paddr + hsurface->src.width * hsurface->src.height);
      break;
    case G2D_RGB565:
    case G2D_RGBX8888:
    case G2D_UYVY:
    case G2D_YUYV:
      hsurface->src.planes[0] = (int)(buffer->paddr);
      break;
    default:
      printf ("not supported format.!\n");
      g2d_close (g2d_handle);
      return -1;
  }


  hsurface->dst.planes[0] = (int)(dest->paddr);
//  printf("zty change src paddr 0x%x  dest paddr 0x%x!\n", buffer->paddr, dest->paddr);
//  printf ("dest, format (%x), res (%d,%d), crop (%d,%d) --> (%d,%d)!\n",
//      hsurface->dst.format, hsurface->dst.width, hsurface->dst.height,
//      hsurface->dst.left, hsurface->dst.top, hsurface->dst.right, hsurface->dst.bottom);

  g2d_blit(g2d_handle, &hsurface->src, &hsurface->dst);
  g2d_finish(g2d_handle);
  g2d_close (g2d_handle);

  return 0;
}


V4L2::V4L2()
{
    dev = NULL;
    g2d = NULL;
    v4l2_fd = 0;
    streamon = false;
    type = 0;
    queued_count = 0;
    streamon_count = 1;
    allocated = 0;
    memset(&icrop, 0, sizeof(icrop));
    ifmt = 0;
    buffer_count = 0;
    memset(memblk, 0x00, sizeof(memblk));
    first_request = 0;
    memset(v4lbuf_queued_before_streamon, 0, sizeof(v4lbuf_queued_before_streamon));
}
V4L2::~V4L2()
{
    if(dev != NULL)
    {
        delete dev;
        dev = NULL;
    }
//    if(v4l2_fd > 0)
//    {
//        close(v4l2_fd);
//        v4l2_fd = 0;
//    }
    if(g2d != NULL)
    {
        delete g2d;
        g2d = NULL;
    }
    imx_v4l2_free_buffer();

}

int V4L2::v4l2_enum_fmt(void)
{
    struct v4l2_fmtdesc fmt;

    if(v4l2_fd <= 0)
        return -1;

    memset(&fmt, 0, sizeof(fmt));
    fmt.index = 0;
    fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

    while(ioctl(v4l2_fd, VIDIOC_ENUM_FMT, &fmt) == 0)
    {
        fmt.index++;
        printf("{ pixelformat = ''%c%c%c%c'', description = ''%s'' }\n", fmt.pixelformat & 0xFF,
            (fmt.pixelformat >> 8) & 0xFF, (fmt.pixelformat >> 16) & 0xFF, (fmt.pixelformat >> 24) & 0xFF,
            fmt.description);
    }
    return 0;

}

int V4L2::v4l2out_config_output(struct v4l2_crop * crop)
{
    struct v4l2_cropcap cropcap;

    memset(&cropcap, 0, sizeof(cropcap));
    cropcap.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    if (ioctl(v4l2_fd, VIDIOC_CROPCAP, &cropcap) < 0)
    {
      printf("Get crop capability failed.");
      return -1;
    }

    crop->type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

    print_crop_info(crop);

    if (ioctl(v4l2_fd, VIDIOC_S_CROP, crop) < 0)
    {
      printf ("Set crop failed.");
      return -1;
    }

    return 0;

}


int V4L2::v4l2_open_display_dev(int ty)
{
   struct v4l2_capability cap;
   struct v4l2_crop crop;
   int ret = 0;

   if (ty == V4L2_BUF_TYPE_VIDEO_CAPTURE)
   {
      v4l2_fd = open(dev->name.c_str(), O_RDWR, 0);
   }
   else
   {
      v4l2_fd = open(dev->name.c_str(), O_RDWR | O_NONBLOCK, 0);
   }

   if(v4l2_fd < 0)
   {
       printf("can not open %s.\n", dev->name.c_str());
       return -1;
   }

   if (ioctl(v4l2_fd, VIDIOC_QUERYCAP, &cap) < 0)
   {
     printf("VIDIOC_QUERYCAP error.");
     close (v4l2_fd);
     return -2;
   }

   if (!(cap.capabilities & ty))
   {
     printf("device can't capture.");
     close (v4l2_fd);
     return -2;
   }
   type = ty;
   if (ty == V4L2_BUF_TYPE_VIDEO_OUTPUT)
   {
       memset(&crop, 0x00, sizeof(crop));
       crop.c.width = dev->width;
       crop.c.height = dev->height;
       icrop.width = dev->width;
       icrop.height = dev->height;
       ret = v4l2out_config_output(&crop);
       if(ret != 0)
       {
           printf("v4l2out_config_output error!\n");
           return ret;
       }
   }

   return 0;
}

int V4L2::imx_v4l2_reset_device(void)
{
    if(v4l2_fd > 0)
    {
        if(streamon)
        {
            if(ioctl(v4l2_fd, VIDIOC_STREAMOFF, &type) < 0)
            {
                printf("stream off failed!\n");
                return -1;
            }
            streamon = false;
        }
        printf("v4l2 dev hold %d buffers when reset.!\n", queued_count);
    }

    return 0;
}

int V4L2::v4l2out_config_alpha(int alpha)
{
    struct mxcfb_gbl_alpha galpha;
    char *device = (char*) g_device_maps[dev->device_map_id].bg_fb_name;
    int fd;

    fd = open (device, O_RDWR, 0);
    if (fd < 0)
    {
        printf ("Can't open %s.\n", device);
        return -1;
    }

    printf("set alpha to (%d) for display (%s)", alpha, device);

    galpha.alpha = alpha;
    galpha.enable = 1;

    if (ioctl(fd, MXCFB_SET_GBL_ALPHA, &galpha) < 0)
    {
        printf ("Set %d global alpha failed.", alpha);
    }

    close (fd);

    return 0;

}

int V4L2::imx_ipu_v4l2_config_colorkey (bool enable, uint32_t color_key)
{
  struct mxcfb_color_key colorKey;
  char *device = (char*)g_device_maps[dev->device_map_id].bg_fb_name;
  int fd;
  struct fb_var_screeninfo fbVar;

  fd = open (device, O_RDWR, 0);
  if (fd < 0)
  {
    printf ("Can't open %s.", device);
    return -1;
  }

  if (ioctl(fd, FBIOGET_VSCREENINFO, &fbVar) < 0)
  {
    printf("get vscreen info failed.");
  }
  else
  {
    if (fbVar.bits_per_pixel == 16)
    {
      colorKey.color_key = RGB565TOCOLORKEY(RGB888TORGB565(color_key));
      printf("%08X:%08X", colorKey.color_key, color_key);
    }
    else if (fbVar.bits_per_pixel == 24 || fbVar.bits_per_pixel == 32)
    {
      colorKey.color_key = color_key;
    }
  }

  if (enable)
  {
    colorKey.enable = 1;
    printf ("set colorKey to (%x) for display (%s)", colorKey.color_key, device);
  }
  else
  {
    colorKey.enable = 0;
    printf ("disable colorKey for display (%s)", device);
  }

  if (ioctl (fd, MXCFB_SET_CLR_KEY, &colorKey) < 0)
  {
    printf ("Set %s color key failed.", device);
  }

  close (fd);
  return 0;
}

int V4L2::imx_v4l2out_config_input (uint32_t fmt, IMXV4l2Rect *crop)
{

  struct v4l2_format v4l2fmt;
//  struct v4l2_rect icrop;

  if (imx_v4l2_reset_device() < 0)
    return -1;

  printf("zty config in, fmt(%x), res(%dx%d), crop((%d,%d) -> (%d,%d))",
      fmt, dev->width, dev->height, crop->left, crop->top, crop->width, crop->height);

  //align to 8 pixel for IPU limitation
  crop->left = UPALIGNTO8 (crop->left);
  crop->top = UPALIGNTO8 (crop->top);
  crop->width = DOWNALIGNTO8 (crop->width);
  crop->height = DOWNALIGNTO8 (crop->height);

  if (crop->width <=0 || crop->height <= 0)
  {
    return 1;
  }

  memset(&v4l2fmt, 0, sizeof(struct v4l2_format));
  v4l2fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  v4l2fmt.fmt.pix.width =   dev->width;
  v4l2fmt.fmt.pix.height =  dev->height;
  v4l2fmt.fmt.pix.pixelformat = ifmt = fmt;
  icrop.left = crop->left;
  icrop.top = crop->top;
  icrop.width = crop->width;
  icrop.height = crop->height;
  v4l2fmt.fmt.pix.priv = (unsigned int)&icrop;

  if (ioctl(v4l2_fd, VIDIOC_S_FMT, &v4l2fmt) < 0)
  {
    printf ("Set format failed.!\n");
    return -1;
  }

  if (ioctl(v4l2_fd, VIDIOC_G_FMT, &v4l2fmt) < 0)
  {
    printf ("Get format failed.!\n");
    return -1;
  }

  return 0;
}

int V4L2::imx_v4l2_set_buffer_count (uint32_t count, uint32_t memory_mode)
{
     struct v4l2_requestbuffers buf_req;
     printf("requeset for (%d) buffers.!\n", count);

     memset(&buf_req, 0, sizeof(buf_req));
     buf_req.type = type;
     buf_req.count = count;
     buf_req.memory = memory_mode;

     if (ioctl(v4l2_fd, VIDIOC_REQBUFS, &buf_req) < 0)
     {
       printf("Request %d buffers failed\n", count);
       return -1;
     }

     buffer_count = count;

     return 0;

}

int V4L2::imx_v4l2_allocate_buffer (PhyMemBlock *mem)
{
    struct v4l2_buffer *v4l2buf;

    if(allocated >= buffer_count)
    {
        printf("No more v4l2 buffer for allocating. allocated %d buffer_count %d\n", allocated, buffer_count);
        return -1;
    }
    v4l2buf = &v4l2buffer[allocated];

    memset(v4l2buf, 0, sizeof(struct v4l2_buffer));

    v4l2buf->type = type;
    v4l2buf->memory = V4L2_MEMORY_MMAP;
    v4l2buf->index = allocated;

    if (ioctl(v4l2_fd, VIDIOC_QUERYBUF, v4l2buf) < 0)
    {
      printf ("VIDIOC_QUERYBUF error.!\n");
      return -2;
    }

    mem->size = v4l2buf->length;
    mem->vaddr = (uint8_t *)mmap(NULL, v4l2buf->length, PROT_READ | PROT_WRITE, MAP_SHARED, v4l2_fd, v4l2buf->m.offset);

    if (!mem->vaddr)
    {
      printf ("mmap v4lbuffer address failed\n");
      return -3;
    }

    if (ioctl(v4l2_fd, VIDIOC_QUERYBUF, v4l2buf) < 0)
    {
      printf ("VIDIOC_QUERYBUF for physical address failed.\n");
      return -4;
    }
    mem->paddr = (uint8_t *) v4l2buf->m.offset;
//    handle->buffer_pair[handle->allocated].paddr = memblk->paddr;
    allocated++;
    printf ("zty Allocated v4l2buffer(%p), index(%d), memblk(%p), vaddr(%p), paddr(%p), size(%d).!\n",
          v4l2buf, allocated - 1, mem, mem->vaddr, mem->paddr, mem->size);


    return 0;

}

#define TRY_TIMEOUT (500000) //500ms
#define TRY_INTERVAL (10000) //10ms
#define MAX_TRY_CNT (TRY_TIMEOUT/TRY_INTERVAL)

int V4L2::imx_v4l2_dequeue_v4l2memblk(PhyMemBlock **mem)
{
  struct v4l2_buffer v4l2buf;
  int trycnt = 0;


  memset (&v4l2buf, 0, sizeof(v4l2buf));
  v4l2buf.type = type;
  v4l2buf.memory = V4L2_MEMORY_MMAP;

  while (ioctl (v4l2_fd, VIDIOC_DQBUF, &v4l2buf) < 0) {
    trycnt ++;
    if(trycnt >= MAX_TRY_CNT) {
      printf ("Dequeue buffer from v4l2 device failed.!\n");
      return -1;
    }

    usleep (TRY_INTERVAL);
  }

  if (v4l2buf.field == V4L2_FIELD_INTERLACED)
  {

      v4l2buf.field = V4L2_FIELD_INTERLACED_TB;

  }

  /* set field info */
//  switch (v4l2buf.field) {
//    case V4L2_FIELD_NONE: *flags = GST_VIDEO_FRAME_FLAG_NONE; break;
//    case V4L2_FIELD_TOP: *flags =
//           GST_VIDEO_FRAME_FLAG_ONEFIELD | GST_VIDEO_FRAME_FLAG_TFF; break;
//    case V4L2_FIELD_BOTTOM: *flags = GST_VIDEO_FRAME_FLAG_ONEFIELD; break;
//    case V4L2_FIELD_INTERLACED_TB: *flags =
//           GST_VIDEO_FRAME_FLAG_INTERLACED | GST_VIDEO_FRAME_FLAG_TFF; break;
//    case V4L2_FIELD_INTERLACED_BT: *flags = GST_VIDEO_FRAME_FLAG_INTERLACED; break;
//    default: GST_WARNING("unknown field type"); break;
//  }
  v4l2buf.field = V4L2_FIELD_NONE;
  *mem = &memblk[v4l2buf.index];

//  printf ("deque v4l2buffer memblk (%p), paddr(%p), index (%d)",
//      *mem, (*mem)->paddr, v4l2buf.index);



//  GST_DEBUG ("deque v4l2buffer memblk (%p), index (%d), flags (%d)",
//      v4l2buf.index, handle->buffer_pair[v4l2buf.index].v4l2memblk, *flags);

  return 0;
}


int V4L2::get_next_display_buffer (SurfaceBuffer *buffer)
{

  PhyMemBlock *mem = NULL;
//  int index;
//  printf("zty next display buf %d!\n", first_request);
  if (first_request < DISPLAY_NUM_BUFFERS)
  {
    mem = &memblk[first_request];
    first_request ++;
  }
  else
  {
    if (imx_v4l2_dequeue_v4l2memblk (&mem) < 0)
    {
      printf ("get buffer from %s failed.!\n");
      return -1;
    }
  }

  if (!mem) {
    printf ("get display buffer failed.!\n");
    return -2;
  }

  memcpy (buffer, mem, sizeof (SurfaceBuffer));

//  printf("get display buffer, vaddr (%p) paddr (%p).!\n", buffer->vaddr, buffer->paddr);

  return 0;
}

v4l2_buffer * V4L2::imx_v4l2_find_buffer(PhyMemBlock *mem)
{
//  IMXV4l2Handle *handle = (IMXV4l2Handle*)v4l2handle;
  int i;

  for(i=0; i<DISPLAY_NUM_BUFFERS; i++)
  {
    if (memblk[i].paddr == mem->paddr)
      return &v4l2buffer[i];
  }

  printf ("Can't find the buffer 0x%08X.!\n", mem->paddr);
  return NULL;
}

int V4L2::imx_v4l2_do_queue_buffer(struct v4l2_buffer *v4l2buf)
{
    struct timeval queuetime;
    if (!v4l2buf)
    {
        printf ("queue buffer is NULL!!\n");
        return -1;
     }
    /*display immediately */
    gettimeofday (&queuetime, NULL);
    v4l2buf->timestamp = queuetime;

    if (ioctl (v4l2_fd, VIDIOC_QBUF, v4l2buf) < 0)
    {
      printf ("queue v4l2 buffer failed.!\n");
      return -1;
    }

    return 0;

}

int V4L2::imx_v4l2_queue_v4l2memblk (PhyMemBlock *mem)
{
//  IMXV4l2Handle *handle = (IMXV4l2Handle*)v4l2handle;
  struct v4l2_buffer *v4l2buf;
  int index;

  v4l2buf = imx_v4l2_find_buffer(mem);

  if (!v4l2buf)
    return -1;

  index = v4l2buf->index;

//  printf ("zty queue v4lbuffer memblk (%p), paddr(%p), index(%d).!\n",
//      mem, mem->paddr, index);

  v4l2buf->field = V4L2_FIELD_NONE;
//  if ((flags & GST_VIDEO_FRAME_FLAG_INTERLACED) && handle->do_deinterlace)
//  {
//    if (flags & GST_VIDEO_FRAME_FLAG_TFF)
//      v4l2buf->field = V4L2_FIELD_INTERLACED_TB;
//    else
//      v4l2buf->field = V4L2_FIELD_INTERLACED_BT;
//  }

//  if (flags & GST_VIDEO_FRAME_FLAG_ONEFIELD)
//  {
//    if (flags & GST_VIDEO_FRAME_FLAG_TFF)
//      v4l2buf->field = V4L2_FIELD_TOP;
//    else
//      v4l2buf->field = V4L2_FIELD_BOTTOM;
//  }

  memcpy(memblk+index, mem, sizeof(PhyMemBlock));


  if (!streamon)
  {
    int i;
//    GST_DEBUG ("streamon count (%d), queue count (%d)", handle->streamon_count, handle->queued_count);

    v4lbuf_queued_before_streamon[queued_count] = v4l2buf;
    queued_count ++;
    if (queued_count < streamon_count)
      return 0;

    for (i=0; i < streamon_count; i++)
    {
      if (imx_v4l2_do_queue_buffer (v4lbuf_queued_before_streamon[i]) < 0)
      {
//        handle->buffer_pair[handle->v4lbuf_queued_before_streamon[i]->index].v4l2memblk = NULL;
//        GST_ERROR ("queue buffers before streamon failed.");
        return -1;
      }
    }

    if (ioctl (v4l2_fd, VIDIOC_STREAMON, &type) < 0)
    {
      printf ("Stream on V4L2 device failed.\n");
      return -1;
    }
    streamon = true;
//    GST_DEBUG ("V4L2 device is STREAMON.");
    return 0;
  }

  if (imx_v4l2_do_queue_buffer (v4l2buf) < 0)
  {
//    handle->buffer_pair[v4l2buf->index].v4l2memblk = NULL;
    return -1;
  }

  queued_count ++;

//  GST_DEBUG ("queued (%d)\n", handle->queued_count);

  return 0;
}

void V4L2::imx_v4l2_free_buffer(void)
{
    int i;

    if(v4l2_fd > 0)
    {
        ioctl(v4l2_fd, VIDIOC_STREAMOFF, &type);

        for(i = 0; i < allocated; i++)
        {
            munmap(memblk[i].vaddr, memblk[i].size);
        }
        allocated = 0;
        close(v4l2_fd);
    }
    v4l2_fd = 0;
}

int V4L2::v4l2_display_init(string devname)
{
    IMXV4l2Rect rect;
    struct v4l2_crop crop;
    uint32_t fmt;

    int i;
    printf("zty v4l2 display init!\n");
    dev = new DisplayDev(devname);

    dev->get_display_resolution((char *)devname.c_str());

    v4l2_open_display_dev(V4L2_BUF_TYPE_VIDEO_OUTPUT);
    v4l2out_config_alpha(0);
    imx_ipu_v4l2_config_colorkey(false, 0);

    rect.left = rect.top = 0;
    rect.width = dev->width;
    rect.height = dev->height;
    fmt = string_to_fmt("RGBP");
    fmt = display_fmt_to_v4l2_fmt(fmt);
    if(fmt == 0)
    {
        printf("Unsupported display format, check the display config file.!\n");
        return -1;
    }

    if(imx_v4l2out_config_input(fmt, &rect) < 0)
    {
        printf("imx v4l2 out config input error!\n");
        return -2;
    }

    memset(&crop, 0x00, sizeof(crop));
    crop.c.width = dev->width;
    crop.c.height = dev->height;

    if(v4l2out_config_output(&crop) < 0)
    {
        return -3;
    }

    if (imx_v4l2_set_buffer_count (DISPLAY_NUM_BUFFERS, V4L2_MEMORY_MMAP) < 0)
    {
        printf ("zty configure v4l2 device  buffer count failed.!\n");
        return -4;
    }


    for(i = 0; i < DISPLAY_NUM_BUFFERS; i++)
    {
        if(imx_v4l2_allocate_buffer(&memblk[i]) < 0)
        {
            printf("allocate memory from v4l2 device failed.!\n");
            return -4;
        }
    }

    g2d = new G2dDevice;

    fmt = string_to_fmt("RGBP");
    g2d->g2d_device_open(fmt, 1920, 1080);


    return 0;
}



















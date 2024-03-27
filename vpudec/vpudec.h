#ifndef VPUDEC_H
#define VPUDEC_H

#ifdef ARM
#include <stdint.h>
#include <stdlib.h>
#include "imx-mm/vpu/vpu_wrapper.h"
#include "zprint/zprint.h"
#include <list>
#include "g2d.h"
#include "phymem.h"
#include "v4l2/v4l2.h"

using namespace std;

#define PAGE_ALIGN(x) (((x) + 4095) & ~4095)
#define ALIGN(ptr,align)	((align) ? ((((uint32_t)(ptr))+(align)-1)/(align)*(align)) : ((uint32_t)(ptr)))


#define VPU_MEM_ALIGN			0x8
#define MemNotAlign(mem,align)	((((unsigned int)mem)%(align))!=0)

//typedef struct {
//  uint8_t *vaddr;
//  uint8_t *paddr;
//  uint8_t *caddr;
//  uint32_t size;
//  void *user_data;
//} PhyMemBlock;

typedef enum {
  STATE_NULL    = 0,
  STATE_LOADED,
  STATE_ALLOCATED_INTERNAL_BUFFER,
  STATE_OPENED,
  STATE_REGISTRIED_FRAME_BUFFER,
  STATE_MAX
} VpuDecState;

typedef struct {
  VpuMemInfo mem_info;
//    list internal_virt_mem;
//    list internal_phy_mem;
} VpuInternalMem;

#define G2D_BLOCK_BUF 12
#define FRAME_BUF_SIZE 12
class G2dDISPLAY
{
public:
    G2dDISPLAY();
    ~G2dDISPLAY();
public:
    PhyMemBlock block[G2D_BLOCK_BUF];
    int alloc_g2d_mem(void);
};

typedef  struct{
    int wr;
    int rd;
    int sz;
    VpuFrameBuffer* buf[FRAME_BUF_SIZE];
} VPUBUFPOINT;


class VpuDec
{
public:
    VpuDec();
    ~VpuDec();
    int vpu_init(void);
    int vpu_open(void);
    int vpu_open(VpuCodStd type);
    int vpu_close(void);
    int vpu_mem_init(void);
//    int vpu_decode_process(uint8_t * data, int size, uint8_t *ext, int extsize, int * okmark);
    int vpu_decode_process(uint8_t * data, int size);
    int vpu_dec_object_handle_reconfig(void);
    int set_vpu_codec_data(unsigned char * buf, unsigned int size);
    int vpu_framebuffer_alloc(void);
    int vpu_register_frame_buf(void);
    int vpu_dec_data_output(void);
    int vpu_get_src_info(VpuDecOutFrameInfo * frame_info, SurfaceBuffer * srcbuf);
    int vpu_add_used_frame(VpuFrameBuffer* used);
    VpuFrameBuffer* vpu_get_used_frame(void);
    int vpu_release_frame(void);

public:
    VpuDecState state;
    VpuInternalMem vpu_internal_mem;
    VpuMemDesc     internal_mem_info;
    VpuFrameBuffer framebuffer[FRAME_BUF_SIZE];
    VpuMemDesc     frameinfo[FRAME_BUF_SIZE];
    VpuDecHandle handle;
    PhyMemBlock  mem_p;
    VpuDecInitInfo init_info;
    VpuCodecData  vpu_data;
    uint8_t    * virt_ptr;
    G2dDISPLAY * display;
    V4L2       * v4l2;
    uint32_t     framenum;
    VPUBUFPOINT  usedbuf;

};
#endif //ARM
#endif // VPUDEC_H

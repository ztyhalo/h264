#ifdef ARM
#include "vpudec.h"

G2dDISPLAY::G2dDISPLAY()
{
    memset(block, 0x00,sizeof(block));
}

G2dDISPLAY::~G2dDISPLAY()
{
    int i ;
    struct g2d_buf * pbuf;
    for(i = 0; i < G2D_BLOCK_BUF; i++)
    {
        if(block[i].user_data != NULL)
        {
            pbuf = (struct g2d_buf*) block[i].user_data;
            g2d_free (pbuf);
        }
    }
}

int G2dDISPLAY::alloc_g2d_mem(void)
{
    struct g2d_buf *pbuf = NULL;
    int i;
    for(i = 0; i < G2D_BLOCK_BUF; i++)
    {
        block[i].size = PAGE_ALIGN(3133440);
        pbuf = g2d_alloc(block[i].size, 0);
          if (!pbuf) {
            printf ("g2d_alloc failed.");
            return -1;
          }

          block[i].vaddr = (uint8_t *) pbuf->buf_vaddr;
          block[i].paddr = (uint8_t *) pbuf->buf_paddr;
          block[i].user_data = (void *) pbuf;
        printf("zty mem_block phy memblk size %d vaddr 0x%x paddr 0x%x!\n", block[i].size, block[i].vaddr,  block[i].paddr);

    }


    return 0;
}


VpuDec::VpuDec()
{
    state = STATE_NULL;
    framenum = 0;
    memset(frameinfo, 0x00, sizeof(frameinfo));
    virt_ptr = NULL;
    memset(&internal_mem_info, 0x00, sizeof(internal_mem_info));
    v4l2 = NULL;
    display = NULL;
    memset(&usedbuf, 0, sizeof(usedbuf));

}
VpuDec::~VpuDec()
{
    int i;
    VpuDecRetCode ret;

    for(i = 0; i < FRAME_BUF_SIZE; i++)
    {
        if(frameinfo[i].nSize > 0)
        {
            ret = VPU_DecFreeMem(&frameinfo[i]);
            if(ret != VPU_DEC_RET_SUCCESS)
            {
                printf("zty vpu dec freemem fail!\n");
            }
            frameinfo[i].nSize = 0;
        }
    }
    if(virt_ptr != NULL)
    {
        free(virt_ptr);
        virt_ptr = NULL;
    }
    if(internal_mem_info.nSize > 0)
    {
        ret = VPU_DecFreeMem(&internal_mem_info);
        if(ret != VPU_DEC_RET_SUCCESS)
        {
            printf("zty vpu dec free internal mem fail!\n");
        }
        internal_mem_info.nSize = 0;
    }
    if(v4l2 != NULL)
    {
        delete v4l2;
        v4l2 = NULL;
    }
    if(display != NULL)
    {
        delete display;
        display = NULL;
    }
}


static char const *
gst_vpu_dec_object_strerror(VpuDecRetCode code)
{
  switch (code) {
    case VPU_DEC_RET_SUCCESS: return "success";
    case VPU_DEC_RET_FAILURE: return "failure";
    case VPU_DEC_RET_INVALID_PARAM: return "invalid param";
    case VPU_DEC_RET_INVALID_HANDLE: return "invalid handle";
    case VPU_DEC_RET_INVALID_FRAME_BUFFER: return "invalid frame buffer";
    case VPU_DEC_RET_INSUFFICIENT_FRAME_BUFFERS: return "insufficient frame buffers";
    case VPU_DEC_RET_INVALID_STRIDE: return "invalid stride";
    case VPU_DEC_RET_WRONG_CALL_SEQUENCE: return "wrong call sequence";
    case VPU_DEC_RET_FAILURE_TIMEOUT: return "failure timeout";
    default: return NULL;
  }
}

int VpuDec::vpu_framebuffer_alloc(void)
{
    int i;
    VpuDecRetCode ret;
    VpuMemDesc mem_desc;
    memset(framebuffer, 0x00, sizeof(framebuffer));
    for(i = 0; i < FRAME_BUF_SIZE; i++)
    {
        memset(&mem_desc, 0, sizeof(VpuMemDesc));
        mem_desc.nSize = PAGE_ALIGN(524288); //524288
        ret = VPU_DecGetMem(&mem_desc);
        if (ret == VPU_DEC_RET_SUCCESS)
        {
//             frameinfo[i] = mem_desc;
             memcpy(frameinfo+i, &mem_desc, sizeof(mem_desc));
             framebuffer[i].pbufMvCol = (unsigned char* )mem_desc.nPhyAddr;
             framebuffer[i].pbufVirtMvCol = (unsigned char* )mem_desc.nVirtAddr;
             printf("zty vpu allocator malloc size %d paddr: 0x%x vaddr: 0x%x!\n", mem_desc, mem_desc.nPhyAddr, mem_desc.nVirtAddr);
        }
        else
        {
            printf("vpu framebuff alloc fail!\n");
            return -1;
        }
    }
    return 0;

}


int VpuDec::vpu_register_frame_buf(void)
{
    int i;
    VpuDecRetCode dec_ret;
    VpuFrameBuffer *vpu_frame;

    display = new G2dDISPLAY;
    display->alloc_g2d_mem();

    vpu_framebuffer_alloc();

   for(i = 0; i < FRAME_BUF_SIZE; i++)
   {
       vpu_frame = &(framebuffer[i]);
       vpu_frame->nStrideY = 1920;
       vpu_frame->nStrideC = 1920;
       vpu_frame->pbufY  = display->block[i].paddr;
       vpu_frame->pbufCb = vpu_frame->pbufY + (1920 * 1088); //info
       vpu_frame->pbufCr = vpu_frame->pbufCb + 1;
       vpu_frame->pbufVirtY = display->block[i].vaddr;
       vpu_frame->pbufVirtCb = vpu_frame->pbufVirtY + (1920 * 1088);
       vpu_frame->pbufVirtCr = vpu_frame->pbufVirtCb + 1;


   }

   dec_ret = VPU_DecRegisterFrameBuffer (handle, framebuffer, FRAME_BUF_SIZE);
   if (dec_ret != VPU_DEC_RET_SUCCESS)
   {
       printf("zty registering framebuffers failed: %s", gst_vpu_dec_object_strerror(dec_ret));
       return -1;
   }
   return 0;

}

int VpuDec::vpu_mem_init(void)
{
    int i;
    uint32_t size;
//    uint8_t * ptr;
    VpuDecRetCode ret;
    VpuMemDesc mem_desc;

    for (i = 0; i < vpu_internal_mem.mem_info.nSubBlockNum; ++i)
    {
        size = vpu_internal_mem.mem_info.MemSubBlock[i].nAlignment \
           + vpu_internal_mem.mem_info.MemSubBlock[i].nSize;

        printf("zty sub block %d  type: %s  size: %d!\n", i, \
        (vpu_internal_mem.mem_info.MemSubBlock[i].MemType == VPU_MEM_VIRT) ? \
        "virtual" : "phys", size);

        printf("zty sub block %d  pVirtAddr: 0x%x  pPhyAddr: 0x%x!\n", i, \
        (vpu_internal_mem.mem_info.MemSubBlock[i].pVirtAddr ) , vpu_internal_mem.mem_info.MemSubBlock[i].pPhyAddr);

        if(vpu_internal_mem.mem_info.MemSubBlock[i].MemType == VPU_MEM_VIRT)
        {
            virt_ptr = (uint8_t *)malloc(size);
            if (virt_ptr == NULL)
            {
                printf ("Could not allocate memory");
                return -1;
            }

            vpu_internal_mem.mem_info.MemSubBlock[i].pVirtAddr = (unsigned char*)ALIGN(
                         virt_ptr, vpu_internal_mem.mem_info.MemSubBlock[i].nAlignment);
        }
        else if (vpu_internal_mem.mem_info.MemSubBlock[i].MemType == VPU_MEM_PHY)
        {
            memset(&mem_desc, 0, sizeof(VpuMemDesc));
            mem_desc.nSize = PAGE_ALIGN(size);
            printf("zty internal alloc size %d!\n", mem_desc.nSize);
            ret = VPU_DecGetMem(&mem_desc);
            if (ret == VPU_DEC_RET_SUCCESS)
            {
                    internal_mem_info = mem_desc;
                    mem_p.size         = mem_desc.nSize;
                    mem_p.paddr        = (uint8_t *)(mem_desc.nPhyAddr);
                    mem_p.vaddr         = (uint8_t *)(mem_desc.nVirtAddr);
                    mem_p.caddr         = (uint8_t *)(mem_desc.nCpuAddr);
             }
            else
            {
                    return -1;
            }
            vpu_internal_mem.mem_info.MemSubBlock[i].pVirtAddr = mem_p.vaddr;
            vpu_internal_mem.mem_info.MemSubBlock[i].pPhyAddr = mem_p.paddr;

        }
        else
        {
            printf ("sub block %d type is unknown - skipping", i);
        }
    }
    state = STATE_ALLOCATED_INTERNAL_BUFFER;
    return 0;
}

int VpuDec::vpu_init(void)
{
    VpuDecRetCode ret;
    VpuVersionInfo version;
    VpuWrapperVersionInfo wrapper_version;
    int i;

    v4l2 = new V4L2;

    v4l2->v4l2_display_init("/dev/video17");

    ret = VPU_DecLoad();
    if(ret != VPU_DEC_RET_SUCCESS){
        zprintf1("VPU_DecLoad fail: %s!\n", gst_vpu_dec_object_strerror(ret));
        return -1;
    }
    state = STATE_LOADED;

    ret = VPU_DecGetVersionInfo(&version);
    if (ret != VPU_DEC_RET_SUCCESS) {
      zprintf1("VPU_DecGetVersionInfo fail: %s!\n", \
          gst_vpu_dec_object_strerror(ret));
    }

    ret = VPU_DecGetWrapperVersionInfo(&wrapper_version);
    if (ret != VPU_DEC_RET_SUCCESS) {
      zprintf1("VPU_DecGetWrapperVersionInfo fail: %s!\n", \
          gst_vpu_dec_object_strerror(ret));
    }

    printf("====== VPUDEC: build on %s %s. ======\n",__DATE__,__TIME__);;
    printf("\twrapper: %d.%d.%d (%s)\n", wrapper_version.nMajor, wrapper_version.nMinor,
      wrapper_version.nRelease, (wrapper_version.pBinary? wrapper_version.pBinary:"unknow"));
    printf("\tvpulib: %d.%d.%d\n", version.nLibMajor, version.nLibMinor, version.nLibRelease);
    printf("\tfirmware: %d.%d.%d.%d\n", version.nFwMajor, version.nFwMinor, version.nFwRelease, version.nFwCode);

    //allocate memory
    memset(&(vpu_internal_mem.mem_info), 0, sizeof(VpuMemInfo));

    ret = VPU_DecQueryMem(&(vpu_internal_mem.mem_info));
    if (ret != VPU_DEC_RET_SUCCESS) {
      zprintf1("could not get VPU memory information: %s!\n", \
          gst_vpu_dec_object_strerror(ret));
      return -2;
    }

    printf("vpu allocation internal mem %d!\n", vpu_internal_mem.mem_info.nSubBlockNum);

    vpu_mem_init();



    return 0;
}

static void printf_init_paramer(VpuDecOpenParam * param)
{

  printf("zty vpu param CodecFormat %d !\n", param->CodecFormat);
  printf("zty vpu param nReorderEnable %d !\n", param->nReorderEnable);
  printf("zty vpu param nChromaInterleave %d !\n", param->nChromaInterleave);
  printf("zty vpu param nMapType %d !\n", param->nMapType);
  printf("zty vpu param nTiled2LinearEnable %d !\n", param->nTiled2LinearEnable);
  printf("zty vpu param nPicWidth %d !\n", param->nPicWidth);
  printf("zty vpu param nPicHeight %d !\n", param->nPicHeight);
  printf("zty vpu param nEnableFileMode %d !\n", param->nEnableFileMode);
  printf("zty vpu param nReserved %d !\n", param->nReserved[0]);
  printf("zty vpu param nReserved %d !\n", param->nReserved[1]);
  printf("zty vpu param nReserved %d !\n", param->nReserved[2]);
  printf("zty vpu param pAppCxt 0x%x !\n", param->pAppCxt);

}


static void printf_init_mem(VpuMemInfo * mem)
{

    VpuMemSubBlockInfo * pMemPhy;
    pMemPhy=&mem->MemSubBlock[1];
    int i;
    for (i = 0; i < mem->nSubBlockNum; ++i)
    {
        printf("zty sub block %d  type: %s  size: %d!\n", i, \
        (mem->MemSubBlock[i].MemType == VPU_MEM_VIRT) ? \
        "virtual" : "phys", mem->MemSubBlock[i].nSize);
    }

    if ((pMemPhy->pVirtAddr==NULL) || MemNotAlign(pMemPhy->pVirtAddr,VPU_MEM_ALIGN)
        ||(pMemPhy->pPhyAddr==NULL) || MemNotAlign(pMemPhy->pPhyAddr,VPU_MEM_ALIGN)
        )
    {
        printf("%s: failure: invalid parameter !! \r\n",__FUNCTION__);

    }
}

int VpuDec::vpu_open(void)
{
    VpuDecRetCode ret;
    VpuDecOpenParam open_param;
    int config_param;
    int capability=0;

    memset(&open_param, 0, sizeof(open_param));

    open_param.CodecFormat = VPU_V_AVC;

    open_param.nMapType = 0;
    open_param.nTiled2LinearEnable = 0;

    open_param.nReorderEnable = 1;
    open_param.nEnableFileMode = 0;

    open_param.nChromaInterleave =1;

    open_param.nPicWidth = 0;
    open_param.nPicHeight = 0;

    printf_init_paramer(&open_param);
    printf_init_mem(&(vpu_internal_mem.mem_info));
    ret = VPU_DecOpen(&(handle), &open_param, &(vpu_internal_mem.mem_info));
    if (ret != VPU_DEC_RET_SUCCESS)
    {
      zprintf1("opening new VPU handle failed: %s!\n",  gst_vpu_dec_object_strerror(ret));
      return -1;
    }
    ret=VPU_DecGetCapability(handle, VPU_DEC_CAP_FRAMESIZE, &capability);
    if((ret==VPU_DEC_RET_SUCCESS)&&capability)
    {
//       vpu_dec_object->use_new_tsm = TRUE;
        printf("vpu get cap ok!\n");
    }

    config_param = VPU_DEC_SKIPNONE;
      ret = VPU_DecConfig(handle, VPU_DEC_CONF_SKIPMODE, &config_param);
      if (ret != VPU_DEC_RET_SUCCESS)
      {
          zprintf1("could not configure skip mode: %s!\n", gst_vpu_dec_object_strerror(ret));
          printf("zty could not configure skip mode!\n");
          return -2;
      }

      config_param = 0;
      ret = VPU_DecConfig(handle, VPU_DEC_CONF_BUFDELAY, &config_param);
      if (ret != VPU_DEC_RET_SUCCESS)
      {
          zprintf1("could not configure buffer delay: %s!\n", gst_vpu_dec_object_strerror(ret));
          printf("zty could not config delay!\n");
          return -3;
      }

    state = STATE_OPENED;


    return 0;
}

void printf_init_info(VpuDecInitInfo * info)
{
  printf("zty vpu init info nPicWidth %d nPicHeight %d nFrameRateRes %d nFrameRateDiv %d!\n", info->nPicWidth, \
                                        info->nPicHeight , info->nFrameRateRes, info->nFrameRateDiv);
  printf("zty vpu init info piccropret nLeft %d nTop %d nRight %d nBottom %d!\n", info->PicCropRect.nLeft, \
                                        info->PicCropRect.nTop , info->PicCropRect.nRight, info->PicCropRect.nBottom);

  printf("zty vpu init info nMinFrameBufferCount %d nMjpgSourceFormat %d nInterlace %d nQ16ShiftWidthDivHeightRatio %d!\n", info->nMinFrameBufferCount, \
                info->nMjpgSourceFormat , info->nInterlace, info->nQ16ShiftWidthDivHeightRatio);
  printf("zty vpu init info nConsumedByte %d nAddressAlignment %d !\n", info->nConsumedByte, \
                info->nAddressAlignment );
}

int VpuDec::vpu_dec_object_handle_reconfig(void)
{
  VpuDecRetCode dec_ret;
//  GstVideoCodecState *state;
//  GstVideoFormat fmt;
//  gint height_align;
//  GstBuffer *buffer;
//  guint i;

  dec_ret = VPU_DecGetInitialInfo(handle, &(init_info));
  if (dec_ret != VPU_DEC_RET_SUCCESS)
  {
    printf("could not get init info: %s", gst_vpu_dec_object_strerror(dec_ret));
    return -1;
  }

  printf_init_info(&(init_info));

  if (init_info.nPicWidth <= 0 || init_info.nPicHeight <= 0)
  {
    printf("VPU get init info error.");
    return -2;
  }

  vpu_register_frame_buf();

  return 0;
}

int VpuDec::set_vpu_codec_data(unsigned char * buf, unsigned int size)
{
    vpu_data.nSize = size;
    vpu_data.pData = buf;
    return 0;
}

int VpuDec::vpu_get_src_info(VpuDecOutFrameInfo * frame_info, SurfaceBuffer * srcbuf)
{
    int i ;
//    int mark = 0;
    for(i = 0; i < FRAME_BUF_SIZE; i++)
    {
        if(frame_info->pDisplayFrameBuf->pbufVirtY == display->block[i].vaddr)
        {
            memcpy(srcbuf, &display->block[i], sizeof(SurfaceBuffer));
            return i;
        }
    }
    return -1;

}

int VpuDec::vpu_add_used_frame(VpuFrameBuffer* used)
{
    if(usedbuf.sz >=  FRAME_BUF_SIZE)
    {
        printf("zty add used frame error!\n");
        return -1;
    }
    if(used == NULL)
        return -2;

    usedbuf.buf[usedbuf.wr] = used;
    usedbuf.sz++;
    usedbuf.wr++;
    usedbuf.wr %= FRAME_BUF_SIZE;
    return 0;
}
VpuFrameBuffer* VpuDec::vpu_get_used_frame(void)
{
    VpuFrameBuffer* ret = NULL;
    if(usedbuf.sz > 0)
    {
        ret = usedbuf.buf[usedbuf.rd];
        usedbuf.sz--;
        usedbuf.rd++;
        usedbuf.rd %= FRAME_BUF_SIZE;
    }
    return ret;
}

int VpuDec::vpu_release_frame(void)
{
    VpuFrameBuffer * rease;
    VpuDecRetCode dec_ret;

    rease = vpu_get_used_frame();
    if(rease == NULL)
    {
        printf("get used frame error!\n");
        return -1;
    }
//    printf("zty get_gst_buffer phy memblk vaddr 0x%x!\n", rease->pbufVirtY);
//    printf("zty get_gst_buffer phy memblk vaddr!\n");
    dec_ret = VPU_DecOutFrameDisplayed(handle, rease);
    if (dec_ret != VPU_DEC_RET_SUCCESS)
    {
      printf("zty receive frame %d clearing display failed: %s!\n",framenum,  gst_vpu_dec_object_strerror(dec_ret));
      return -2;
    }
    return 0;
}

int VpuDec::vpu_dec_data_output(void)
{
    VpuDecRetCode dec_ret;
    VpuDecOutFrameInfo out_frame_info;
    SurfaceBuffer  dstbuf;
    SurfaceBuffer  srcbuf;
    int index ;
//    VpuFrameBuffer * frame_buffer;


    framenum++;
    dec_ret = VPU_DecGetOutputFrame(handle, &out_frame_info);
    if (dec_ret != VPU_DEC_RET_SUCCESS)
    {
      printf("zty could not get decoded output frame: %s", gst_vpu_dec_object_strerror(dec_ret));
      return -1;
    }
//    printf("zty vpu display buffer: 0x%x pbufVirtY: 0x%x\n", out_frame_info.pDisplayFrameBuf, out_frame_info.pDisplayFrameBuf->pbufVirtY);
    index = vpu_get_src_info(&out_frame_info, &srcbuf);
    if(index < 0)
    {
        printf("zty get output src info error!\n");
        return -1;
    }

//    printf("zty get_src_info: 0x%x pbufVirtY: 0x%x\n", srcbuf.paddr, srcbuf.vaddr);
    if(v4l2->get_next_display_buffer(&dstbuf) != 0)
    {
        printf("zty get next buf error!\n");
        return -2;
    }
//    printf("zty dstbuf vaddr 0x%x!\n", dstbuf.vaddr);
    if(v4l2->g2d->g2d_device_blit_surface(&srcbuf ,&dstbuf) != 0)
    {
        printf("zty g2d device blit  fail!\n");
        return -3;
    }
    if(v4l2->imx_v4l2_queue_v4l2memblk(&dstbuf) !=0)
    {
        printf("zty imx_v4l2_queue_v4l2memblk  fail!\n");
        return -4;
    }
////    frame_buffer = &framebuffer[index];
////    printf("zty frame buffer 0x%x!\n", frame_buffer);
//    dec_ret = VPU_DecOutFrameDisplayed(handle, out_frame_info.pDisplayFrameBuf);
//    if (dec_ret != VPU_DEC_RET_SUCCESS)
//    {
//      printf("zty receive frame %d clearing display framebuffer %d failed: %s!\n",framenum, index, gst_vpu_dec_object_strerror(dec_ret));
//      return -5;
//    }
      if(vpu_add_used_frame(out_frame_info.pDisplayFrameBuf) != 0)
      {
          printf("zty add used frame error!\n");
      }
    return 0;
}

static int testMark = 0;
static int savemark = 0;
static uint8_t * saveadd = 0;

int VpuDec::vpu_decode_process(uint8_t * data, int size, uint8_t *ext, int extsize, int * okmark)
{
    VpuBufferNode in_data;
    int buf_ret;
    VpuDecRetCode dec_ret;
    int ret = 0;
    int frameok = 0;



    memset(&in_data, 0, sizeof(in_data));
    in_data.nSize = size;
    in_data.pPhyAddr = NULL;
    in_data.pVirAddr = data;
    in_data.sCodecData.pData = vpu_data.pData;
    in_data.sCodecData.nSize = vpu_data.nSize;

//    printf("zty size %d framenum %d!\n", size, framenum);
//    if(framenum == 121)
//    {
//        in_data.nSize = extsize;
//        in_data.pVirAddr = ext;
//    }
//     printf("zty size\n");
//    if(size > 80000 && savemark == 0)
//    {
//        saveadd = data;
//        frameok = 1;
//        savemark = 1;
//    }
    while(1)
    {
//        if(framenum >12)
//            return 0;
        dec_ret = VPU_DecDecodeBuf(handle, &in_data, &buf_ret);
        if (dec_ret != VPU_DEC_RET_SUCCESS)
        {
            int i, j;
             printf("failed to decode frame: %s", \
                 gst_vpu_dec_object_strerror(dec_ret));
             printf("indaa.nsize %d process %d!\n", in_data.nSize, framenum);

             printf("zty in_data.sCodecData.nSize %d!\n", in_data.sCodecData.nSize);
             for(i = 0; i < in_data.sCodecData.nSize; i++)
             {
               printf("0x%x ", in_data.sCodecData.pData[i]);
             }
             printf("\n");
//             j= 0;
             for(i = 0; i < 38; i++)
             {
                 printf("0x%x ", in_data.pVirAddr[i]);
//                 if(j >15)
//                 {
//                     printf("\n");
//                     j = 0;
//                 }
//                 j++;
             }
             printf("\n");
//             printf("error end!\n");
             VPU_DecReset(handle);
             return -1;
        }

//        printf("zty vpu decodebuf 0x%x!\n", buf_ret);

        if (buf_ret & VPU_DEC_INIT_OK || buf_ret & VPU_DEC_RESOLUTION_CHANGED)
        {
          if (buf_ret & VPU_DEC_RESOLUTION_CHANGED)
          {
              printf("zty resolution change!\n");
          }
          ret = vpu_dec_object_handle_reconfig();
          if (ret != 0)
          {
            printf("gst_vpu_dec_object_handle_reconfig fail: %d\n", ret);
            return ret;
          }
        }

        if (buf_ret & VPU_DEC_OUTPUT_DIS)
        {
//            int i;
//            printf("zty VPU_DEC_OUTPUT_DIS!\n");

//            for(i = 0; i < 38; i++)
//            {
//                printf("0x%x ", saveadd[i]);
//            }
//            printf("\n");
            testMark =0;
            ret = vpu_dec_data_output ();
            if(ret != 0)
                return ret;
            *okmark= 38;
        }

        if (buf_ret & VPU_DEC_NO_ENOUGH_BUF)
        {
//           printf("zty VPU_DEC_NO_ENOUGH_BUF!\n");
           ret = vpu_release_frame();
           if(ret != 0)
           {
               printf("vpu release frame error!\n");
               return ret;
           }

        }

        if (buf_ret & VPU_DEC_OUTPUT_MOSAIC_DIS)
        {
           printf("zty VPU_DEC_OUTPUT_MOSAIC_DIS!\n");
        }

        if (buf_ret & VPU_DEC_FLUSH)
        {
          printf("zty VPU_DEC_FLUSH!\n");
        }

        if (buf_ret & VPU_DEC_OUTPUT_DROPPED || buf_ret & VPU_DEC_SKIP || buf_ret & VPU_DEC_OUTPUT_REPEAT)
        {
           printf("zty VPU_DEC_OUTPUT_DROPPED!\n");
        }

        if (buf_ret & VPU_DEC_OUTPUT_EOS)
        {
           printf("zty VPU_DEC_OUTPUT_EOS!\n");
          break;
        }

        if (buf_ret & VPU_DEC_NO_ENOUGH_INBUF)
        {
//            printf("zty VPU_DEC_NO_ENOUGH_INBUF!\n");
//          testMark++;
//          if(testMark >= 2)
//          {
//               printf("zty VPU_DEC_NO_ENOUGH_INBUF!\n");
//          }
          break;
        }

        if ((buf_ret & VPU_DEC_INPUT_USED))
        {
            testMark = 0;
//            printf("zty VPU_DEC_INPUT_USED!\n");
            if (in_data.nSize)
             {
//              printf("zty in data size 0!\n");
              in_data.nSize = 0;
              in_data.pVirAddr = 0;
             }
         }

    }
    return ret;

}

#endif //ARM


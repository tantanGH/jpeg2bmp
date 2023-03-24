#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "himem.h"
#include "picojpeg.h"
#include "jpeg_decode.h"

#define GVRAM ((uint16_t*)0xC00000)

//
//  picojpeg callback function
//
static uint8_t pjpeg_need_bytes_callback(uint8_t* pBuf, uint8_t buf_size, uint8_t* pBytes_actually_read, void* pCallback_data) {

  uint32_t* jpeg_buffer_meta = (uint32_t*)pCallback_data;
  uint8_t* jpeg_buffer = (uint8_t*)(jpeg_buffer_meta[0]);
  uint32_t jpeg_buffer_bytes = jpeg_buffer_meta[1];
  uint32_t jpeg_buffer_ofs = jpeg_buffer_meta[2];

  uint32_t remain = jpeg_buffer_bytes - jpeg_buffer_ofs;
  uint32_t n = buf_size < remain ? buf_size : remain;  
  if (n > 0) memcpy(pBuf, jpeg_buffer + jpeg_buffer_ofs, n);

  *pBytes_actually_read = (unsigned char)n;
  jpeg_buffer_meta[2] += n;

  return 0;
}

//
//  init JPEG decode handler
//
int32_t jpeg_decode_init(JPEG_DECODE_HANDLE* jpeg) {

  int32_t rc = -1;

  jpeg->bmp = NULL;

  jpeg->rgb555_r = (uint16_t*)himem_malloc(sizeof(uint16_t) * 256, 0);
  jpeg->rgb555_g = (uint16_t*)himem_malloc(sizeof(uint16_t) * 256, 0);
  jpeg->rgb555_b = (uint16_t*)himem_malloc(sizeof(uint16_t) * 256, 0);

  if (jpeg->rgb555_r == NULL || jpeg->rgb555_g == NULL || jpeg->rgb555_b == NULL) goto exit;

  for (int16_t i = 0; i < 256; i++) {
    uint16_t c = (uint16_t)(i / 8);
    jpeg->rgb555_r[i] = (uint16_t)((c <<  6) + 1);
    jpeg->rgb555_g[i] = (uint16_t)((c << 11) + 1);
    jpeg->rgb555_b[i] = (uint16_t)((c <<  1) + 1);
  }

  rc = 0;

exit:
  return rc;
}

//
//  set BMP encode handler
//
void jpeg_decode_set_bmp_encoder(JPEG_DECODE_HANDLE* jpeg, BMP_ENCODE_HANDLE* bmp) {
  if (jpeg != NULL) {
    jpeg->bmp = bmp;
  }
}

//
//  close JPEG decode handler
//
void jpeg_decode_close(JPEG_DECODE_HANDLE* jpeg) {
  if (jpeg->rgb555_r != NULL) {
    himem_free(jpeg->rgb555_r, 0);
    jpeg->rgb555_r = NULL;
  }
  if (jpeg->rgb555_g != NULL) {
    himem_free(jpeg->rgb555_g, 0);
    jpeg->rgb555_g = NULL;
  }
  if (jpeg->rgb555_b != NULL) {
    himem_free(jpeg->rgb555_b, 0);
    jpeg->rgb555_b = NULL;
  }   
}

//
//  decode
//
int32_t jpeg_decode_exec(JPEG_DECODE_HANDLE* jpeg, uint8_t* jpeg_buffer, size_t jpeg_buffer_bytes) {

  uint32_t jpeg_buffer_meta[3];
  jpeg_buffer_meta[0] = (uint32_t)jpeg_buffer;
  jpeg_buffer_meta[1] = jpeg_buffer_bytes;
  jpeg_buffer_meta[2] = 0;

  pjpeg_image_info_t image_info = { 0 };
  int32_t rc = pjpeg_decode_init(&image_info, pjpeg_need_bytes_callback, jpeg_buffer_meta, 0);
  if (rc != 0) {
    goto exit;
  }

  // write BMP header
  bmp_encode_write_header(jpeg->bmp, image_info.m_width, image_info.m_height);

  int32_t mcu_y = 0;
  int32_t mcu_x = 0;

  // image_info.m_width ... total image width pixels
  // image_info.m_comps ... bytes per pixel
  int32_t row_pitch = image_info.m_width * image_info.m_comps;
//  printf("m_width=%d, m_comps=%d, m_MCUWidth=%d, m_MCUHeight=%d, row_pitch=%d\n",
//          image_info.m_width, image_info.m_comps, 
//          image_info.m_MCUWidth, image_info.m_MCUHeight, 
//          row_pitch);

  /// image_info.m_MCUHeight ... block height pixels
  uint8_t* bmp_buffer = himem_malloc(row_pitch * image_info.m_MCUHeight, 0);
  size_t total_lines = 0;

  for (;;) {

    uint8_t status = pjpeg_decode_mcu();
    if (status == PJPG_NO_MORE_BLOCKS) {
      break;
    } else if (status != 0) {
      rc = status;
      goto exit;
    }
    if (mcu_y >= image_info.m_MCUSPerCol) {
      goto exit;
    }

    uint8_t* buf_row = bmp_buffer + (mcu_x * image_info.m_MCUWidth * image_info.m_comps);
    size_t buf_lines = 0;

    for (int32_t y = 0; y < image_info.m_MCUHeight; y += 8) {

      const int by_limit = min(8, image_info.m_height - (mcu_y * image_info.m_MCUHeight + y));
      buf_lines += by_limit;

      for (int32_t x = 0; x < image_info.m_MCUWidth; x += 8) {

        uint8_t *buf_block = buf_row + x * image_info.m_comps;
        
        uint8_t src_ofs = (x * 8U) + (y * 16U);
        const uint8_t* pSrcR = image_info.m_pMCUBufR + src_ofs;
        const uint8_t* pSrcG = image_info.m_pMCUBufG + src_ofs;
        const uint8_t* pSrcB = image_info.m_pMCUBufB + src_ofs;

        const int16_t bx_limit = min(8, image_info.m_width - (mcu_x * image_info.m_MCUWidth + x));

        for (int16_t by = 0; by < by_limit; by++) {
          uint8_t* buf = buf_block;
          for (int16_t bx = 0; bx < bx_limit; bx++) {
            buf[2] = *pSrcR++;
            buf[1] = *pSrcG++;
            buf[0] = *pSrcB++;
            buf += 3;
          }

          pSrcR += (8 - bx_limit);
          pSrcG += (8 - bx_limit);
          pSrcB += (8 - bx_limit);

          buf_block += row_pitch;
        }
      }

      buf_row += row_pitch * 8;
    }

    if (++mcu_x == image_info.m_MCUSPerRow) {
      bmp_encode_write(jpeg->bmp, mcu_y * image_info.m_MCUHeight, bmp_buffer, row_pitch, buf_lines); 
      total_lines += buf_lines;
      mcu_x = 0;
      mcu_y++;
    }

  }
printf("total lines=%d\n",total_lines);
end:

  himem_free(bmp_buffer, 0);

  rc = 0;

exit:

  return rc;
}
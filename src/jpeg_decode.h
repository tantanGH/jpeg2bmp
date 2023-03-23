#ifndef __H_JPEG_DECODE__
#define __H_JPEG_DECODE__

#include <stdint.h>
#include <stddef.h>
#include "bmp_encode.h"

typedef struct {
  BMP_ENCODE_HANDLE* bmp;
  uint16_t* rgb555_r;
  uint16_t* rgb555_g;
  uint16_t* rgb555_b;
} JPEG_DECODE_HANDLE;

int32_t jpeg_decode_init(JPEG_DECODE_HANDLE* jpeg);
void jpeg_decode_close(JPEG_DECODE_HANDLE* jpeg);
void jpeg_decode_set_bmp_encoder(JPEG_DECODE_HANDLE* jpeg, BMP_ENCODE_HANDLE* bmp);
int32_t jpeg_decode_exec(JPEG_DECODE_HANDLE* jpeg, uint8_t* jpeg_buffer, size_t jpeg_buffer_len);

#endif
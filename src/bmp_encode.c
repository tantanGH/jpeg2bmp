#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "himem.h"
#include "bmp_encode.h"

//
//  init BMP encode handler
//
int32_t bmp_encode_init(BMP_ENCODE_HANDLE* bmp, FILE* fp) {

  bmp->fp = fp;
  
  return 0;
}

//
//  close BMP encode handler
//
void bmp_encode_close(BMP_ENCODE_HANDLE* bmp) {
}

//
//  write BMP header
//
int32_t bmp_encode_write_header(BMP_ENCODE_HANDLE* bmp, uint32_t width, uint32_t height) {

  int32_t rc = -1;

  if (bmp == NULL || bmp->fp == NULL) goto exit;

  static uint8_t bmp_header[ 54 ];

  memset(bmp_header, 0, 54);

  // eye catch
  bmp_header[0] = 0x42;
  bmp_header[1] = 0x4d;

  // file size
  uint32_t file_size = 54 + width * height * 3;
  bmp_header[2] = file_size & 0xff;
  bmp_header[3] = (file_size >> 8) & 0xff;
  bmp_header[4] = (file_size >> 16) & 0xff;
  bmp_header[5] = (file_size >> 24) & 0xff;

  // header size
  bmp_header[10] = 54;

  // information size
  bmp_header[14] = 40;

  // width
  bmp_header[18] = width & 0xff;
  bmp_header[19] = (width >> 8) & 0xff;
  bmp_header[20] = (width >> 16) & 0xff;
  bmp_header[21] = (width >> 24) & 0xff;
  
  // height
  bmp_header[22] = height & 0xff;
  bmp_header[23] = (height >> 8) & 0xff;
  bmp_header[24] = (height >> 16) & 0xff;
  bmp_header[25] = (height >> 24) & 0xff;

  // planes
  bmp_header[26] = 1;

  // bit depth
  bmp_header[28] = 24;

  rc = fwrite(bmp_header, 1, 54, bmp->fp) == 54 ? 0 : -1;

exit:
  return rc;
}

//
//  encode
//
int32_t bmp_encode_write(BMP_ENCODE_HANDLE* bmp, uint8_t* bmp_buffer, size_t bmp_buffer_bytes) {
  size_t len = fwrite(bmp_buffer, 1, bmp_buffer_bytes, bmp->fp);
  return len >= bmp_buffer_bytes ? 0 : -1;
}
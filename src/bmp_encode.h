#ifndef __H_BMP_DECODE__
#define __H_BMP_DECODE__

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

typedef struct {
  FILE* fp;
} BMP_ENCODE_HANDLE;

int32_t bmp_encode_init(BMP_ENCODE_HANDLE* bmp, FILE* fp);
void bmp_encode_close(BMP_ENCODE_HANDLE* bmp);
int32_t bmp_encode_write_header(BMP_ENCODE_HANDLE* bmp, uint32_t width, uint32_t height);
int32_t bmp_encode_write(BMP_ENCODE_HANDLE* bmp, uint8_t* bmp_buffer, size_t bmp_buffer_bytes);

#endif
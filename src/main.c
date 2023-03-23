#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <doslib.h>
#include <iocslib.h>
#include "himem.h"
#include "jpeg_decode.h"
#include "bmp_encode.h"

#define VERSION "0.1.0 (2023/03/24)"

#define MAX_PATH_LEN (255)

//
//  show help message
//
static void show_help_message() {
  printf("usage: jpeg2bmp [options] <file.jpg>\n");
  printf("options:\n");
  printf("   -o <outfile> ... output filename (default:auto)\n");
  printf("   -h           ... show help message\n");
}

//
//  main
//
int32_t main(int32_t argc, uint8_t* argv[]) {

  // default return code
  int32_t rc = -1;

  // source file name
  uint8_t* file_name = NULL;

  // source buffer pointer
  uint8_t* file_data = NULL;

  // source file pointer
  FILE* fp = NULL;

  // output file name
  static uint8_t out_name[ MAX_PATH_LEN + 1 ];
  out_name[0] = '\0';

  // output file pointer
  FILE* fo = NULL;

  // credit
  printf("JPEG2BMP.X - JPEG to BMP converter for X680x0 " VERSION " by tantan\n");

  // check command line
  if (argc < 2) {
    show_help_message();
    goto exit;
  }

  // parse command lines
  for (int16_t i = 1; i < argc; i++) {
    if (argv[i][0] == '-' && strlen(argv[i]) >= 2) {
      if (argv[i][1] == 'o') {
        if (i+1 < argc && out_name[0] == '\0') {
          strcpy(out_name, argv[i+1]);
        } else {
          show_help_message();
          goto exit;
        }
      } else if (argv[i][1] == 'h') {
        show_help_message();
        goto exit;
      } else {
        printf("error: unknown option (%s).\n",argv[i]);
        goto exit;
      }
    } else {
      if (file_name != NULL || strlen(file_name) < 5) {
        show_help_message();
        goto exit;
      }
      file_name = argv[i];
    }
  }

  if (file_name == NULL) {
    show_help_message();
    goto exit;
  }

  if (out_name[0] == '\0') {
    strcpy(out_name, file_name);
    uint8_t* out_ext = out_name + strlen(out_name) - 4;
    uint8_t* file_ext = file_name + strlen(file_name) - 4;
    if (strcmp(file_ext, ".jpg") == 0) {
      strcpy(out_ext, ".bmp");
    } else {
      strcpy(out_ext, ".BMP");
    }
  }
    
  // read JPEG data from file into memory buffer
  fp = fopen(file_name, "rb");
  if (fp == NULL) {
    printf("error: failed to open file: %s\n", file_name);
    goto exit;
  }

  fseek(fp, 0, SEEK_END);
  size_t file_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  file_data = (uint8_t*)himem_malloc(file_size, 0);
  if (file_data == NULL) {
    printf("error: out of memory.\n");
    goto exit;
  }

  size_t read_len = 0;
  do {
    size_t len = fread(file_data + read_len, 1, file_size - read_len, fp);
    if (len == 0) break;
    read_len += len;
  } while (read_len < file_size);
  fclose(fp);
  fp = NULL;

  BMP_ENCODE_HANDLE bmp_encode = { 0 };
  JPEG_DECODE_HANDLE jpeg_decode = { 0 };
  jpeg_decode_init(&jpeg_decode);
  jpeg_decode_set_bmp_encoder(&jpeg_decode, &bmp_encode);
  rc = jpeg_decode_exec(&jpeg_decode, file_data, file_size);
  jpeg_decode_close(&jpeg_decode);
  bmp_encode_close(&bmp_encode);

exit:

  if (fo != NULL) {
    fclose(fo);
    fo = NULL;
  }

  if (fp != NULL) {
    fclose(fp);
    fp = NULL;
  }

  if (file_data != NULL) {
    himem_free(file_data, 0);
    file_data = NULL;
  }

  return rc;
}
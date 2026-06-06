#ifndef IMAGE_H
#define IMAGE_H

#include <stddef.h>

typedef struct Image {
  int width;
  int height;
  int channels;
  unsigned char *buf;    // Raw downloaded data
  int buf_size;          // Number of bytes
  unsigned char *pixels; // Decoded pixels
} Image;

Image *image_load_url(char *url);

int image_print(Image *img, int width);

int image_free(Image *img);

#endif

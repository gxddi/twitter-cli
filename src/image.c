#include "../include/image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

#include <caca.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

size_t write_to_mem(void *contents, size_t size_n, size_t n, Image *img) {

  size_t chunk_size = (size_n * n);

  img->buf = realloc(img->buf, img->buf_size + chunk_size);
  memcpy(img->buf + img->buf_size, contents, chunk_size);

  img->buf_size += chunk_size;

  return chunk_size;
}

Image *image_load_url(char *url) {

  curl_global_init(CURL_GLOBAL_DEFAULT);
  CURL *curl = curl_easy_init();

  if (!curl) {
    printf("Curl init failed\n");
    return NULL;
  } else {
    // printf("Succesfully initialized curl\n");
  }

  // Setting up options
  // printf("Setting up request options:\n");

  // Set URL
  // printf("  URL...\n");
  curl_easy_setopt(curl, CURLOPT_URL, url);

  // Set up write function
  // printf(" Write function...\n");
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_mem);

  Image *img = malloc(sizeof(Image));
  img->buf_size = 0;
  img->buf = malloc(0);

  curl_easy_setopt(curl, CURLOPT_WRITEDATA, img);

  // Perform request
  // printf("Request configured.\nPerforming request...\n");
  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    fprintf(stderr, "curl error: %s\n", curl_easy_strerror(res));
  } else {
    // printf("Request performed succesfully\n");
  }

  // Set decoded pixels, width, height and channels
  // printf("Decoding pixels...\n");
  img->pixels = stbi_load_from_memory(img->buf, img->buf_size, &(img->width),
                                      &(img->height), &(img->channels), 4);

  // Clean up
  // printf("(image) Cleaning up...\n");
  curl_easy_cleanup(curl);
  curl_global_cleanup();

  return img;
}

int image_print(Image *img, int width) {
  // printf("Printing with caca...");
  int height = (img->height * width) / (img->width * 2);

  caca_canvas_t *canvas = caca_create_canvas(width, height);
  caca_dither_t *dither =
      caca_create_dither(32, img->width, img->height, img->width * 4,
                         0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

  caca_dither_bitmap(canvas, 0, 0, width, height, dither, img->pixels);

  // Print directly to stdout, no display/window needed
  size_t len;
  void *out = caca_export_canvas_to_memory(canvas, "ansi", &len);
  fwrite(out, 1, len, stdout);
  free(out);

  caca_free_dither(dither);
  caca_free_canvas(canvas);
  return 0;
};

int image_free(Image *img) {
  if (img) {
    free(img->buf);
    stbi_image_free(img->pixels);
    img->buf = img->pixels = NULL;
    img->buf_size = img->width = img->height = img->channels = 0;
    return 0;
    free(img);
  }
  return 1;
}

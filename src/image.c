#include "../include/image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb_image.h"

#include <curl/curl.h>
// #include <sixel.h>
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
  if (!img || !img->pixels) {
    return 1;
  }

  // Kitty requires 32-bit RGBA for raw pixels (which stb_image provides with 4
  // channels)
  size_t total_bytes = img->width * img->height * 4;

  // Kitty protocol limits payload chunks to 4096 bytes.
  // Base64 expands data by 4/3, so 3000 raw bytes = 4000 base64 bytes (safe).
  size_t chunk_size = 3000;

  char initial_keys[128];
  if (width > 0) {
    // a=T (transmit and display), f=32 (RGBA), s=width, v=height, c=columns
    // (scales to fit)
    snprintf(initial_keys, sizeof(initial_keys), "a=T,f=32,s=%d,v=%d,c=%d",
             img->width, img->height, width);
  } else {
    snprintf(initial_keys, sizeof(initial_keys), "a=T,f=32,s=%d,v=%d",
             img->width, img->height);
  }

  static const char b64_table[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  for (size_t offset = 0; offset < total_bytes; offset += chunk_size) {
    size_t current_chunk_len = total_bytes - offset;
    if (current_chunk_len > chunk_size) {
      current_chunk_len = chunk_size;
    }

    int is_last = (offset + current_chunk_len >= total_bytes);

    // The first chunk needs the metadata format keys.
    // m=1 means more chunks are coming, m=0 means this is the final chunk.
    if (offset == 0) {
      printf("\033_G%s,m=%d;", initial_keys, is_last ? 0 : 1);
    } else {
      printf("\033_Gm=%d;", is_last ? 0 : 1);
    }

    // Base64 encode the current chunk directly to stdout
    unsigned char *data = img->pixels + offset;
    for (size_t i = 0; i < current_chunk_len; i += 3) {
      uint32_t n = data[i] << 16;
      if (i + 1 < current_chunk_len)
        n |= data[i + 1] << 8;
      if (i + 2 < current_chunk_len)
        n |= data[i + 2];

      putchar(b64_table[(n >> 18) & 63]);
      putchar(b64_table[(n >> 12) & 63]);
      putchar(i + 1 < current_chunk_len ? b64_table[(n >> 6) & 63] : '=');
      putchar(i + 2 < current_chunk_len ? b64_table[n & 63] : '=');
    }

    // Close the escape sequence for this chunk
    printf("\033\\");
  }

  return 0;
}

int image_free(Image *img) {
  if (img) {
    free(img->buf);
    stbi_image_free(img->pixels);
    img->buf = img->pixels = NULL;
    img->buf_size = img->width = img->height = img->channels = 0;
    free(img);
    return 0;
  }
  return 1;
}

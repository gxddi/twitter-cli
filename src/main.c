#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "../include/fetch.h"
#include "../include/image.h"
#include "../include/tweet.h"

int main() {

  // Get window width
  struct winsize w;

  // Fetch tweets
  Fetched *resp = fetch_tweets(20, NULL);

  if (resp == NULL) {
    printf("Request failed\n");
    return 1;
  } else if (strlen(resp->data) == 0) {
    printf("Response is empty\n");
    return 1;
  }

  // Parse response into tweets linked list
  Tweet *tweet_head = tweet_parse_json(resp->data);

  // Go down tweets
  Tweet *cur = tweet_head;
  while (cur) {

    // Formatting
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n");
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    for (int i = 0; i < w.ws_col; i++) {
      printf("=");
    }
    printf("\n");

    // Print user and text
    printf("@%s: %s\n", cur->username, cur->text);

    // Print image
    if (cur->image_url) {
      printf("%s\n", cur->image_url);
      ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

      // Load image url and print
      Image *img = image_load_url(cur->image_url);
      image_print(img, w.ws_col / 2);
      // printf("%s\n", cur->image_url);
      image_free(img);
    }

    // Print quoted tweet
    if (cur->quoted_tweet) {
      printf("  \"@%s: %s\"\n", cur->quoted_tweet->username,
             cur->quoted_tweet->text);
      // Print image
      if (cur->image_url) {
        printf("%s\n", cur->image_url);
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

        // Load image url and print
        Image *img = image_load_url(cur->image_url);
        image_print(img, w.ws_col / 3);
        // printf("%s\n", cur->image_url);
        image_free(img);
      }
    }

    // Formatting
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    for (int i = 0; i < w.ws_col; i++) {
      printf("=");
    }
    printf("\n");
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");

    // Clear input buffer
    while (getchar() != '\n')
      ;
    // Wait for keypress
    getchar();
    cur = cur->next;
  }

  fetch_free(resp);
  tweet_free(tweet_head);
}

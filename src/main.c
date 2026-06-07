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

  // Clear screen before starting the feed
  printf("\033[2J\033[H");

  // Go through tweets
  Tweet *cur = tweet_head;
  while (cur) {
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    // Top grid separator
    printf("\n\033[38;5;239m");
    for (int i = 0; i < w.ws_col; i++)
      printf("━");
    printf("\033[0m\n\n");

    // Main tweet
    printf("\033[1;34m@%s\033[0m\n", cur->username);
    printf("%s\n\n", cur->text);

    if (cur->image_url) {
      Image *img = image_load_url(cur->image_url);
      image_print(img, w.ws_col / 2);
      printf("\n");
      image_free(img);
    }

    // Quoted tweet layout
    if (cur->quoted_tweet) {
      // Left border for quoted content
      printf("\033[38;5;239m  "
             "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\033[0m\n");
      printf("\033[38;5;239m  ┃\033[0m \033[1;34m@%s\033[0m\n",
             cur->quoted_tweet->username);
      printf("\033[38;5;239m  ┃\033[0m %s\n", cur->quoted_tweet->text);
      printf("\033[38;5;239m  ┃\033[0m\n");

      if (cur->quoted_tweet->image_url) {
        // Print 4 spaces to align the image with the quoted text
        printf("    ");
        fflush(stdout);

        Image *img = image_load_url(cur->quoted_tweet->image_url);
        image_print(img, w.ws_col / 3);
        printf("\n");
        image_free(img);
      }
    }

    printf("\n\033[2mPress Enter to load next...\033[0m");
    fflush(stdout);

    // Wait for the user to press Enter
    while (getchar() != '\n')
      ;

    // Move cursor up one line and clear it so the feed looks continuous
    printf("\033[1A\033[K");
    cur = cur->next;
  }

  fetch_free(resp);
  tweet_free(tweet_head);
}

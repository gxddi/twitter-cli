#ifndef TWEET_H
#define TWEET_H

typedef struct Tweet {
  char *username;
  char *text;
  char *image_url;
  struct Tweet *quoted_tweet; // Tweet it's interacting with
  struct Tweet *next;         // Linked list
} Tweet;

Tweet *tweet_parse_json(char *json);

int tweet_free(Tweet *root);

#endif

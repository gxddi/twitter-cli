#ifndef FETCH_H
#define FETCH_H

#include <stddef.h>

// Struct specified in tweet.h
typedef struct Tweet Tweet;

typedef struct Fetched {
  char *data;
  size_t size;
} Fetched;

// Fetch n tweets
Fetched *fetch_tweets(int n, Tweet *seen_tweets);

int fetch_free(Fetched *resp);

#endif

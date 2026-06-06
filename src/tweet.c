#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/tweet.h"

Tweet *parse_single_result(cJSON *content_result) {
  // Result
  cJSON *result = cJSON_GetObjectItem(content_result, "result");
  if (!result)
    return NULL;

  // Essentials:
  // Get full text
  cJSON *legacy = cJSON_GetObjectItem(result, "legacy");
  if (!legacy)
    return NULL;
  cJSON *text = cJSON_GetObjectItem(legacy, "full_text");
  if (!text)
    return NULL;
  // Get user details
  cJSON *core = cJSON_GetObjectItem(result, "core");
  if (!core)
    return NULL;
  cJSON *user_results = cJSON_GetObjectItem(core, "user_results");
  if (!user_results)
    return NULL;
  cJSON *user_result = cJSON_GetObjectItem(user_results, "result");
  if (!user_result)
    return NULL;
  cJSON *user_core = cJSON_GetObjectItem(user_result, "core");
  if (!user_core)
    return NULL;
  cJSON *username = cJSON_GetObjectItem(user_core, "screen_name");
  if (!username)
    return NULL;

  Tweet *tweet = malloc(sizeof(Tweet));
  if (tweet == NULL) {
    printf("Allocation of quoted_tweet failed\n");
    return NULL;
  }

  tweet->text = strdup(text->valuestring); // duplicate and own strings
  tweet->username = strdup(username->valuestring);

  // Get image
  tweet->image_url = NULL;
  cJSON *entities = cJSON_GetObjectItem(legacy, "entities");
  if (entities) {
    cJSON *media = cJSON_GetObjectItem(entities, "media");
    if (media) {
      cJSON *media_object = media->child;
      if (media_object) {
        cJSON *media_url_https =
            cJSON_GetObjectItem(media_object, "media_url_https");
        if (media_url_https) {
          tweet->image_url = strdup(media_url_https->valuestring);
        } else {
          printf("media_object doesn't have the object media_url_https");
          printf("media_object: %s", media_object->valuestring);
        }
      } else {
        printf("Something went wrong, you don't understand media_object.");
      }
    }
  }

  tweet->quoted_tweet = NULL;
  tweet->next = NULL;

  return tweet;
}

Tweet *tweet_parse_json(char *json) {
  // Read data from json response file
  /*FILE *stream = fopen(json_file, "r");
  fseek(stream, 0, SEEK_END);
  int stream_len = ftell(stream);
  rewind(stream);
  char *raw_data = malloc(stream_len);
  fread(raw_data, 1, stream_len, stream);*/

  // Parse json
  printf("Parsing json...\n");
  cJSON *root = cJSON_Parse(json);

  // root -> data -> home -> home_timeline -> instructions -> list of
  // instructions -> entries
  cJSON *entries = root->child->child->child->child->child->child;

  Tweet *head = NULL;
  Tweet *tail = NULL;

  // For each entry
  printf("Parsing each entry:\n");
  int no_entries = cJSON_GetArraySize(entries);
  for (int i = 0; i < no_entries; i++) {
    printf("  Entry %d/%d\n", i + 1, no_entries);

    // Item
    cJSON *entry = cJSON_GetArrayItem(entries, i);
    cJSON *item = cJSON_GetObjectItem(cJSON_GetObjectItem(entry, "content"),
                                      "itemContent");
    if (!item)
      continue; // skip cursors

    // Result
    cJSON *result = cJSON_GetObjectItem(
        cJSON_GetObjectItem(item, "tweet_results"), "result");
    if (!result)
      continue;

    // Essentials:
    // Get full text
    cJSON *legacy = cJSON_GetObjectItem(result, "legacy");
    if (!legacy)
      continue;
    cJSON *text = cJSON_GetObjectItem(legacy, "full_text");
    if (!text)
      continue;
    // Get user details
    cJSON *core = cJSON_GetObjectItem(result, "core");
    if (!core)
      continue;
    cJSON *user_results = cJSON_GetObjectItem(core, "user_results");
    if (!user_results)
      continue;
    cJSON *user_result = cJSON_GetObjectItem(user_results, "result");
    if (!user_result)
      continue;
    cJSON *user_core = cJSON_GetObjectItem(user_result, "core");
    if (!user_core)
      continue;
    cJSON *username = cJSON_GetObjectItem(user_core, "screen_name");
    if (!username)
      continue;

    // Construct Tweet
    Tweet *tweet = malloc(sizeof(Tweet));
    if (tweet == NULL) {
      printf("Allocation of tweet failed\n");
      return NULL;
    }

    tweet->text = strdup(text->valuestring); // duplicate to own strings
    tweet->username = strdup(username->valuestring);

    // Get image
    tweet->image_url = NULL;
    cJSON *entities = cJSON_GetObjectItem(legacy, "entities");
    if (entities) {
      cJSON *media = cJSON_GetObjectItem(entities, "media");
      if (media) {
        cJSON *media_object = media->child;
        if (media_object) {
          cJSON *media_url_https =
              cJSON_GetObjectItem(media_object, "media_url_https");
          if (media_url_https) {
            tweet->image_url = strdup(media_url_https->valuestring);
          } else {
            printf("media_object doesn't have the object media_url_https");
            printf("media_object: %s", media_object->valuestring);
          }
        } else {
          printf("Something went wrong, you don't understand media_object.");
        }
      }
    }

    // Get quoted tweet
    cJSON *quoted_status_result =
        cJSON_GetObjectItem(result, "quoted_status_result");
    if (quoted_status_result) { // If it exists
      tweet->quoted_tweet = parse_single_result(quoted_status_result);
    } else {
      tweet->quoted_tweet = NULL;
    }

    tweet->next = NULL; // Current tail

    if (!head) {
      head = tweet;
      tail = tweet;
    } else {
      tail->next = tweet;
      tail = tweet;
    }
  }

  // Clean up
  printf("(tweet) Cleaning up...\n");
  cJSON_Delete(root);

  // Return tweets
  printf("Returning tweets...\n");
  return head;
}

int tweet_free(Tweet *root) {
  // Cleanup
  Tweet *cur = root;
  while (cur) {
    Tweet *tmp = cur->next;
    free(cur->text);
    free(cur->username);
    if (cur->quoted_tweet) {
      free(cur->quoted_tweet->text);
      free(cur->quoted_tweet->username);
      free(cur->quoted_tweet);
    }
    free(cur);
    cur = tmp;
  }
  return 0;
}

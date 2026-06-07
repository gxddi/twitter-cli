#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cjson/cJSON.h>
#include <curl/curl.h>

#include "../include/fetch.h"
#include "../include/tweet.h"

typedef struct Credentials {
  char qid[50]; // Query ID
  char csrf[200];
  char tid[200]; // Transaction ID
  char auth_bearer[200];
  char twid[100]; // Twitter ID
  char auth_token[100];
  char cf_bm[250]; // Cloudflare bot management
} Credentials;

/* Read and parse path into Credentials struct */
Credentials get_creds(char *path) {
  // Read credentials.conf
  FILE *file = fopen(path, "r");
  if (!file) {
    printf("Could not open %s\n", path);
    exit(1);
  }

  fseek(file, 0, SEEK_END);
  int len = ftell(file);
  rewind(file);

  char *contents = malloc((sizeof(char) * len) + 1);
  if (!contents) {
    printf("Allocation of memory for credentials failed.\nIs %s too large?\n",
           path);
    exit(1);
  }
  fread(contents, sizeof(char), len, file);
  contents[len] = '\0';
  fclose(file);

  // Parse credentials and fill out Credentials struct
  Credentials creds;
  memset(&creds, 0, sizeof(Credentials));

  char *line = strtok(contents, "\n");
  while (line) {
    char key[50], value[250];
    if (sscanf(line, "%49[^=]=%249[^;]", key, value) == 2) {
      if (strcmp(key, "query_id") == 0)
        strncpy(creds.qid, value, sizeof(creds.qid) - 1);
      else if (strcmp(key, "csrf_token") == 0)
        strncpy(creds.csrf, value, sizeof(creds.csrf) - 1);
      else if (strcmp(key, "transaction_id") == 0)
        strncpy(creds.tid, value, sizeof(creds.tid) - 1);
      else if (strcmp(key, "auth_bearer") == 0)
        strncpy(creds.auth_bearer, value, sizeof(creds.auth_bearer) - 1);
      else if (strcmp(key, "twitter_id") == 0)
        strncpy(creds.twid, value, sizeof(creds.twid) - 1);
      else if (strcmp(key, "auth_token") == 0)
        strncpy(creds.auth_token, value, sizeof(creds.auth_token) - 1);
      else if (strcmp(key, "cf_bm") == 0)
        strncpy(creds.cf_bm, value, sizeof(creds.cf_bm) - 1);
    }
    line = strtok(NULL, "\n");
  }

  free(contents);
  return creds;
}

/* Internal write function to write json to Response struct */
size_t fetch_write_to_mem(void *contents, size_t size_n, size_t n,
                          Fetched *resp) {
  printf("Writing %zu characters of %zu bytes, into memory...\n", n, size_n);

  size_t chunk_size = (size_n * n);

  resp->data = realloc(resp->data, resp->size + chunk_size + 1);
  memcpy(resp->data + resp->size, contents, chunk_size);

  resp->size += chunk_size;

  resp->data[resp->size] = '\0';

  return chunk_size;
}

/* fetch n tweets */
Fetched *fetch_tweets(int n, Tweet *seen_tweets) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
  CURL *curl = curl_easy_init();

  if (!curl) {
    printf("Curl init failed\n");
    return NULL;
  } else {
    printf("Succesfully initialized curl\n");
  }

  // Credentials
  printf("Getting credentials...\n");
  Credentials creds = get_creds("credentials.conf");

  printf("Setting up request options:\n");

  // Build URL with payload parameters
  printf("  URL & Payload...\n");
  char url[3072];
  sprintf(
      url,
      "https://x.com/i/api/graphql/%s/HomeTimeline"
      "?variables=%%7B%%22count%%22%%3A20%%2C%%22includePromotedContent%%22%%"
      "3Atrue%%2C%%22withCommunity%%22%%3Atrue%%7D&features=%%7B%%22rweb_vide"
      "o_screen_enabled%%22%%3Afalse%%2C%%22rweb_cashtags_enabled%%22%%3Atrue"
      "%%2C%%22profile_label_improvements_pcf_label_in_post_enabled%%22%%3Atr"
      "ue%%2C%%22responsive_web_profile_redirect_enabled%%22%%3Afalse%%2C%%22"
      "rweb_tipjar_consumption_enabled%%22%%3Afalse%%2C%%22verified_phone_lab"
      "el_enabled%%22%%3Afalse%%2C%%22creator_subscriptions_tweet_preview_api"
      "_enabled%%22%%3Atrue%%2C%%22responsive_web_graphql_timeline_navigation"
      "_enabled%%22%%3Atrue%%2C%%22responsive_web_graphql_skip_user_profile_i"
      "mage_extensions_enabled%%22%%3Afalse%%2C%%22premium_content_api_read_e"
      "nabled%%22%%3Afalse%%2C%%22communities_web_enable_tweet_community_resu"
      "lts_fetch%%22%%3Atrue%%2C%%22c9s_tweet_anatomy_moderator_badge_enabled"
      "%%22%%3Atrue%%2C%%22responsive_web_grok_analyze_button_fetch_trends_en"
      "abled%%22%%3Afalse%%2C%%22responsive_web_grok_analyze_post_followups_e"
      "nabled%%22%%3Atrue%%2C%%22rweb_cashtags_composer_attachment_enabled%%2"
      "2%%3Atrue%%2C%%22responsive_web_jetfuel_frame%%22%%3Atrue%%2C%%22respo"
      "nsive_web_grok_share_attachment_enabled%%22%%3Atrue%%2C%%22responsive_"
      "web_grok_annotations_enabled%%22%%3Atrue%%2C%%22articles_preview_enabl"
      "ed%%22%%3Atrue%%2C%%22responsive_web_edit_tweet_api_enabled%%22%%3Atru"
      "e%%2C%%22rweb_conversational_replies_downvote_enabled%%22%%3Afalse%%2C"
      "%%22graphql_is_translatable_rweb_tweet_is_translatable_enabled%%22%%3A"
      "true%%2C%%22view_counts_everywhere_api_enabled%%22%%3Atrue%%2C%%22long"
      "form_notetweets_consumption_enabled%%22%%3Atrue%%2C%%22responsive_web_"
      "twitter_article_tweet_consumption_enabled%%22%%3Atrue%%2C%%22content_d"
      "isclosure_indicator_enabled%%22%%3Atrue%%2C%%22content_disclosure_ai_g"
      "enerated_indicator_enabled%%22%%3Atrue%%2C%%22responsive_web_grok_show"
      "_grok_translated_post%%22%%3Atrue%%2C%%22responsive_web_grok_analysis_"
      "button_from_backend%%22%%3Atrue%%2C%%22post_ctas_fetch_enabled%%22%%3A"
      "true%%2C%%22freedom_of_speech_not_reach_fetch_enabled%%22%%3Atrue%%2C%"
      "%22standardized_nudges_misinfo%%22%%3Atrue%%2C%%22tweet_with_visibilit"
      "y_results_prefer_gql_limited_actions_policy_enabled%%22%%3Atrue%%2C%%2"
      "2longform_notetweets_rich_text_read_enabled%%22%%3Atrue%%2C%%22longfor"
      "m_notetweets_inline_media_enabled%%22%%3Afalse%%2C%%22responsive_web_g"
      "rok_image_annotation_enabled%%22%%3Atrue%%2C%%22responsive_web_grok_im"
      "agine_annotation_enabled%%22%%3Atrue%%2C%%22responsive_web_grok_commun"
      "ity_note_auto_translation_is_enabled%%22%%3Atrue%%2C%%22responsive_web"
      "_enhance_cards_enabled%%22%%3Afalse%%7D",
      creds.qid);
  curl_easy_setopt(curl, CURLOPT_URL, url);

  // Set headers
  printf("  Headers...\n");

  //  Non personal
  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Accept: */*");
  headers = curl_slist_append(headers, "Accept-Language: en-US,en;q=0.9");
  headers = curl_slist_append(headers, "Referer: https://x.com/home");
  headers = curl_slist_append(headers, "content-type: application/json");
  headers = curl_slist_append(headers, "x-twitter-auth-type: OAuth2Session");
  headers = curl_slist_append(headers, "x-twitter-client-language: en");
  headers = curl_slist_append(headers, "x-twitter-active-user: yes");

  //  Personal
  //    CSRF token
  char x_csrf_token[225];
  sprintf(x_csrf_token, "x-csrf-token: %s", creds.csrf);
  headers = curl_slist_append(headers, x_csrf_token);

  //    Transaction ID
  char transaction_id[225];
  sprintf(transaction_id, "x-client-transaction-id: %s", creds.tid);
  headers = curl_slist_append(headers, transaction_id);

  //    Authentication
  char auth_bearer[225];
  sprintf(auth_bearer, "authorization: %s", creds.auth_bearer);
  headers = curl_slist_append(headers, auth_bearer);

  //    Cookies
  //      Client unique id (Optional)
  // cookies.cuid = "__cuid=";
  //      Google state (Optional)
  // cookies.g_state = "g_state=";
  //      Known device token (Optional)
  // cookies.kdt = "kdt=";
  //      Do not track (Optional)
  // cookies.dnt = "dnt=";
  //      Guest ID (Optional)
  // cookies.guest_id = "guest_id=";
  //      Display preferences (Optional)
  // cookies.d_prefs = "d_prefs=";
  //      Twitter ID (Required)
  char twid[125];
  sprintf(twid, "twid=%s;", creds.twid);
  //      CSRF token (Required, needs to match)
  char ct0[225];
  sprintf(ct0, "ct0=%s;", creds.csrf);
  //      Auth token (Required)
  char auth_token[125];
  sprintf(auth_token, "auth_token=%s;", creds.auth_token);
  //      Language (Required)
  char lang[] = "lang=en;";
  //      Cloudflare bot management (Required)
  char cf_bm[300];
  sprintf(cf_bm, "__cf_bm=%s", creds.cf_bm);
  //      All cookies
  char all_cookies[2000];
  sprintf(all_cookies, "Cookie: %s%s%s%s", twid, ct0, auth_token, lang);
  headers = curl_slist_append(headers, all_cookies);

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  // Encoding
  printf("  Encoding...\n");
  curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

  // Configure write function
  printf("  Write function...\n");
  Fetched *resp = malloc(sizeof(Fetched));
  if (resp == NULL)
    printf("Allocation for resp failed");
  resp->data = malloc(1);
  resp->size = 0;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fetch_write_to_mem);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, resp);

  // Perform request
  printf("Request configured.\nPerforming request...\n");
  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    fprintf(stderr, "curl error: %s\n", curl_easy_strerror(res));
  } else {
    printf("Request performed succesfully\n");
    /*FILE *response_json = fopen("response.json", "w");
    cJSON *response_cjson = cJSON_Parse(resp->data);

    fwrite(cJSON_Print(response_cjson), 1, resp->size, response_json);
    fclose(response_json);
    cJSON_Delete(response_cjson);*/
  }

  // Clean up
  printf("(fetch) Cleaning up...\n");
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  curl_global_cleanup();
  return resp;
}

int fetch_free(Fetched *resp) {
  free(resp->data);
  free(resp);
  return 0;
}

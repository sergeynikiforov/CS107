/**
 * Use libcurl to get output from a server to FILE *
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "urlconnection.h"
#include "url.h"
#include <curl/curl.h>

static const char *const BUFFER_FILE_NAME = "url_conn_buff.txt";

static size_t fileWrite( void *buffer, size_t size, size_t nmemb, void *stream ) {
    size_t written = fwrite(buffer, size, nmemb, (FILE*)stream);
    return written * size;
}

void MyURLConnectionNew(urlconnection* urlconn, const url* u) {
    assert(urlconn != NULL);
    assert(u != NULL);

    // set response code to 0
    urlconn->responseCode = 0;

    // set up full url, allocate space, copy from url->fullName
    urlconn->fullUrl = (char*) malloc(strlen(u->fullName) + 1);
    assert(urlconn->fullUrl != NULL);
    strcpy(urlconn->fullUrl, u->fullName);

    // set up newUrl to be NULL at the beginning
    urlconn->newUrl = NULL;

    // use open_memstream data stream
    size_t size;
    urlconn->dataStream = open_memstream(&(urlconn->ptr), &size);
    assert(urlconn->dataStream != NULL);

    // set response message to be empty
    urlconn->responseMessage = (char*) malloc(CURL_ERROR_SIZE);
    assert(urlconn->responseMessage != NULL);
    (urlconn->responseMessage)[0] = 0;

    // set up CURL
    CURL *curl = curl_easy_init();
    if(curl) {
        CURLcode res;
        // set the URL
        curl_easy_setopt(curl, CURLOPT_URL, urlconn->fullUrl);

        // follow redirects
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // set error buffer to be a response message
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, urlconn->responseMessage);

        // set write callback
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fileWrite);

        // write to urlconn's dataStream
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, urlconn->dataStream);

        // perform request
        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        else
            strcpy(urlconn->responseMessage, "OK!");

        // get response code
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &(urlconn->responseCode));

        // set content type
        char *ct;
        curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &ct);
        urlconn->contentType = NULL;
        if (ct != NULL) {
            urlconn->contentType = (char*) malloc(strlen(ct) + 1);
            strcpy(urlconn->contentType, ct);
        }

        // clean everything curl-related
        curl_easy_cleanup(curl);
    }
}

void MyURLConnectionDispose(urlconnection* urlconn) {
    if (urlconn->fullUrl != NULL)
        free(urlconn->fullUrl);
    if (urlconn->contentType != NULL)
        free(urlconn->contentType);
    if (urlconn->responseMessage != NULL)
        free(urlconn->responseMessage);
    // close FILE dataStream
    fclose(urlconn->dataStream);
    free(urlconn->ptr);
}

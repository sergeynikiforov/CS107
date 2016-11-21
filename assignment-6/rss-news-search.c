#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <curl/curl.h>

#include "url.h"
#include "bool.h"
#include "urlconnection.h"
#include "streamtokenizer.h"
#include "html-utils.h"
#include "hashset.h"
#include "vector.h"
#include "search-result.h"

static void Welcome(const char *welcomeTextFileName);
static void BuildIndices(const char *feedsFileName, const hashset *stopwords, hashset *words, vector *seenArticles, vector *threads, sem_t *wSem, sem_t *httpSem);
static void ProcessFeed(const char *remoteDocumentName, hashset *seenUrls, const hashset *stopwords, hashset *words, vector *seenArticles, vector *threads, sem_t *wSem, sem_t *httpSem);
static void PullAllNewsItems(urlconnection *urlconn, hashset *seenUrls, const hashset *stopwords, hashset *words, vector *seenArticles, vector *threads, sem_t *wSem, sem_t *httpSem);
static bool GetNextItemTag(streamtokenizer *st);
static void ProcessSingleNewsItem(streamtokenizer *st, hashset *seenUrls, const hashset *stopwords, hashset *words, vector *seenArticles, vector *threads, sem_t *wSem, sem_t *httpSem);
static void ExtractElement(streamtokenizer *st, const char *htmlTag, char dataBuffer[], int bufferLength);
static void *ParseArticle(void* args);
static void ScanArticle(streamtokenizer *st, const char *articleTitle, const char *unused, const char *articleURL, const hashset *stopwords, hashset *words, int article_index, sem_t *wSem);
static void QueryIndices(const hashset *stopwords, const hashset *words, const vector *seenArticles);
static void ProcessResponse(const char *word, const hashset *stopwords, const hashset *words, const vector *seenArticles);
static bool WordIsWellFormed(const char *word);
static void BuildStopwordsHash(const char *stopWordsFileName, hashset *stopwords);
static int StringHash(const void *elem, int numBuckets);
static int StringCompare(const void *elem1, const void *elem2);
static void StringFree(void *elem);
static void PthreadFree(void *elem);
static void JoinPthreads(void *elem, void *aux);

/**
 * Function: main
 * --------------
 * Serves as the entry point of the full application.
 * You'll want to update main to declare several hashsets--
 * one for stop words, another for previously seen urls, etc--
 * and pass them (by address) to BuildIndices and QueryIndices.
 * In fact, you'll need to extend many of the prototypes of the
 * supplied helpers functions to take one or more hashset *s.
 *
 * Think very carefully about how you're going to keep track of
 * all of the stop words, how you're going to keep track of
 * all the previously seen articles, and how you're going to
 * map words to the collection of news articles where that
 * word appears.
 */

static const char *const kWelcomeTextFile = "/home/dissolved/Dropbox/CS107/assignment-6/assn-6-rss-news-search-data/welcome.txt";
static const char *const kDefaultFeedsFile = "/home/dissolved/Dropbox/CS107/assignment-6/assn-6-rss-news-search-data/rss-feeds-large.txt";
static const char *const stopWordsFile = "/home/dissolved/Dropbox/CS107/assignment-6/assn-6-rss-news-search-data/stop-words.txt";


int main(int argc, char **argv)
{
    // stopwords and words are hashsets of char** - ptr to dynamically allocated C-strings
    hashset stopwords;
    HashSetNew(&stopwords, sizeof(char**), 701, StringHash, StringCompare, StringFree);
    BuildStopwordsHash(stopWordsFile, &stopwords);

    // hashset of searchResultEntries
    hashset words;
    HashSetNew(&words, sizeof(searchResultEntry*), 10007, SearchResultEntryHash, SearchResultEntryCompare, SearchResultEntryFree);

    // vector of all scanned articles
    vector seenArticles;
    VectorNew(&seenArticles, sizeof(article*), ArticleFree, 93);

    // vector of malloc'ed pthread_t pointers that scan articles
    vector threads;
    VectorNew(&threads, sizeof(pthread_t*), PthreadFree, 4);

    // POSIX semaphore as a binary lock for hashset of words
    sem_t wordsSemaphore;
    int wSemRes = sem_init(&wordsSemaphore, 0, 1); // init to 1, share between threads
    assert(wSemRes != -1);

    // POSIX semaphore to limit number of open http connections
    sem_t httpSemaphore;
    int httpSemRes = sem_init(&httpSemaphore, 0, 24); // init to 24, share between threads
    assert(httpSemRes != -1);

    // initialize CURL before any thread starts
    curl_global_init(CURL_GLOBAL_ALL);

    Welcome(kWelcomeTextFile);
    BuildIndices((argc == 1) ? kDefaultFeedsFile : argv[1], &stopwords, &words, &seenArticles, &threads, &wordsSemaphore, &httpSemaphore);

    // wait all threads to terminate, dispose vector of threads' ids
    VectorMap(&threads, JoinPthreads, NULL);
    VectorDispose(&threads);

    // query for words
    QueryIndices(&stopwords, &words, &seenArticles);

    curl_global_cleanup();
    HashSetDispose(&stopwords);
    HashSetDispose(&words);
    VectorDispose(&seenArticles);
    pthread_exit(NULL);
}

/**
 * Function: Welcome
 * -----------------
 * Displays the contents of the specified file, which
 * holds the introductory remarks to be printed every time
 * the application launches.  This type of overhead may
 * seem silly, but by placing the text in an external file,
 * we can change the welcome text without forcing a recompilation and
 * build of the application.  It's as if welcomeTextFileName
 * is a configuration file that travels with the application.
 */

static const char *const kNewLineDelimiters = "\r\n";
static void Welcome(const char *welcomeTextFileName)
{
  FILE *infile;
  streamtokenizer st;
  char buffer[1024];

  infile = fopen(welcomeTextFileName, "r");
  assert(infile != NULL);

  STNew(&st, infile, kNewLineDelimiters, true);
  while (STNextToken(&st, buffer, sizeof(buffer))) {
    printf("%s\n", buffer);
  }

  printf("\n");
  STDispose(&st); // remember that STDispose doesn't close the file, since STNew doesn't open one..
  fclose(infile);
}

/**
 * Function: BuildIndices
 * ----------------------
 * As far as the user is concerned, BuildIndices needs to read each and every
 * one of the feeds listed in the specied feedsFileName, and for each feed parse
 * content of all referenced articles and store the content in the hashset of indices.
 * Each line of the specified feeds file looks like this:
 *
 *   <feed name>: <URL of remore xml document>
 *
 * Each iteration of the supplied while loop parses and discards the feed name (it's
 * in the file for humans to read, but our aggregator doesn't care what the name is)
 * and then extracts the URL.  It then relies on ProcessFeed to pull the remote
 * document and index its content.
 */

static void BuildIndices(const char *feedsFileName, const hashset *stopwords, hashset *words, vector *seenArticles, vector *threads, sem_t *wSem, sem_t *httpSem)
{
  FILE *infile;
  streamtokenizer st;
  char remoteFileName[1024];
  hashset seenUrls;
  HashSetNew(&seenUrls, sizeof(char**), 1001, StringHash, StringCompare, StringFree);

  infile = fopen(feedsFileName, "r");
  assert(infile != NULL);
  STNew(&st, infile, kNewLineDelimiters, true);
  while (STSkipUntil(&st, ":") != EOF) { // ignore everything up to the first semicolon of the line
    STSkipOver(&st, ": ");		 // now ignore the semicolon and any whitespace directly after it
    STNextToken(&st, remoteFileName, sizeof(remoteFileName));
    ProcessFeed(remoteFileName, &seenUrls, stopwords, words, seenArticles, threads, wSem, httpSem);
  }
  printf("\nNumber of parsed articles: %d\n", HashSetCount(&seenUrls));

  HashSetDispose(&seenUrls);
  STDispose(&st);
  fclose(infile);
  printf("\n");
}


/**
 * Function: ProcessFeed
 * ---------------------
 * ProcessFeed locates the specified RSS document, and if a (possibly redirected) connection to that remote
 * document can be established, then PullAllNewsItems is tapped to actually read the feed.  Check out the
 * documentation of the PullAllNewsItems function for more information, and inspect the documentation
 * for ParseArticle for information about what the different response codes mean.
 */

static void ProcessFeed(const char *remoteDocumentName, hashset *seenUrls, const hashset *stopwords, hashset *words, vector *seenArticles, vector *threads, sem_t *wSem, sem_t *httpSem)
{
  url u;
  urlconnection urlconn;

  URLNewAbsolute(&u, remoteDocumentName);
  MyURLConnectionNew(&urlconn, &u);
  printf("full url: %s\n", urlconn.fullUrl);
  printf("response msg: %s\n", urlconn.responseMessage);
  printf("content type: %s\n", urlconn.contentType);
  printf("response code: %d\n", urlconn.responseCode);

  if (urlconn.dataStream) {
      int c = 0;
      while ((c = getc(urlconn.dataStream)) != EOF)
          putchar(c);
      rewind(urlconn.dataStream);
  }

  switch (urlconn.responseCode) {
      case 0:
          printf("Unable to connect to \"%s\".  Ignoring...", u.serverName);
          break;
      case 200:
          PullAllNewsItems(&urlconn, seenUrls, stopwords, words, seenArticles, threads, wSem, httpSem);
          break;
      case 301:
      case 302:
          ProcessFeed(urlconn.newUrl, seenUrls, stopwords, words, seenArticles, threads, wSem, httpSem);
          break;
      default:
          printf("Connection to \"%s\" was established, but unable to retrieve \"%s\". [response code: %d, response message:\"%s\"]\n",
		      u.serverName, u.fileName, urlconn.responseCode, urlconn.responseMessage);
          break;
  };

  MyURLConnectionDispose(&urlconn);
  URLDispose(&u);
}

/**
 * Function: PullAllNewsItems
 * --------------------------
 * Steps though the data of what is assumed to be an RSS feed identifying the names and
 * URLs of online news articles.  Check out "datafiles/sample-rss-feed.txt" for an idea of what an
 * RSS feed from the www.nytimes.com (or anything other server that syndicates is stories).
 *
 * PullAllNewsItems views a typical RSS feed as a sequence of "items", where each item is detailed
 * using a generalization of HTML called XML.  A typical XML fragment for a single news item will certainly
 * adhere to the format of the following example:
 *
 * <item>
 *   <title>At Installation Mass, New Pope Strikes a Tone of Openness</title>
 *   <link>http://www.nytimes.com/2005/04/24/international/worldspecial2/24cnd-pope.html</link>
 *   <description>The Mass, which drew 350,000 spectators, marked an important moment in the transformation of Benedict XVI.</description>
 *   <author>By IAN FISHER and LAURIE GOODSTEIN</author>
 *   <pubDate>Sun, 24 Apr 2005 00:00:00 EDT</pubDate>
 *   <guid isPermaLink="false">http://www.nytimes.com/2005/04/24/international/worldspecial2/24cnd-pope.html</guid>
 * </item>
 *
 * PullAllNewsItems reads and discards all characters up through the opening <item> tag (discarding the <item> tag
 * as well, because once it's read and indentified, it's been pulled,) and then hands the state of the stream to
 * ProcessSingleNewsItem, which handles the job of pulling and analyzing everything up through and including the </item>
 * tag. PullAllNewsItems processes the entire RSS feed and repeatedly advancing to the next <item> tag and then allowing
 * ProcessSingleNewsItem do process everything up until </item>.
 */

static const char *const kTextDelimiters = " \t\n\r\b!@$%^*()_+={[}]|\\'\":;/?.>,<~`";
static void PullAllNewsItems(urlconnection *urlconn, hashset *seenUrls, const hashset *stopwords, hashset *words, vector *seenArticles, vector *threads, sem_t *wSem, sem_t *httpSem)
{
  streamtokenizer st;
  STNew(&st, urlconn->dataStream, kTextDelimiters, false);
  while (GetNextItemTag(&st)) { // if true is returned, then assume that <item ...> has just been read and pulled from the data stream
      ProcessSingleNewsItem(&st, seenUrls, stopwords, words, seenArticles, threads, wSem, httpSem);
  }

  STDispose(&st);
}

/**
 * Function: GetNextItemTag
 * ------------------------
 * Works more or less like GetNextTag below, but this time
 * we're searching for an <item> tag, since that marks the
 * beginning of a block of HTML that's relevant to us.
 *
 * Note that each tag is compared to "<item" and not "<item>".
 * That's because the item tag, though unlikely, could include
 * attributes and perhaps look like any one of these:
 *
 *   <item>
 *   <item rdf:about="Latin America reacts to the Vatican">
 *   <item requiresPassword=true>
 *
 * We're just trying to be as general as possible without
 * going overboard.  (Note that we use strncasecmp so that
 * string comparisons are case-insensitive.  That's the case
 * throughout the entire code base.)
 */

static const char *const kItemTagPrefix = "<item";
static bool GetNextItemTag(streamtokenizer *st)
{
  char htmlTag[1024];
  while (GetNextTag(st, htmlTag, sizeof(htmlTag))) {
    if (strncasecmp(htmlTag, kItemTagPrefix, strlen(kItemTagPrefix)) == 0) {
      return true;
    }
  }
  return false;
}

/**
 * Function: ProcessSingleNewsItem
 * -------------------------------
 * Code which parses the contents of a single <item> node within an RSS/XML feed.
 * At the moment this function is called, we're to assume that the <item> tag was just
 * read and that the streamtokenizer is currently pointing to everything else, as with:
 *
 *      <title>Carrie Underwood takes American Idol Crown</title>
 *      <description>Oklahoma farm girl beats out Alabama rocker Bo Bice and 100,000 other contestants to win competition.</description>
 *      <link>http://www.nytimes.com/frontpagenews/2841028302.html</link>
 *   </item>
 *
 * ProcessSingleNewsItem parses everything up through and including the </item>, storing the title, link, and article
 * description in local buffers long enough so that the online new article identified by the link can itself be parsed
 * and indexed.  We don't rely on <title>, <link>, and <description> coming in any particular order.  We do asssume that
 * the link field exists (although we can certainly proceed if the title and article descrption are missing.)  There
 * are often other tags inside an item, but we ignore them.
 */

static const char *const kItemEndTag = "</item>";
static const char *const kTitleTagPrefix = "<title";
static const char *const kDescriptionTagPrefix = "<description";
static const char *const kLinkTagPrefix = "<link";

// struct to pass arguments to a new thread
typedef struct parseArticleArgs {
    char *articleTitle;
    char *articleDescription;
    char *articleUrl;
    const hashset *stopWords;
    hashset *words;
    int artilceIndex;
    sem_t *wordsSemaphore;
    sem_t *httpSemaphore;
} parseArticleArgs;

static void ProcessSingleNewsItem(streamtokenizer *st, hashset *seenUrls, const hashset *stopwords, hashset *words, vector *seenArticles, vector *threads, sem_t *wSem, sem_t *httpSem)
{
  char htmlTag[1024];
  char articleTitle[1024];
  char articleDescription[1024];
  char articleURL[1024];
  articleTitle[0] = articleDescription[0] = articleURL[0] = '\0';

  while (GetNextTag(st, htmlTag, sizeof(htmlTag)) && (strcasecmp(htmlTag, kItemEndTag) != 0)) {
    if (strncasecmp(htmlTag, kTitleTagPrefix, strlen(kTitleTagPrefix)) == 0)
        ExtractElement(st, htmlTag, articleTitle, sizeof(articleTitle));
    if (strncasecmp(htmlTag, kDescriptionTagPrefix, strlen(kDescriptionTagPrefix)) == 0)
        ExtractElement(st, htmlTag, articleDescription, sizeof(articleDescription));
    if (strncasecmp(htmlTag, kLinkTagPrefix, strlen(kLinkTagPrefix)) == 0)
        ExtractElement(st, htmlTag, articleURL, sizeof(articleURL));
  }

  if (strncmp(articleURL, "", sizeof(articleURL)) == 0) return;     // punt, since it's not going to take us anywhere

  // check if it's the first time we're parsing the article
  char *url = strdup(articleURL);
  if (HashSetLookup(seenUrls, &url) == NULL) {
      // add to hashset
      HashSetEnter(seenUrls, &url);

      // add article to seenArticles vector
      article *current_article = (article*) malloc(sizeof(article));
      current_article->title = strdup(articleTitle);
      current_article->url = strdup(articleURL);
      VectorAppend(seenArticles, &current_article);
      int cur_index = VectorLength(seenArticles) - 1;

      // parse the article in a new joinable thread
      pthread_t *t = (pthread_t*) malloc(sizeof(pthread_t));
      pthread_attr_t attr;
      pthread_attr_init(&attr);
      pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

      parseArticleArgs *args = (parseArticleArgs*) malloc(sizeof(parseArticleArgs));
      args->articleTitle = strdup(articleTitle);;
      args->articleDescription = strdup(articleDescription);
      args->articleUrl = strdup(articleURL);
      args->stopWords = stopwords;
      args->words = words;
      args->artilceIndex = cur_index;
      args->wordsSemaphore = wSem;
      args->httpSemaphore = httpSem;
      printf("Args articleUrl: %s\n", args->articleUrl);

      int error = pthread_create(t, &attr, ParseArticle, (void*)args);
      if (error) {
          fprintf(stderr, "Couldn't run thread, errno %d\n", error);
          pthread_attr_destroy(&attr);
          exit(-1);
      } else {
          fprintf(stdout, "Thread started\n");
          VectorAppend(threads, &t);
          printf("Length of threads vector: %d\n", VectorLength(threads));
      }
      pthread_attr_destroy(&attr);
  } else {
      free(url);
      return;
  }
}

/**
 * Function: ExtractElement
 * ------------------------
 * Potentially pulls text from the stream up through and including the matching end tag.  It assumes that
 * the most recently extracted HTML tag resides in the buffer addressed by htmlTag.  The implementation
 * populates the specified data buffer with all of the text up to but not including the opening '<' of the
 * closing tag, and then skips over all of the closing tag as irrelevant.  Assuming for illustration purposes
 * that htmlTag addresses a buffer containing "<description" followed by other text, these three scanarios are
 * handled:
 *
 *    Normal Situation:     <description>http://some.server.com/someRelativePath.html</description>
 *    Uncommon Situation:   <description></description>
 *    Uncommon Situation:   <description/>
 *
 * In each of the second and third scenarios, the document has omitted the data.  This is not uncommon
 * for the description data to be missing, so we need to cover all three scenarious (I've actually seen all three.)
 * It would be quite unusual for the title and/or link fields to be empty, but this handles those possibilities too.
 */

static void ExtractElement(streamtokenizer *st, const char *htmlTag, char dataBuffer[], int bufferLength)
{
  assert(htmlTag[strlen(htmlTag) - 1] == '>');
  if (htmlTag[strlen(htmlTag) - 2] == '/') return;    // e.g. <description/> would state that a description is not being supplied
  STNextTokenUsingDifferentDelimiters(st, dataBuffer, bufferLength, "<");
  RemoveEscapeCharacters(dataBuffer);
  if (dataBuffer[0] == '<') strcpy(dataBuffer, "");  // e.g. <description></description> also means there's no description
  STSkipUntil(st, ">");
  STSkipOver(st, ">");
}

/**
 * Function: ParseArticle
 * ----------------------
 * Attempts to establish a network connect to the news article identified by the three
 * parameters.  The network connection is either established of not.  The implementation
 * is prepared to handle a subset of possible (but by far the most common) scenarios,
 * and those scenarios are categorized by response code:
 *
 *    0 means that the server in the URL doesn't even exist or couldn't be contacted.
 *    200 means that the document exists and that a connection to that very document has
 *        been established.
 *    301 means that the document has moved to a new location
 *    302 also means that the document has moved to a new location
 *    4xx and 5xx (which are covered by the default case) means that either
 *        we didn't have access to the document (403), the document didn't exist (404),
 *        or that the server failed in some undocumented way (5xx).
 *
 * The are other response codes, but for the time being we're punting on them, since
 * no others appears all that often, and it'd be tedious to be fully exhaustive in our
 * enumeration of all possibilities.
 */
static void *ParseArticle(void *args)
{
  url u;
  urlconnection urlconn;
  streamtokenizer st;

  URLNewAbsolute(&u, ((parseArticleArgs*)args)->articleUrl);

  // semaphore wait for new http connection
  int res_wait = sem_wait(((parseArticleArgs*)args)->httpSemaphore);
  assert(res_wait != -1);

  MyURLConnectionNew(&urlconn, &u);
  printf("Full URL: %s\n", urlconn.fullUrl);

  switch (urlconn.responseCode) {
      case 0:
          printf("Unable to connect to \"%s\".  Domain name or IP address is nonexistent.\n", ((parseArticleArgs*)args)->articleUrl);
	      break;
      case 200:
          printf("Scanning \"%s\" from \"http://%s\"\n", ((parseArticleArgs*)args)->articleTitle, u.serverName);
          STNew(&st, urlconn.dataStream, kTextDelimiters, false);
          ScanArticle(&st, ((parseArticleArgs*)args)->articleTitle, ((parseArticleArgs*)args)->articleDescription, ((parseArticleArgs*)args)->articleUrl, ((parseArticleArgs*)args)->stopWords, ((parseArticleArgs*)args)->words, ((parseArticleArgs*)args)->artilceIndex, ((parseArticleArgs*)args)->wordsSemaphore);
          STDispose(&st);
          break;
      case 301:
      case 302:
          // just pretend we have the redirected URL all along, though index using the new URL and not the old one...
          free((void*)((parseArticleArgs*)args)->articleUrl);
          ((parseArticleArgs*)args)->articleUrl = strdup(urlconn.newUrl);
          ParseArticle(args);
          printf("status 302, breaking from switch...");
          break;
      default:
          printf("Unable to pull \"%s\" from \"%s\". [Response code: %d] Punting...\n", ((parseArticleArgs*)args)->articleTitle, u.serverName, urlconn.responseCode);
          break;
  }

  MyURLConnectionDispose(&urlconn);

  // semaphore signal after closed http connection
  int res_post = sem_post(((parseArticleArgs*)args)->httpSemaphore);
  assert(res_post != -1);

  URLDispose(&u);

  // free passed args
  free((void*)((parseArticleArgs*)args)->articleTitle);
  free((void*)((parseArticleArgs*)args)->articleDescription);
  free((void*)((parseArticleArgs*)args)->articleUrl);
  free(args);
  pthread_exit(NULL);
}

/**
 * Function: ScanArticle
 * ---------------------
 * Parses the specified article, skipping over all HTML tags, and counts the numbers
 * of well-formed words that could potentially serve as keys in the set of indices.
 * Once the full article has been scanned, the number of well-formed words is
 * printed, and the longest well-formed word we encountered along the way
 * is printed as well.
 *
 * This is really a placeholder implementation for what will ultimately be
 * code that indexes the specified content.
 */

static void ScanArticle(streamtokenizer *st, const char *articleTitle, const char *unused, const char *articleURL, const hashset *stopwords, hashset *words, int article_index, sem_t *wSem)
{
  int numWords = 0, numStopWords = 0;
  char word[1024];
  char longestWord[1024] = {'\0'};

  // create extraInfoEntry
  extraInfoEntry info;
  info.counter = 1;
  info.articleID = article_index;

  while (STNextToken(st, word, sizeof(word))) {
    if (strcasecmp(word, "<") == 0) {
        SkipIrrelevantContent(st); // in html-utls.h
    } else {
        RemoveEscapeCharacters(word);
        if (WordIsWellFormed(word)) {
            numWords++;
            if (strlen(word) > strlen(longestWord))
                strcpy(longestWord, word);

            // convert word to lowercase
            for (int i = 0; word[i]; ++i)
                word[i] = tolower(word[i]);

            char *new_word = strdup(word);

            // if word not in the stopwords hashset, work with it, else discard
            if (HashSetLookup(stopwords, &new_word) == NULL) {
                searchResultEntry *sre = (searchResultEntry*)malloc(sizeof(searchResultEntry));
                assert(sre != NULL);
                sre->word = new_word;

                searchResultEntry **found; // found is a ptr to searchResultEntry

                // semaphore wait for words hashset
                int res_wait = sem_wait(wSem);
                assert(res_wait != -1);

                // try to find in seen words hashset
                if ((found = HashSetLookup(words, &sre)) != NULL) {
                    // the word is found within the words hashset,
                    // try to find the extraInfoEntry for the article being scanned within the word's extraInfo
                    int info_index;
                    if ((info_index = VectorSearch(&(*found)->extraInfo, &info, ExtraInfoEntryCompareByArticleID, 0, false)) != -1) {
                        // the word had been found within the article before, increment the counter
                        extraInfoEntry *found_info = (extraInfoEntry*) VectorNth(&(*found)->extraInfo, info_index);
                        ++(found_info->counter);
                    } else {
                        // it's the first time we see the word within this article,
                        // add the extraInfoEntry to a vector of extraInfoEntries for a given searchResultEntry
                        VectorAppend(&(*found)->extraInfo, &info);
                        // sort by counter
                        VectorSort(&(*found)->extraInfo, ExtraInfoEntryCompareByCounter);
                    }
                    free(sre);
                    free(new_word);
                } else {
                    // it's the first time we see this word, add new searchResultEntry to words hashset
                    VectorNew(&sre->extraInfo, sizeof(extraInfoEntry), NULL, 4);
                    VectorAppend(&sre->extraInfo, &info);
                    HashSetEnter(words, &sre);
                }

                // semaphore signal for words hashset
                int res_post = sem_post(wSem);
                assert(res_post != -1);
            } else {
                ++numStopWords;
                free(new_word);
            }
        }
    }
  }

  printf("\tWe counted %d well-formed words [including duplicates].\n", numWords);
  printf("\tThe longest word scanned was \"%s\".\n", longestWord);
  printf("\tNumber of found stopwords: %d\n", numStopWords);
  if (strlen(longestWord) >= 15 && (strchr(longestWord, '-') == NULL))
      printf(" [Ooooo... long word!]");

  printf("\tTotal number of words in a hashset: %d\n", HashSetCount(words));
}

/**
 * Function: QueryIndices
 * ----------------------
 * Standard query loop that allows the user to specify a single search term, and
 * then proceeds (via ProcessResponse) to list up to 10 articles (sorted by relevance)
 * that contain that word.
 */

static void QueryIndices(const hashset *stopwords, const hashset *words, const vector *seenArticles)
{
  char response[1024];
  while (true) {
    printf("Please enter a single query term that might be in our set of indices [enter to quit]: ");
    fgets(response, sizeof(response), stdin);
    response[strlen(response) - 1] = '\0';
    if (strcasecmp(response, "") == 0) break;
    ProcessResponse(response, stopwords, words, seenArticles);
  }
}

/**
 * Function: ProcessResponse
 * -------------------------
 * Placeholder implementation for what will become the search of a set of indices
 * for a list of web documents containing the specified word.
 */
static const int numberOfEntriesToShow = 5;
static void ProcessResponse(const char *word, const hashset *stopwords, const hashset *words, const vector *seenArticles)
{
  if (WordIsWellFormed(word)) {
      char *query = strdup(word);
      // convert word to lowercase
      char *iter = query;
      for ( ; *iter; ++iter) *iter = tolower(*iter);

      // first, check if a word is not in a stopwords hashset
      if (HashSetLookup(stopwords, &query) == NULL) {
          // it's not a stop word, proceed
          searchResultEntry *sre = (searchResultEntry*)malloc(sizeof(searchResultEntry));
          assert(sre != NULL);
          sre->word = query;
          // try to find within words hashset
          searchResultEntry **found;
          if ((found = HashSetLookup(words, &sre)) != NULL) {
              // iterate over a vector printing the results
              int num_articles = VectorLength(&(*found)->extraInfo);
              printf("Nice! We found %d articles that include the word \"%s\".", num_articles, word);
              (num_articles > numberOfEntriesToShow) ? printf(" [We'll just list %d, though.]\n\n", numberOfEntriesToShow) : printf("\n\n");
              int num_entries = (num_articles > numberOfEntriesToShow) ? numberOfEntriesToShow : num_articles;
              for (int i = 0; i != num_entries; ++i) {
                  extraInfoEntry *info = (extraInfoEntry*) VectorNth(&(*found)->extraInfo, i);
                  article **a = (article**) VectorNth(seenArticles, info->articleID);
                  printf("%d.) \"%s\" [search term occurs %d times]\n    \"%s\"\n\n", i+1, (*a)->title, info->counter, (*a)->url);
              }
              free(sre);
          } else {
              // we haven't indexed this word
              printf("None of today's news articles contain the word %s\n", word);
              free(sre);
          }
      } else {
          // oops, it's a stopword
          printf("Too common a word to be taken seriously. Try something more specific.\n");
      }
      free(query);
  } else {
      printf("\tWe won't be allowing words like \"%s\" into our set of indices.\n", word);
  }
}

/**
 * Predicate Function: WordIsWellFormed
 * ------------------------------------
 * Before we allow a word to be inserted into our map
 * of indices, we'd like to confirm that it's a good search term.
 * One could generalize this function to allow different criteria, but
 * this version hard codes the requirement that a word begin with
 * a letter of the alphabet and that all letters are either letters, numbers,
 * or the '-' character.
 */

static bool WordIsWellFormed(const char *word)
{
  int i;
  if (strlen(word) == 0) return true;
  if (!isalpha((int) word[0])) return false;
  for (i = 1; i < strlen(word); i++)
    if (!isalnum((int) word[i]) && (word[i] != '-')) return false;

  return true;
}

/**
 *
 * Function to build stopwords hashset: BuildStopwordsHash
 * -------------------------------------------------------
 * Builds a hashset from a given text-file of stopwords - words that are too common
 * to count as good search terms.
 */

static void BuildStopwordsHash(const char *stopWordsFileName, hashset *stopwords)
{
  FILE *infile;
  char *buffer = NULL, *word = NULL;
  size_t len = 0;
  ssize_t read;
  infile = fopen(stopWordsFileName, "r");
  assert(infile != NULL);

  // read lines from the file, stripping off the newline char after each word
  while ((read = getline(&buffer, &len, infile)) != -1) {
      buffer = strsep(&buffer, "\n");
      word = strdup(buffer);
      HashSetEnter(stopwords, &word);
  }
  printf("\nNumber of elements in stopwords hashset: %d\n", HashSetCount(stopwords));
  free(buffer);
  fclose(infile);
}

/**
 * Hash function provided by the goddess of lecturing,
 * Julie Zelenski.  I'm not sure where it came from, but
 * I'm guessing the multiplier is standard.
 *
 * @param elem a void * which is understood to be the address
 *             of a char *, which itself addresses the first of
 *             a series of characters making up a C string.
 * @param numBuckets the number of buckets in the hash table.
 * @return the hashcode of the C string addressed by elem.
 */

static const signed long kHashMultiplier = -1664117991L;
static int StringHash(const void *elem, int numBuckets)
{
    char *s = *(char **) elem;
    unsigned long hashcode = 0;
    for (int i = 0; i < strlen(s); i++)
        hashcode = hashcode * kHashMultiplier + tolower(s[i]);
    return hashcode % numBuckets;
}

/**
 * Compares the two C strings planted at the specified addresses.
 * elem1 and elem2 are statically identified as void *s, but
 * we know that they're really char **s.  We cast and deferences
 * to arrive at char *s, and let strcmp do the traditional comparison
 * and use its return value as our own.
 *
 * @param elem1 the address of a char *, which itself addresses a null-terminated
 *              character array.
 * @param elem2 the address of a char *, just like elem1.
 * @return an integer representing the difference between the ASCII values of the
 *         first non matching characters, or 0 if the two strings are equal.
 */

static int StringCompare(const void *elem1, const void *elem2)
{
    return strcmp(*(const char **) elem1, *(const char **) elem2);
}

/**
 * Custom free function for malloc'ed strings
 *
 */

static void StringFree(void *elem)
{
    free(*(void**)elem);
}

/**
 * Custom free function for malloc'ed pthreads
 *
 */
static void PthreadFree(void *elem)
{
    free(*(void**)elem);
}

/**
 * VectorMap function to join a vector of pthreads
 *
 */
static void JoinPthreads(void *elem, void *aux)
{
    int res = pthread_join(**(pthread_t**)elem, NULL);
    assert(res == 0);
    fprintf(stdout, "Thread terminated with status %d\n", res);
}

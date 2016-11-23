# RSS News Search Revisited

```
$ make
$ ./rss-news-search [path/to/a/file/with/a/list/of/RSS-feeds.txt]
```

This multithreaded program indexes RSS news feeds and prompts user to search through the resulting database and uses custom vector and hashmap structs under the hood. Each article is downloaded in its own thread (I used POSIX threads). Thead synchronization is done with POSIX unnamed semaphores. The number of all active http connections is limited to 24 and the number of connections to the articles within one feed is capped at 7.

I used [libcurl](https://curl.haxx.se/libcurl/) and my own version of URLConnection struct to establish http connections.


```
Number of elements in stopwords hashset: 669

Welcome to the CS107 RSS Search Engine!
This handy tool pulls and processes a number of RSS (Rich Site Summary) pages,
and then allows for a simple search of those documents.  The application is
multithreaded where all feeds are processed in parallel (each feed is downloaded in
its own thread).


Number of parsed articles: 10

Total number of words in a hashset: 1824
Please enter a single query term that might be in our set of indices [enter to quit]: Trump
Nice! We found 10 articles that include the word "Trump". [We'll just list 5, though.]

1.) "Times reporters tweet news of Trump meeting as it happens" [search term occurs 22 times]
    "http://www.boston.com/news/politics/2016/11/22/times-reporters-tweet-news-of-trump-meeting-as-it-happens"

2.) "You can now watch Bernie Sanders's full Boston speech on identity politics and the progressive movement" [search term occurs 3 times]
    "http://www.boston.com/news/politics/2016/11/22/you-can-now-watch-bernie-sanderss-full-boston-speech-on-identity-politics-and-the-progressive-movement"

3.) "Employee found dead at Nashoba Valley Ski Area" [search term occurs 3 times]
    "http://www.boston.com/news/local-news/2016/11/22/employee-found-dead-at-nashoba-valley-ski-area"

4.) "Emerson students capture stunning helicopter video of Mass. lighthouses, fall foliage" [search term occurs 3 times]
    "http://www.boston.com/news/arts/2016/11/22/what-massachusettss-lighthouses-look-like-from-a-helicopter-in-the-fall"

5.) "Report: Parents beware of recalled toys for sale online" [search term occurs 2 times]
    "http://www.boston.com/news/local-news/2016/11/22/report-parents-beware-of-recalled-toys-for-sale-online"

```

Path to a list of RSS-feeds links is defined in `rss-news-search.c` or you can pass it as an argument to `rss-news-search`

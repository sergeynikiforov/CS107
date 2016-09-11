# RSS News Feed Aggregation

```
$ make
$ ./rss-news-search [path/to/a/file/with/a/list/of/RSS-feeds.txt]
```

The program indexes RSS news feeds and prompts user to search through the resulting database and uses custom vector and hashmap structs under the hood.

I used [libcurl](https://curl.haxx.se/libcurl/) and my own version of URLConnection struct to establish http connections.


```
Please enter a single query term that might be in our set of indices [enter to quit]: boston
Nice! We found 11 articles that include the word "boston". [We'll just list 5, though.]

1.) "Red Sox offense cant come through in 3-2 loss to Blue Jays" [search term occurs 17 times]
    "http://www.boston.com/sports/boston-red-sox/2016/09/10/red-sox-blue-jays"

2.) "Highlights: Boston College uses quick strikes to beat UMass 26-7" [search term occurs 17 times]
    "http://www.boston.com/sports/college-sports/2016/09/10/bc-umass-26-7"

3.) "The Mount Washington Observatory is offering a close look at winter" [search term occurs 15 times]
    "http://www.boston.com/culture/travel/2016/09/10/the-mount-washington-observatory-is-offering-a-close-look-at-winter"

4.) "Gisele Bundchen is helping Tom Brady train during his Deflategate suspension" [search term occurs 13 times]
    "http://www.boston.com/sports/new-england-patriots/2016/09/11/gisele-bundchen-is-helping-tom-brady-train-during-his-deflategate-suspension"

5.) "Clinton says deplorables comment is grossly generalistic" [search term occurs 10 times]
    "http://www.boston.com/news/politics/2016/09/10/hillary-clinton-says-she-regrets-calling-trump-supporters-basket-of-deplorables"

```

Path to a list of RSS-feeds links is defined in `rss-news-search.c` or you can pass it as an argument to `rss-news-search`

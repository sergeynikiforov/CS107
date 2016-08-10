#include <vector>
#include <list>
#include <set>
#include <string>
#include <iostream>
#include <iomanip>
#include "imdb.h"
#include "path.h"
using namespace std;

static void generateShortestPath(const string& source, const string& target, const imdb& db) {
  list<path> partialPaths;
  set<string> previouslySeenActors;
  set<film> previouslySeenFilms;

  // create a partial path around the source actor and add it to paths queue
  partialPaths.push_back(path(source));

  while (!partialPaths.empty() && (partialPaths.front().getLength() < 6)) {
    // pull off front path
    path frontPath(partialPaths.front());
    partialPaths.pop_front();

    // look through the last player's movies
    vector<film> actorFilms;
    if (db.getCredits(frontPath.getLastPlayer(), actorFilms)) {
        for (vector<film>::iterator film_iter = actorFilms.begin(); film_iter != actorFilms.end(); ++film_iter) {
          // for all unseen movies
          if (previouslySeenFilms.find(*film_iter) == previouslySeenFilms.end()) {
            // add to set of previously seen movies
            previouslySeenFilms.insert(*film_iter);

            // look up the film's cast
            vector<string> filmCast;
            if (db.getCast(*film_iter, filmCast)) {
              for (vector<string>::iterator cast_iter = filmCast.begin(); cast_iter != filmCast.end(); ++cast_iter) {
                // for all unseen actors
                if (previouslySeenActors.find(*cast_iter) == previouslySeenActors.end()) {
                  // add to previously seen actors
                  previouslySeenActors.insert(*cast_iter);

                  // clone the path
                  path clonedPath = frontPath;

                  // add connection
                  clonedPath.addConnection(*film_iter, *cast_iter);

                  // we found the target
                  if (*cast_iter == target) {
                    cout << clonedPath;
                    return;
                  } else {
                    partialPaths.push_back(clonedPath);
                  }
                }
              }
            }
          }
      }
    }
  }
  cout << endl << "No path between those two people could be found." << endl << endl;
}

/**
 * Using the specified prompt, requests that the user supply
 * the name of an actor or actress.  The code returns
 * once the user has supplied a name for which some record within
 * the referenced imdb existsif (or if the user just hits return,
 * which is a signal that the empty string should just be returned.)
 *
 * @param prompt the text that should be used for the meaningful
 *               part of the user prompt.
 * @param db a reference to the imdb which can be used to confirm
 *           that a user's response is a legitimate one.
 * @return the name of the user-supplied actor or actress, or the
 *         empty string.
 */

static string promptForActor(const string& prompt, const imdb& db)
{
  string response;
  while (true) {
    cout << prompt << " [or <enter> to quit]: ";
    getline(cin, response);
    if (response == "") return "";
    vector<film> credits;
    if (db.getCredits(response, credits)) return response;
    cout << "We couldn't find \"" << response << "\" in the movie database. "
	 << "Please try again." << endl;
  }
}

/**
 * Serves as the main entry point for the six-degrees executable.
 * There are no parameters to speak of.
 *
 * @param argc the number of tokens passed to the command line to
 *             invoke this executable.  It's completely ignored
 *             here, because we don't expect any arguments.
 * @param argv the C strings making up the full command line.
 *             We expect argv[0] to be logically equivalent to
 *             "six-degrees" (or whatever absolute path was used to
 *             invoke the program), but otherwise these are ignored
 *             as well.
 * @return 0 if the program ends normally, and undefined otherwise.
 */

int main(int argc, const char *argv[])
{
  imdb db(determinePathToData(argv[1])); // inlined in imdb-utils.h
  if (!db.good()) {
    cout << "Failed to properly initialize the imdb database." << endl;
    cout << "Please check to make sure the source files exist and that you have permission to read them." << endl;
    exit(1);
  }
  
  while (true) {
    string source = promptForActor("Actor or actress", db);
    if (source == "") break;
    string target = promptForActor("Another actor or actress", db);
    if (target == "") break;
    if (source == target) {
      cout << "Good one.  This is only interesting if you specify two different people." << endl;
    } else {
      generateShortestPath(source, target, db);
    }
  }
  
  cout << "Thanks for playing!" << endl;
  return 0;
}


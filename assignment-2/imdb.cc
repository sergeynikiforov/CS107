#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <cstring>
#include <iostream>
#include "imdb.h"

using namespace std;

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) ||
	    (movieInfo.fd == -1) );
}

/**
 * Returns void pointer to i-th Actor record (i is a given subscript)
 * helper to binary search in getActorRecord
 */
const void *imdb::getIthActorRecord(const unsigned int i) const {
    int offset = ((int*)actorFile)[i + 1];
    return (void*) &((char*)actorFile)[offset];
}

/**
 * Returns void pointer to i-th Movie record (i is a given subscript)
 */
const void *imdb::getIthMovieRecord(const unsigned int i) const {
    int offset = ((int*)movieFile)[i + 1];
    return (void*) &((char*)movieFile)[offset];
}

/**
 * Returns void pointer to Actor record in question
 * If the actor wasn't found, returns void ptr to actorFile
 */
const void *imdb::getActorRecord(const char* name) const {
    int left = 0, right = *(int*) actorFile - 1, middle = (right + left)/2;
    while (left <= right) {
      char* current_name = (char*) getIthActorRecord(middle);
      int compare_result = strcmp(name, current_name);
      if (compare_result == 0)
        return (void*) current_name;
      if (compare_result < 0) {
        right = middle - 1;
        middle = (right + left)/2;
      }
      if (compare_result > 0) {
        left = middle + 1;
        middle = (right + left)/2;
      }
    }
    return actorFile;
}

/**
 * Returns void pointer to Actor record in question
 * If the actor wasn't found, returns void ptr to actorFile
 */
const void *imdb::getMovieRecord(const film& movie) const {
    int left = 0, right = *(int*) movieFile - 1, middle = (right + left)/2;
    while (left <= right) {
      const void *curr_movie_ptr = getIthMovieRecord(middle);
      string curr_movie_title((char*) curr_movie_ptr);
      //cout << curr_movie_title << " - ";
      int curr_movie_year = 1900 + *((char*) curr_movie_ptr + curr_movie_title.size() + 1);
      //cout << curr_movie_year << endl;
      film curr_movie = {curr_movie_title, curr_movie_year};
      if (curr_movie == movie) {
        //cout << "Found: " << curr_movie.title << endl;
        return curr_movie_ptr;
      }
      if (movie < curr_movie) {
        right = middle - 1;
        middle = (right + left)/2;
      } else {
        left = middle + 1;
        middle = (right + left)/2;
      }
    }
    return movieFile;
}

/**
 * Populates vector of films where a given player acted
 * Returns true if the player has been found, false otherwise
 *
 */
bool imdb::getCredits(const string& player, vector<film>& films) const {
    // get a ptr to Actor Record
    const void *actor_record = getActorRecord(player.c_str());

    // return false if no Actor has been found
    if (actor_record == actorFile)
      return false;

    // get to the num of films starred
    char *ptr = (char*) actor_record;

    // length to check if additional padding is needed after name+num_films
    short length = 1;
    while (*ptr++ != '\0')
      ++length;

    // check for double '\0'
    if (*ptr == '\0') {
      ++ptr;
      ++length;
    }

    // get num of films, set right after
    short num_films = *(short*)ptr;

    // set offset depending on whether length + 2 is a multiple of 4
    int *offset = ((length + 2) % 4 == 0) ? (int*)(((short*)ptr) + 1) : (int*)(((short*)ptr) + 2);

    // iterate over offsets to movieFile, populate vector of films
    for (short i = 0; i != num_films; ++i, ++offset) {
      string movie_title(((char*) movieFile) + (*offset));
      int movie_year = 1900 + *(((char*) movieFile) + (*offset) + movie_title.size() + 1);
      film tmp = { movie_title, movie_year };
      films.push_back(tmp);
    }
    return true ;
}

/**
 * Populates vector of strings (players) - full cast for a given movie
 * Returns true if the movie has been found, false otherwise
 *
 */
bool imdb::getCast(const film& movie, vector<string>& players) const {
  // get a ptr to the movie in question
  const void *movie_ptr = getMovieRecord(movie);

  // return false if the movie wasn't found
  if (movie_ptr == movieFile)
    return false;

  // get to the array of offsets
  short offset_to_number_of_actors = movie.title.size() + 2 + (movie.title.size() + 2) % 2;
  short *ptr = (short*) (((char*) movie_ptr) + offset_to_number_of_actors);

  // check if additional 2 bytes were used for padding
  int *arr_of_offsets = ((offset_to_number_of_actors + 2) % 4 == 0) ? (int*)(ptr + 1) : (int*) (ptr + 2);
  short num_actors = *ptr;

  // iterate over the array, populating players vector
  for (int i = 0; i != num_actors; ++i, ++arr_of_offsets) {
    string actor_name(((char*) actorFile) + (*arr_of_offsets));
    players.push_back(actor_name);
  }

  return true;
}

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM..
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
  return info.fileMap;
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}

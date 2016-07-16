/**
 * File: rsg.cc
 * ------------
 * Provides the implementation of the full RSG application, which
 * relies on the services of the built-in string, ifstream, vector,
 * and map classes as well as the custom Production and Definition
 * classes provided with the assignment.
 */
 
#include <map>
#include <string>
#include <fstream>
#include "definition.h"
#include "production.h"
using namespace std;

/**
 * Takes a reference to a legitimate infile (one that's been set up
 * to layer over a file) and populates the grammar map with the
 * collection of definitions that are spelled out in the referenced
 * file.  The function is written under the assumption that the
 * referenced data file is really a grammar file that's properly
 * formatted.  You may assume that all grammars are in fact properly
 * formatted.
 *
 * @param infile a valid reference to a flat text file storing the grammar.
 * @param grammar a reference to the STL map, which maps nonterminal strings
 *                to their definitions.
 */

static void readGrammar(ifstream& infile, map<string, Definition>& grammar)
{
  while (true) {
    string uselessText;
    getline(infile, uselessText, '{');
    if (infile.eof()) return;  // true? we encountered EOF before we saw a '{': no more productions!
    infile.putback('{');
    Definition def(infile);
    grammar[def.getNonterminal()] = def;
  }
}

/**
 * Helper recursive function to expand a given non-terminal into terminals 
 * and append them to a resulting vector of terminals
 *
 * @param nonterminal: const reference to string denoting a possible non-terminal from a grammar
 * @param grammar: reference to map from string to Definition that stores grammar rules
 * @param terminals: reference to vector of strings that's being populated with terminals from the CFG
 */

static void expandNonTerminal(const string& nonterminal, map<string, Definition>& grammar, vector<string>& terminals)
{
  // check if nonterminal is really a non-terminal
  if (grammar.find(nonterminal) == grammar.end())
    terminals.push_back(nonterminal);
  else {
    Production prod = grammar[nonterminal].getRandomProduction();
    for (Production::iterator curr = prod.begin(); curr != prod.end(); ++curr)
      expandNonTerminal(*curr, grammar, terminals);
  }
}

/**
 * Given a valid reference to an STL map storing the grammar (string->Definition), 
 * generates random sequence of terminals in STL's vector.
 * 
 * @param grammar: reference to map from string to Definition that stores grammar rules
 * @param terminals: reference to vector of strings that's being populated with terminals from the CFG
 */

static void generateTerminals(map<string, Definition>& grammar, vector<string>& terminals)
{
  // start with <start>
  expandNonTerminal(string("<start>"), grammar, terminals);
}

/**
 * Given a vector of terminals (strings), pretty-prints it into a terminal
 *
 * @param terminals: const ref to vector of strings containing sequence of terminals
 */

static void printTerminals(const vector<string>& terminals)
{
  cout << "     ";
  unsigned int len = 5, limit = 55;
  for (vector<string>::const_iterator curr = terminals.begin(); curr != terminals.end(); ++curr) {
    vector<string>::const_iterator next = curr + 1;
    if (next != terminals.end() && \
	*next != "." && *next != "!" && *next != "," && *next != "?" && *next != ";") { 
      // next terminal is not a punctuation char
      if (len + 1 + curr->size() > limit) {
	cout << "\n" << *curr << " ";
	len = curr->size() + 1;
      } else {
	cout << *curr << " ";
	len = len + (curr->size() + 1);
      }
    } else {
      // next terminal is punctuation
      cout << *curr;
      len = len + curr->size();
    }  
  } 
  cout << endl;
}

/**
 * Prints to terminal n expansions from a given grammar
 *
 * @param n: number of expansions
 * @param grammar: reference to map from string to Definition that stores grammar rules
 */
static void getNExpansions(const unsigned int n, map<string, Definition>& grammar)
{
  for (unsigned int i = 0; i != n; ++i) {
    cout << "Version #" << i + 1 << ": -----------------------\n";
    vector<string> result;
    generateTerminals(grammar, result);
    printTerminals(result);
    cout << endl;
  }
}

/**
 * Performs the rudimentary error checking needed to confirm that
 * the client provided a grammar file.  It then continues to
 * open the file, read the grammar into a map<string, Definition>,
 * and then print out the total number of Definitions that were read
 * in.  You're to update and decompose the main function to print
 * three randomly generated sentences, as illustrated by the sample
 * application.
 *
 * @param argc the number of tokens making up the command that invoked
 *             the RSG executable.  There must be at least two arguments,
 *             and only the first two are used.
 * @param argv the sequence of tokens making up the command, where each
 *             token is represented as a '\0'-terminated C string.
 */

int main(int argc, char *argv[])
{
  if (argc == 1) {
    cerr << "You need to specify the name of a grammar file." << endl;
    cerr << "Usage: rsg <path to grammar text file>" << endl;
    return 1; // non-zero return value means something bad happened 
  }
  
  ifstream grammarFile(argv[1]);
  if (grammarFile.fail()) {
    cerr << "Failed to open the file named \"" << argv[1] << "\".  Check to ensure the file exists. " << endl;
    return 2; // each bad thing has its own bad return value
  }
  
  // things are looking good...
  map<string, Definition> grammar;
  readGrammar(grammarFile, grammar);
  
  cout << "The grammar file called \"" << argv[1] << "\" contains "
       << grammar.size() << " definitions." << endl;

  // get 3 expansions
  getNExpansions(3, grammar);
  
  return 0;
}

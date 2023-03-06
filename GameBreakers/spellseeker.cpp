// CS 307 - Jim Plank
// Mason Stott
// 4-25-22
// TA's: Ankush / ChaoHui

#include <string>
#include <vector>
#include <list>
#include <cmath>
#include <algorithm>
#include <map>
#include <set>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <fstream>
#include <sstream>
using namespace std;

// Class for DFS
class Node
{
  public:
    char value;           // The char value of the corresponding cell
    int r, c;             // The coordinate of the cell in the grid
    bool visited;
    vector <Node *> adj;  // Adjacent Nodes
};

// Class for the answer
class Answer
{
  public:
    int length;       // Max pathlength
    int r;
    int c;
    string key;       // String that represents the grid (used for the cache)
    Answer *nexta;    // Null if no path forward, otherwise next node in
                      // the longest path
};

// Class for the game
class Spellseeker
{
  public:
    ~Spellseeker();
    vector <string> grid;                       // Used for everything but key generation
    vector < vector <Node *> > keygraph;        // Used solely for key generation (DFS)
    map <string, Answer *> cache;
    int total_r, total_c;
    vector <Answer *> answers;                 // Stores all answers for memory cleanup

    void InitKeyGraph();
    string CalcKey(const int r, const int c);
    void FindReachables(const int r, const int c, string &key);  // The DFS for key gen, sets the corresponding char in the key
    
    Answer *Solve(const int r, const int c);
    Answer *ProcessNeighbors(const int r, const int c, const char curr);
    bool CheckDiff(const int r, const int c, const char curr);
};

// Destructor
Spellseeker::~Spellseeker()
{
  // Free all memory
  for (size_t i = 0; i < answers.size(); i++) delete answers[i];
  for (size_t i = 0; i < keygraph.size(); i++)
  {
    for (size_t j = 0; j < keygraph[i].size(); j++) delete keygraph[i][j];
  }
}

// Helper function for checking the difference of two elements in the grid
bool Spellseeker::CheckDiff(const int r, const int c, const char curr)
{
  char val1, val2;
  // Set the values
  val1 = grid[r][c];
  val2 = curr;
  // Ignore '-'
  if (val1 == '-' || val2 == '-') return 0;
  // Compare the values and return
  return (abs(val1 - val2) == 1);
}

// Function for initializing the graph for DFS. Call only once in main()
void Spellseeker::InitKeyGraph()
{
  Node *curr;

  // Create all of the nodes
  keygraph.resize(total_r);
  for (int i = 0; i < total_r; i++) for (int j = 0; j < total_c; j++) keygraph[i].push_back(new Node);

  for (int r = 0; r < total_r; r++)
  {
    for (int c = 0; c < total_c; c++)
    {
      // Set the values for the current node
      curr = keygraph[r][c];
      curr->value = grid[r][c];
      curr->r = r;
      curr->c = c;
      /* All of these if statements set the adjacency list */
      // Above
      if (r - 1 >= 0) curr->adj.push_back(keygraph[r - 1][c]);
      // Below
      if (r < (int) total_r - 1) curr->adj.push_back(keygraph[r + 1][c]);
      if (c % 2 == 0)
      {
        if (c - 1 >= 0)
        {
          // Left
          curr->adj.push_back(keygraph[r][c - 1]);
          // Below and Left
          if (r < (int) total_r - 1) curr->adj.push_back(keygraph[r + 1][c - 1]);
        }
        if (c < (int) total_c - 1)
        {
          // Right
          curr->adj.push_back(keygraph[r][c + 1]);
          // Below and right
          if (r < (int) total_r - 1) curr->adj.push_back(keygraph[r + 1][c + 1]);
        }
      }
      else
      {
        if (c - 1 >= 0)
        {
          // Left
          curr->adj.push_back(keygraph[r][c - 1]);
          // Above and left
          if (r - 1 >= 0) curr->adj.push_back(keygraph[r - 1][c - 1]);
        }
        if (c < (int) total_c - 1)
        {
          // Right
          curr->adj.push_back(keygraph[r][c + 1]);
          // Above and right
          if (r - 1 >= 0) curr->adj.push_back(keygraph[r - 1][c + 1]);
        }
      }
    }
  }
}

// The DFS for key generation
void Spellseeker::FindReachables(const int r, const int c, string &key)
{
  Node *curr, *next;
  
  curr = keygraph[r][c];

  // Don't do anything if we've been here
  if (curr->visited) return;
  // Mark the node as reachable if this isn't the first call
  if (key[r * total_c + c] != 'X') key[r * total_c + c] = 'O';
  // Mark this cell as visited
  curr->visited = 1;
  // Base case: if we have nowhere to go
  // cout << curr->adj.empty() << endl;
  if (curr->adj.empty()) return;
  // Recursion for all the valid adjacent cells
  for (size_t i = 0; i < curr->adj.size(); i++)
  {
    next = curr->adj[i];
    if (abs(curr->value - next->value) == 1) FindReachables(next->r, next->c, key);
  }
  // Reset the visited field
  curr->visited = 0;
}

// Helper function for generating a key from the grid
string Spellseeker::CalcKey(const int r, const int c)
{
  string key;
  
  // Initialize key
  key.resize(total_r * total_c, '-');
  key[(r * total_c) + c] = 'X';
  // Perform the DFS to set the key properly
  FindReachables(r, c, key);
  return key;
}

// Helper function for performing the recusion/validating adjacent nodes
Answer *Spellseeker::ProcessNeighbors(const int r, const int c, const char curr)
{
  set <Answer *> paths;
  set <Answer *>::iterator pit;
  Answer *rv;

  // Above
  if (r - 1 >= 0 && CheckDiff(r - 1, c, curr)) paths.insert(Solve(r - 1, c));
  // Below
  if (r < (int) total_r - 1 && CheckDiff(r + 1, c, curr)) paths.insert(Solve(r + 1, c));
  if (c % 2 == 0)
  {
    if (c - 1 >= 0)
    {
      // Left
      if (CheckDiff(r, c - 1, curr)) paths.insert(Solve(r, c - 1));
      // Below and Left
      if (r < (int) total_r - 1 && CheckDiff(r + 1, c - 1, curr)) paths.insert(Solve(r + 1, c - 1));
    }
    if (c < (int) total_c - 1)
    {
      // Right
      if (CheckDiff(r, c + 1, curr)) paths.insert(Solve(r, c + 1));
      // Below and right
      if (r < (int) total_r - 1 && CheckDiff(r + 1, c + 1, curr)) paths.insert(Solve(r + 1, c + 1));
    }
  }
  else
  {
    if (c - 1 >= 0)
    {
      // Left
      if (CheckDiff(r, c - 1, curr)) paths.insert(Solve(r, c - 1));
      // Above and left
      if (r - 1 >= 0 && CheckDiff(r - 1, c - 1, curr)) paths.insert(Solve(r - 1, c - 1));
    }
    if (c < (int) total_c - 1)
    {
      // Right
      if (CheckDiff(r, c + 1, curr)) paths.insert(Solve(r, c + 1));
      // Above and right
      if (r - 1 >= 0 && CheckDiff(r - 1, c + 1, curr)) paths.insert(Solve(r - 1, c + 1));
    }
  }
  // Return null if there are no paths
  if (paths.empty()) return NULL;
  // Have to initialize rv
  rv = *(paths.begin());
  // Make rv the largest and return
  for (pit = paths.begin(); pit != paths.end(); pit++) if ((*pit)->length > rv->length) rv = *pit;
  return rv;
}

// Function to find the longest path
Answer *Spellseeker::Solve(const int r, const int c)
{
  char curr = grid[r][c];
  string key;
  Answer *temp, *path;

  // Calculate the key
  //key = CalcKey(r, c);
  // If it's in the cache, return it
  //if (cache.find(key) != cache.end()) return (cache.find(key)->second);
  // Otherwise, create a new answer
  temp = new Answer;
  temp->key = key;
  temp->c = c;
  temp->r = r;
  temp->length = 1;
  // Store temp for garbage collection
  answers.push_back(temp);
  // Remove the current element for the next calls
  grid[r][c] = '-';
  // Call the recursion on the valid adjacent cells
  path = ProcessNeighbors(r, c, curr);
  // Reset the current element
  grid[r][c] = curr;
  // Base case
  if (path == NULL) 
  {
    temp->nexta = NULL;
    return temp;
  }
  // Set the nexta to the longest of the recursions
  temp->nexta = path;
  temp->length = path->length + 1;
  // Cache temp and return nexta
  //cache.insert(make_pair(key, temp));
  return temp;
}

// Main
int main()
{
  Spellseeker seeker;
  string s;
  Answer *max, *curr;

  // Read in the graph
  while (getline(cin, s))
  {
    printf("%s\n", s.c_str());
    seeker.grid.push_back(s);
    s.clear();
  }
  printf("PATH\n");
  // Set total_r and total_c
  seeker.total_r = seeker.grid.size();
  if (seeker.total_r != 0) seeker.total_c = seeker.grid[0].size();
  // Set up the graph for key generation
  seeker.InitKeyGraph();
  // Call Solve() on all elements
  for (int i = 0; i < seeker.total_r; i++)
  {
    for (int j = 0; j < seeker.total_c; j++)
    {
      curr = seeker.Solve(i, j);
      if (curr->length > max->length || max == NULL) max = curr;
    }
  }
  // Output the path
  curr = max;
  while (curr != NULL)
  {
    printf("%d %d\n", curr->r, curr->c);
    curr = curr->nexta;
  }
  return 0;
}
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

/*
  HOW TO PLAY
  ---------------------------
  Our goal is to find the longest path through the cells
  such that adjacent nodes in the path have values that differ
  by exactly 1. If there are multiple longest paths, finding just
  1 will do.

  Cells are to be indexed by [row][column], where row zero is
  at the top and column zero is at the left.

  Formally, 2 cells are adjacent if:
    - They are in the same column and their row numbers differ by one.              - DONE
    - They are in the same row and their column numbers differ by one.              - DONE
    - If c is even, then (r,c) is adjacent to (r+1,c+1) and (r+1,c-1).              - DONE
    - Similarly, if c is odd, then (r,c) is adjacent to (r-1,c+1) and (r-1,c-1).    - DONE

    GOAL OF SPELLPATH : Simply read in the grid on stdin and print the length of
    the longest path. Don't need to use memoization (can be relatively slow).
    ---------------------------
    GOAL OF SPELLSEEKER : Same as Spellpath but instead of the length we print the actual path,
    so will need to do BFS or DFS. Needs to use memoization.
*/

// Class for the game
class Spellseeker
{
  public:
    vector <string> grid;
    int MaxPathLength(const int r, const int c);
    set <int> ProcessAdjacents(const int r, const int c, const char curr);
    bool CheckDiff(const int r1, const int c1, const char curr);
    // Making a cache for this would be trivial, but I don't have to according
    // to Papa Plank
};

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

// Helper function for performing the recusion/validating adjacent nodes
set <int> Spellseeker::ProcessAdjacents(const int r, const int c, const char curr)
{
  set <int> output;

  // Above
  if (r - 1 >= 0 && CheckDiff(r - 1, c, curr)) output.insert(MaxPathLength(r - 1, c));
  // Below
  if (r < (int) grid.size() - 1 && CheckDiff(r + 1, c, curr)) output.insert(MaxPathLength(r + 1, c));
  if (c % 2 == 0)
  {
    if (c - 1 >= 0)
    {
      // Left
      if (CheckDiff(r, c - 1, curr)) output.insert(MaxPathLength(r, c - 1));
      // Below and Left
      if (r < (int) grid.size() - 1 && CheckDiff(r + 1, c - 1, curr)) output.insert(MaxPathLength(r + 1, c - 1));
    }
    if (c < (int) grid[0].size() - 1)
    {
      // Right
      if (CheckDiff(r, c + 1, curr)) output.insert(MaxPathLength(r, c + 1));
      // Below and right
      if (r < (int) grid.size() - 1 && CheckDiff(r + 1, c + 1, curr)) output.insert(MaxPathLength(r + 1, c + 1));
    }
  }
  else
  {
    if (c - 1 >= 0)
    {
      // Left
      if (CheckDiff(r, c - 1, curr)) output.insert(MaxPathLength(r, c - 1));
      // Above and left
      if (r - 1 >= 0 && CheckDiff(r - 1, c - 1, curr)) output.insert(MaxPathLength(r - 1, c - 1));
    }
    if (c < (int) grid[0].size() - 1)
    {
      // Right
      if (CheckDiff(r, c + 1, curr)) output.insert(MaxPathLength(r, c + 1));
      // Above and right
      if (r - 1 >= 0 && CheckDiff(r - 1, c + 1, curr)) output.insert(MaxPathLength(r - 1, c + 1));
    }
  }
  return output;
}

// Function to find the longest path's length
int Spellseeker::MaxPathLength(const int r, const int c)
{
  set <int> lengths;
  char curr = grid[r][c];

  // Remove the current element for the next calls
  grid[r][c] = '-';
  // Call the recursion on the valid adjacent cells
  lengths = ProcessAdjacents(r, c, curr);
  // Reset the current element
  grid[r][c] = curr;
  // Base case
  if (lengths.empty()) return 1; 
  // Return the greatest path length from this node + 1
  return *(lengths.rbegin()) + 1;
}

// Main
int main()
{
  Spellseeker seeker;
  string s;
  int max = -1,
      curr;

  // Read in the graph
  while (getline(cin, s)) seeker.grid.push_back(s);
  // Call MaxPathLength() on all elements
  for (size_t i = 0; i < seeker.grid.size(); i++)
  {
    for (size_t j = 0; j < seeker.grid[0].size(); j++)
    {
      curr = seeker.MaxPathLength(i, j);
      if (curr > max) max = curr;
    }
  }
  // Output the maximum path length
  cout << max << endl;
  return 0;
}
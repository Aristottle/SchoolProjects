// CS 307 - Jim Plank
// Mason Stott
// 3-30-22
// TA's: Ankush / ChaoHui 

#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
using namespace std;

/*
  These are all of the necessary classes for our graph (copied initially from the lecture notes)
*/

// Class for the nodes of the graph
class Node 
{
  public:
    vector <int> edges;     // Adjacency list, holding the 
                            // numbers of the nodes to which
                            // this node is connected.
    int status;             // Denotes whether the node is visited (0), a wall (1), or unvisited (-1)
};

// Class for the graph itself
class Graph 
{
  public:
    vector <Node *> nodes;                    // All of the nodes.  nodes[i]->number is i
                                              // Note that it is a vector of pointers.

    vector <int> path;                        // Stores the solution path for the maze

    int rows, cols;                           // The number of rows and columns in the graph
    
    void Create_Edges();                                           // Creates the edges of the graph and accounts for walls we read on std input
    void Solve_Maze(int index, int end);                           // Finds a path through the maze via a DFS
    void Print() const;                                            // Print the final output if/when we've solved the maze
    ~Graph();                                                      // Destructor
};

// Function for setting the edges of the node. ONLY call after the graph has already been initialized
void Graph::Create_Edges()
{
  int index, cell_1, cell_2;
  string s;

  // Create all edges
  for (size_t i = 0; i < rows; i++)
  {
    for (size_t j = 0; j < cols; j++)
    {
      index = i * cols + j;

      // Check above
      if (i != 0) nodes[index]->edges.push_back((i - 1) * cols + j);
      // Check to left
      if (j != 0) nodes[index]->edges.push_back(i * cols + j - 1);
      // Check below
      if (i != rows - 1) nodes[index]->edges.push_back((i + 1) * cols + j);
      // Check to right
      if (j != cols - 1) nodes[index]->edges.push_back(i * cols + j + 1);
    }
  }

  // Fix edges based on the walls we read in
  while (cin >> s)
  {
    // Read the wall
    cin >> cell_1 >> cell_2;
    // Output the wall
    printf("WALL %d %d\n", cell_1, cell_2);

    // Set to zero to negate any relevance of the entry
    for (size_t i = 0; i < nodes[cell_1]->edges.size(); i++) if (nodes[cell_1]->edges[i] == cell_2) nodes[cell_1]->edges[i] = 0;
    for (size_t i = 0; i < nodes[cell_2]->edges.size(); i++) if (nodes[cell_2]->edges[i] == cell_1) nodes[cell_2]->edges[i] = 0;
  }

  return;
}

// Function for solving the maze via a DFS
void Graph::Solve_Maze(int index, int end)
{
  Node *n = nodes[index];

  // Base case: if we've reached the end of the maze
  if (index == end)
  {
    path.push_back(index);
    Print();
    return;
  }

  // Check if we've visited already (or if a wall)
  if (n->status != -1) return;

  // Say we've visited this node
  n->status = 0; 
  
  // Add the node to the path
  path.push_back(index);

  // Recursion on all of the adjacent nodes
  for (size_t i = 0; i < n->edges.size(); i++) Solve_Maze(n->edges[i], end);

  // We only make it here if the path fails, so remove from the path vector
  path.pop_back();
}

// Function for printing the output and the path
void Graph::Print() const
{
  // Print the path
  for (size_t i = 0; i < path.size(); i++) printf("PATH %d\n", path[i]);

  // Since exit(0) bypasses destructors, we have to call it here manually
  this->~Graph();
  exit(0);
}

// Graph destructor
Graph::~Graph()
{
  // Clear memory
  for (size_t i = 0; i < nodes.size(); i++) delete nodes[i];
}

// MAIN
int main()
{
	string s;
  Node *n;
  Graph g;
  
  // Get rows and columns
  cin >> s >> g.rows >> s >> g.cols;
  // Output Rows and Cols
  printf("ROWS %d COLS %d\n", g.rows, g.cols);

  // Initialize the graph
  for (size_t i = 0; i < g.rows * g.cols; i++)
  {
    n = new Node;
    n->status = -1;
    g.nodes.push_back(n);
  }

  // Set all of the edges in the graph
  g.Create_Edges();

  // Solve the maze if possible
  g.Solve_Maze(0, g.rows * g.cols - 1);

  return 0;
}
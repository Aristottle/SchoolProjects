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

// Class for the simulator
class Calc
{
  public:
	  double Calc_Prob(const double &s, int t, int lr);
    vector < vector <double> > cache;                   // Initialized to -1, coords are [t][lr]
};

// The recursion
double Calc::Calc_Prob(const double &s, int t, int lr)
{
	double  prob_success = 0, 
          sum = 0;

  // Base case: we return 0 because adding 0 changes nothing
  if (t == 0) return 0;
  // Special case for lr == -1 and t = 1 because I'm lazy
  if (t == 1 && lr == -1) return 1;
  // Check the cache before doing any repition
  if (lr != -1 && cache[t][lr] != -1) return cache[t][lr];
  // prob_success is only dependendant on whether lr was the firstly or lastly numbered side
  prob_success = (lr == 0 || lr == s - 1) ? ((s - 2) / s) : ((s - 3) / s);
  // Sum all of the probabilities for the legal rolls
  for (int i = 0; i < s; i++) if (abs(lr - i) > 1 || lr == -1) sum += Calc_Prob(s, t - 1, i);
  // If sum is 0, we just want the probability of the current roll, not it's children
  if (sum > 0) prob_success = sum / s;
  // Cache and return the probability
  if (lr != -1) cache[t][lr] = prob_success;
  return prob_success;
}

// Main
int main(int argc, char** argv)
{
  int s, t, lr;
  Calc calc;

  // Validate
  if (argc != 4) return -1;
  // Store the input
  s = atoi(argv[1]);
  t = atoi(argv[2]);
  lr = atoi(argv[3]);
  // Initialize the cash
  calc.cache.resize(t + 1, vector <double>(s, -1));
  // Perform and output the calculation
  cout << calc.Calc_Prob(s, t, lr) << endl;
  return 0;
}
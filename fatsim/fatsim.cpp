// -------------------------------------------------------------------------------------
// this is the only file you need to edit
// -------------------------------------------------------------------------------------
//
// (c) 2023, Pavol Federl, pfederl@ucalgary.ca
// Do not distribute this file.

#include "fatsim.h"
#include <iostream>
std::vector<bool> visited; // We do depth first traversal but we claim that a node is visited provided we visit it
std::vector<int> depths; // Attempting to measure the depth via a global variable since pair is likely slowing down the code
std::vector<long> DFS(std::vector<std::vector<long>> &adj_list, int start, int depth){
  visited[start] = true; // We can say we have visited any given node before we recursively enter it
  int max_depth = 0;
  // printf("node %d with depth %d\n", start, depth);
  std::vector<long> temp;
  std::vector<long> deepest;
  for(long node : adj_list[start]){ // Whatever our node it, we go down in depth
    if(visited[node]) continue;
    std::vector<long> temp = DFS(adj_list, node, depth+1);
    // printf("%d is a node, and %d is the depth\n", (int)temp[0], depths[temp[0]]);
    if(!temp.empty()){
      if(max_depth < depths[temp[0]]){ // We can use any element because the depth of all things in temp are equal
      max_depth = depths[temp[0]];
      deepest.clear();
      deepest.insert(deepest.end(), temp.begin(), temp.end());
      }
      else if(max_depth == depths[temp[0]]) {
        deepest.insert(deepest.end(), temp.begin(), temp.end()); // If they are the same depth then we want both nodes
      }
    }
  }
  if(!deepest.empty()) return deepest;
  depths[start] = depth;
  return {start}; // If you never entered the for loop, it was a node with no other nodes, and therefore can be returned as a deepest node
}

// // reimplement this function
std::vector<long> fat_check(const std::vector<long> & fat)
{
 visited.resize(fat.size());
 depths.resize(fat.size());
 std::vector<std::vector<long>> adj_list(fat.size()+1); // Store adjecent vectors (size+1 is -1 node). Bool included for visited that will be used for a DFS

  for(int i = 0; i < (long)fat.size(); i++){ // This gets all nodes with their adjacent nodes
    if(fat[i] == -1){
      adj_list[fat.size()].push_back(i);
      continue;
    }
    adj_list[fat[i]].push_back(i);
  }
  return DFS(adj_list, fat.size(), 0);
}
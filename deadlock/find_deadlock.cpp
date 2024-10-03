// this is the ONLY file you should edit and submit to D2L

#include "find_deadlock.h"
#include "common.h"
#include <iostream>
#include <vector>

/// this is the function you need to (re)implement
///
/// parameter edges[] contains a list of request- and assignment- edges
///   example of a request edge, process "p1" resource "r1"
///     "p1 -> r1"
///   example of an assignment edge, process "XYz" resource "XYz"
///     "XYz <- XYz"
///
/// You need to process edges[] one edge at a time, and run a deadlock
/// detection after each edge. As soon as you detect a deadlock, your function
/// needs to stop processing edges and return an instance of Result structure
/// with 'index' set to the index that caused the deadlock, and 'procs' set
/// to contain names of processes that are in the deadlock.
///
/// To indicate no deadlock was detected after processing all edges, you must
/// return Result with index=-1 and empty procs.

//Adapted from hints;
class Graph{
    public:
        std::vector<std::vector<int>> adj_list;
        std::vector<int> out_counts;
        // unordered_map<string,vector<string>> adj_list;
        // unordered_map<string, int> out_counts;
}graph;
Result find_deadlock(const std::vector<std::string> & edges)
{
    // Create variables
    Result result;
    Graph graph;
    Word2Int keys;
    std::vector<std::string> revert;
    
  
    // For each edge
    for(int i = 0; i < (long)edges.size(); i++){
        // Divide the parts of the edge string into process, ->/<-, and resource
        std::vector<std::string> splitted_tokens = split(edges[i]);
        // Append ! to the process (to differentiate between processes and resources with the same name and confirm if something is a process)
        splitted_tokens[0].append("!");
        // Convert the process and resource into respective integers
        int processNum = keys.get(splitted_tokens[0]);
        int resourceNum = keys.get(splitted_tokens[2]);


       if((int)graph.out_counts.size() ==  processNum){
            revert.push_back(splitted_tokens.at(0));
            graph.out_counts.push_back(0);
            std::vector<int>p{};
            graph.adj_list.push_back(p);
        }
        if((int)graph.out_counts.size() == resourceNum){
            revert.push_back(splitted_tokens.at(2));
            graph.out_counts.push_back(0);
            std::vector<int>p {};
            graph.adj_list.push_back(p);
        }
        
        // Place going into coming for toposort
        // If the splitted token is a request edge
        if(splitted_tokens[1].compare("->")==0){
            // Add process to adjacents of request
            graph.adj_list[resourceNum].push_back(processNum);
            graph.out_counts[processNum]++;
        }
        // If the splitted token is an assignment edge
        else{
            // Add request to adjacents of process
            graph.adj_list[processNum].push_back(resourceNum);
            graph.out_counts[resourceNum]++;              
        }

        // Out is the out counts currently in our Graph
        std::vector<int> out = graph.out_counts;
        std::vector<int> zeroes;
        // For each int in out
        for(int i=0; i<(long)out.size();i++){
            if(out[i]==0) zeroes.push_back(i);
        }
        // If zeroes has something in it
        while(!zeroes.empty()){ 
            // Pop an int off the stack in zeroes
            int n = zeroes.back();
            zeroes.pop_back();
            // For the corresponding adjacent list in zeroes
            for(int n2 : graph.adj_list[n] ){
                out[n2]--;
                if(out[n2]==0) zeroes.push_back(n2);
            }
        }
        // processes involved in deadlock are nodes n that represent a process and out[n]>0
        for(int i=0; i<(long)out.size();i++){
            if(out[i]!=0){

                if(revert[i].back()=='!') {
                    revert[i].pop_back();
                    result.procs.push_back(revert[i]);
                }
            }
        }
        // If there is a process in results then it is a deadlock
        if(!result.procs.empty()){
            result.index = i;
            return result;
        }
    }
    // If we never detect a deadlock we return -1
    result.index = -1;
    return result;
}
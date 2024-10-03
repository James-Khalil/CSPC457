// this is the only file you should modify and submit for grading

#include "scheduler.h"
#include "common.h"
#include "iostream"


// this is the function you should implement
//
// runs Round-Robin scheduling simulator
// input:
//   quantum = time slice
//   max_seq_len = maximum length of the reported executing sequence
//   processes[] = list of process with populated IDs, arrivals, and bursts
// output:
//   seq[] - will contain the execution sequence but trimmed to max_seq_len size
//         - idle CPU will be denoted by -1
//         - other entries will be from processes[].id
//         - sequence will be compressed, i.e. no repeated consecutive numbers
//   processes[]
//         - adjust finish_time and start_time for each process
//         - do not adjust other fields
//

void simulate_rr(
    int64_t quantum, 
    int64_t max_seq_len,
    std::vector<Process> & processes,
    std::vector<int> & seq
) {
    seq.clear();
    int64_t curr_time =0;
    //int64_t remaining_slice = quantum;
    int cpu = -1;
    std::vector<int> rq, jq;
    std::vector<int64_t> remainingB;
    int i = 0;
    while(i<(int)processes.size()){
        jq.push_back(processes[i].id);
        remainingB.push_back(processes[i].burst);
        i++;
    }

    while(!rq.empty() || !jq.empty()){
        cpu = -1;
        if(!(!jq.empty() || (int)rq.size()!=1)){
            curr_time+=remainingB[rq.front()];
            cpu = rq.front();
            remainingB[rq.front()]=0;
            if(seq.back()!=cpu && (int)seq.size()<max_seq_len)  seq.push_back(cpu);
            processes[rq.front()].finish_time = curr_time;
            break;
        }

        if(!jq.empty() && !rq.empty()){
            while(processes[jq.front()].arrival<curr_time && !jq.empty()){
                rq.push_back(jq.front());
                jq.erase(jq.begin());             
            }
            if(!jq.empty() && processes[jq.front()].arrival == curr_time){
                if(remainingB[rq.front()]!=0){
                    int current_process = rq.front(); 
                    rq.erase(rq.begin());
                    rq.push_back(current_process);
                }else  rq.erase(rq.begin());   
                rq.push_back(jq.front());
                jq.erase(jq.begin());
            }else{
                
                if(remainingB[rq.front()]!=0){
                    int current_process = rq.front(); 
                    rq.erase(rq.begin());
                    rq.push_back(current_process);
                }else rq.erase(rq.begin());   
            }   
        }
        if(!(!rq.empty() || jq.empty())){
            rq.push_back(jq.at(0));
            jq.erase(jq.begin());
            if(curr_time == processes[rq.front()].arrival) cpu = rq.front();
            curr_time = processes.at(rq.front()).arrival;
            if((int)seq.size()<max_seq_len)  seq.push_back(cpu);
        }
        bool optimizeFlag = true;
        int64_t n = 1;
        if(!rq.empty() && !jq.empty() && curr_time+(int)rq.size()*quantum <processes[jq.front()].arrival){
            int64_t minRB = remainingB[rq.front()];
            int burstCheck = 0;
            while(burstCheck<(int)rq.size()){
                if(remainingB[rq[burstCheck]] <= quantum) optimizeFlag=false;
                if((remainingB[rq[burstCheck]])<minRB) minRB = remainingB[rq[burstCheck]];
                burstCheck++;
            }
            int64_t maybe_n1 = minRB/quantum;
            if(minRB%quantum==0) maybe_n1--;
            int64_t maybe_n2 = (processes[jq.front()].arrival-curr_time)/((int)rq.size()*quantum);
            n = std::min(maybe_n1,maybe_n2 );
        }else if(!rq.empty() && jq.empty()){
            int64_t minRB = remainingB[rq.front()];
            int rqCheck = 0;
            while(rqCheck<(int)rq.size()){
                if(remainingB[rq[rqCheck]] < quantum) optimizeFlag=false;
                if(remainingB[rq[rqCheck]] == quantum) optimizeFlag=false;
                if((remainingB[rq[rqCheck]])<minRB) minRB = remainingB[rq[rqCheck]];
                rqCheck++;
            }
            n = minRB/quantum;
            if(minRB%quantum==0) n--;  
        }else optimizeFlag=false;

        if(!rq.empty() && optimizeFlag){  
            int processRQ = 0;        
            while(processRQ<(int)rq.size()){
                if(processes[rq[processRQ]].start_time == -1) processes[rq[processRQ]].start_time=curr_time+processRQ*quantum;         
                remainingB[rq[processRQ]]-=n*quantum; 
                processRQ++;
            }
            int64_t timeCheck = 0;
            while(timeCheck<n){ 
                if(timeCheck >=(int64_t) max_seq_len) break;  
                int innerLoop = 0;
                while(innerLoop<(int)rq.size()){ 
                    cpu = rq[innerLoop];
                    if(seq.back()!=cpu && (int)seq.size()<max_seq_len)  seq.push_back(cpu);
                    innerLoop++;
                } 
                timeCheck++;   
            }              
            curr_time+=n*rq.size()*quantum;    
        }
        
        if(!(rq.empty() || remainingB[rq.front()]>quantum)){
          
            if(!(processes[rq.front()].start_time != -1)) processes[rq.front()].start_time=curr_time;
            cpu = rq.front();
            curr_time+=remainingB[rq.front()];
            if(!(seq.back()==cpu || (int)seq.size()>=max_seq_len))  seq.push_back(cpu);   
            remainingB[rq.front()] = 0;
            processes[rq.front()].finish_time = curr_time;
        }
        else if(!rq.empty() && remainingB[rq.front()] > quantum){
            if(processes[rq.front()].start_time == -1) processes[rq.front()].start_time=curr_time;
            cpu = rq.front();
            curr_time=curr_time+quantum;
            if(!(seq.back()==cpu || (int)seq.size()>=max_seq_len))  seq.push_back(cpu);
            remainingB[rq.front()] -= quantum;
        }

        if(!(rq.empty() || !jq.empty())){
            if(!(remainingB[rq.front()]==0)){
                int current_process = rq.front(); 
                rq.erase(rq.begin());
                rq.push_back(current_process);
            } else  rq.erase(rq.begin());

        }       
    } 
    return;
}
// this is the file you need to edit
// -------------------------------------------------------------------------------------
// you don't have to use any of the code below, but you can

#include "memsim.h"
#include <iostream>
#include <cassert>
#include <unordered_map>
#include <list>
#include <set>
#include <iterator>
#include <climits>

struct Partition
{
  int tag;
  int64_t size, addr;
};

typedef std::list<Partition>::iterator PartitionRef;
typedef std::set<PartitionRef>::iterator PartitionRefRef;

struct scmp
{
  bool operator()(const PartitionRef &c1, const PartitionRef &c2) const
  {
    if (c1->size == c2->size)
      return c1->addr < c2->addr;
    else
      return c1->size > c2->size;
  }
};

struct Simulator
{
  int64_t page_size;
  int64_t page_requested;
  std::list<Partition> partitions;                 
  std::set<PartitionRef, scmp> unoccupied;                    
  std::unordered_map<int, std::list<PartitionRef>> tag_partitions; 

  Simulator(int64_t page_size)
  {
    this->page_size = page_size;
    this->page_requested = 0;
    this->partitions = std::list<Partition>();
    this->tag_partitions = std::unordered_map<int, std::list<PartitionRef>>();
  }

  int64_t page_to_request(int64_t size)
  {
    int64_t page_size = this->page_size;

    if (!(size % page_size != 0))
    {
      return size / page_size;
    }
    else
    {
      return size / page_size + 1;
    }
  }

  void unoccupied_erase_by_partition_iterator(PartitionRef partition_iterator)
  {
    PartitionRefRef partition_iterator_iterator = this->unoccupied.find(partition_iterator);
    if (!(partition_iterator_iterator == this->unoccupied.end()))
    {
      this->unoccupied.erase(partition_iterator_iterator);
    }
  }

  void allocate(int tag, int64_t size)
  {
    if (partitions.size() == 0)
    {
      int64_t new_pages = this->page_to_request(size);
      Partition head;
      head.tag = 0;
      head.addr = new_pages;
      head.size = this->page_size;
      this->page_requested = new_pages + this->page_requested;

      this->partitions.push_back(head);
      this->unoccupied.insert(this->partitions.begin());
    }

    // Local variables
    int has_suitable = 1; 
    PartitionRef suitable;

    if (this->unoccupied.size() != 0)
    { // Have unoccupied partitions, find largest suitable partition
      PartitionRefRef max_unoccupied_iterator = this->unoccupied.begin();
      PartitionRef max_unoccupied = *max_unoccupied_iterator;
      if (!(max_unoccupied->size < size))
      {
        has_suitable = 0;
        suitable = max_unoccupied;
        this->unoccupied.erase(max_unoccupied_iterator);
      }
    }

    // Split partiton OR expand by page
    if (has_suitable == 1)
    {
      // No suitable unoccupied partition, expand partition at the end
      PartitionRef last = std::prev(this->partitions.end());

      // Last partition unoccupied, make its size to total last size
      if (!(last->tag != 0))
      {
        int64_t total_last_size = last->size;

        // Add pages
        int64_t new_pages = this->page_to_request(size - last->size);
        total_last_size = total_last_size + new_pages * this->page_size;
        this->page_requested = new_pages + this->page_requested;

        // Occupy last partition
        this->unoccupied_erase_by_partition_iterator(last);
        last->tag = tag;
        last->size = size;
        this->tag_partitions[tag].push_back(last);
        total_last_size = total_last_size - size;

        // Append Leftover size if there is any
        if (total_last_size > 0)
        {
          int64_t addr = last->addr + last->size;
          Partition new_last;
          new_last.addr = addr;
          new_last.size = total_last_size;
          new_last.tag = 0;

          this->partitions.push_back(new_last);
          this->unoccupied.insert(std::prev(this->partitions.end()));
        }
      }
      else
      {
        int64_t new_pages = this->page_to_request(size);
        this->page_requested = this->page_requested + new_pages;
        int64_t total_last_size = new_pages * this->page_size;
        int64_t leftover_size = total_last_size - size;

        // New partition
        int64_t new_last_addr = last->addr + last->size;
        Partition new_last;
        new_last.addr = new_last_addr;
        new_last.size = size;
        new_last.tag = tag;

        auto itPartitions = partitions.insert(partitions.end(), new_last);
        tag_partitions[tag].push_back(std::prev(itPartitions, 1));

        // Append leftover partition
        if (!(leftover_size <= 0))
        {
          int64_t leftover_addr = new_last_addr;
          leftover_addr += size;
          Partition leftover;
          leftover.addr = leftover_addr;
          leftover.size = leftover_size;
          leftover.tag = 0;

          auto itPartitions = partitions.insert(partitions.end(), leftover);
          unoccupied.insert(std::prev(itPartitions, 1));
        }
      }
    }
    else
    {
      // Split partition
      int64_t total_suitable_size = suitable->size;
      int64_t leftover_size = total_suitable_size - size;

      // Occupy partition
      suitable->tag = tag;
      suitable->size = size;
      auto itTagPartitions = tag_partitions[tag].insert(tag_partitions[tag].end(), suitable);

      // Append leftover partition
      if (!(leftover_size <= 0))
      {
        int64_t leftover_addr = suitable->addr + size;
        Partition leftover;
        leftover.addr = leftover_addr;
        leftover.size = leftover_size;
        leftover.tag = 0;

        this->partitions.insert(std::next(suitable), leftover);
        this->unoccupied.insert(std::next(suitable));
      }
    }
  }

  void deallocate(int tag)
  {

    std::list<PartitionRef> partitions = this->tag_partitions[tag];

    for (PartitionRef partition : partitions)
    {
      partition->tag = 0;

      // If has predecessor
      if (!(partition == this->partitions.begin()))
      {
        PartitionRef predecessor = std::prev(partition);

        // And predecessor is unoccupied, merge with partition
        if (!(predecessor->tag != 0))
        {
          partition->addr = predecessor->addr;
          partition->size = partition->size + predecessor->size;
          this->partitions.erase(predecessor);
          this->unoccupied_erase_by_partition_iterator(predecessor);
        }
      }

      // If has successor
      if (!(partition == std::prev(this->partitions.end())))
      {
        PartitionRef successor = std::next(partition);

        // And and successor is unoccupied
        if (!(successor->tag != 0))
        {
          // Successor is unoccupied, merge with partition

          partition->size = partition->size + successor->size;
          this->partitions.erase(successor);
          this->unoccupied_erase_by_partition_iterator(successor);
        }
      }

      this->unoccupied.insert(partition);
    }

    this->tag_partitions[tag].clear();
  }

  MemSimResult getStats()
  {
    MemSimResult result;

    if (!(this->unoccupied.size() != 0))
    {

      result.max_free_partition_size = 0;
      result.max_free_partition_address = 0;
    }
    else
    {
      PartitionRefRef max_unoccupied_iterator = this->unoccupied.begin();
      PartitionRef max_unoccupied = *max_unoccupied_iterator;
      result.max_free_partition_size = max_unoccupied->size;
      result.max_free_partition_address = max_unoccupied->addr-1;
    }

    result.n_pages_requested = this->page_requested;
    return result;
  }
};

// re-implement the following function
// ===================================
// parameters:
//    page_size: integer in range [1..1,000,000]
//    requests: array of requests
// return:
//    some statistics at the end of simulation
MemSimResult mem_sim(int64_t page_size, const std::vector<Request> &requests)
{
  Simulator sim(page_size);
  for (const auto &req : requests)
  {
    if (req.tag < 0)
    {
      sim.deallocate(-req.tag);
    }
    else
    {
      sim.allocate(req.tag, req.size);
    }
  }
  return sim.getStats();
}
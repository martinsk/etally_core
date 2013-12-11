#ifndef _TYPES_HH
#define _TYPES_HH

#include <stdint.h>

#include <unordered_map>
#include <unordered_set>

#include <iostream>
#include <string>


typedef uint32_t idx_t;
typedef uint32_t timestamp_t;

typedef idx_t counter_idx_t;
typedef idx_t leaderboard_idx_t;

typedef std::string lb_name_t;
typedef std::string counter_name_t;

class leaderboard;

typedef unsigned int metric_payload_t;


typedef std::unordered_map<leaderboard_idx_t, std::unordered_map<counter_idx_t,  leaderboard* > > leaderboard_lookup_counter_map;
typedef std::unordered_map<counter_idx_t, std::unordered_set<leaderboard_idx_t > > lb_lookup_set_map;


struct event {
  timestamp_t timestamp;
  idx_t       event_idx; 
};


struct event_metric {
  timestamp_t      timestamp;
  idx_t            event_idx; 
  metric_payload_t payload;
};

struct metric_counter_stats {
  unsigned long count;
  unsigned long sum;
  unsigned long sqsum;
};




std::ostream& operator<<(std::ostream& out, const struct event e); 
std::ostream& operator<<(std::ostream& out, const struct event_metric e); 


struct eqidx {
  bool operator()(idx_t i0, idx_t i1) const
  {
    return i0 < i1;
  }
};

#endif // _TYPES_HH

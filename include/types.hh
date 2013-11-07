#ifndef _TYPES_HH
#define _TYPES_HH

#include <stdint.h>

#include <unordered_map>
#include <unordered_set>

#include <string>

typedef uint32_t idx_t;
typedef uint32_t timestamp_t;

typedef uint32_t action_t;
typedef uint32_t counter_idx_t;
typedef uint32_t counter_t;



typedef std::string lb_name_t;
typedef std::string count_name_t;

class leaderboard;

typedef std::unordered_map<lb_name_t, std::unordered_map<unsigned int,  leaderboard* > > lb_double_map_t;
typedef std::unordered_map<count_name_t, std::unordered_set<lb_name_t > > lb_idx_t;

struct event {
  timestamp_t timestamp;
  idx_t event_idx; 
};



struct eqidx {
  bool operator()(idx_t i0, idx_t i1) const
  {
    return i0 < i1;
  }
};

#endif // _TYPES_HH

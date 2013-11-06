#ifndef _TYPES_HH
#define _TYPES_HH

#include <stdint.h>

#include <string>

typedef uint32_t idx_t;
typedef uint32_t timestamp_t;

typedef uint32_t action_t;
typedef uint32_t counter_idx_t;
typedef uint32_t counter_t;



typedef std::string lb_name_t;
typedef std::string count_name_t;


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

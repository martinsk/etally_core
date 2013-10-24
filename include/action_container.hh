#ifndef _ACTION_CONTAINER_HH
#define _ACTION_CONTAINER_HH

#include "types.hh"

#include <vector>
#include <tr1/functional>
#include <tr1/unordered_map>

using std::tr1::hash;
using std::tr1::unordered_map;

typedef unordered_map<action_t, std::vector<counter_idx_t> , hash<idx_t>, eqidx> action_map_t;


class action_container{
private:
  action_map_t action_map;

public:
  void add_action(action_t action, std::vector<counter_idx_t> idxs);
  
  void handle_incr_action(action_t action, counter_t* counters);
  void handle_decr_action(action_t action, counter_t* counters);
};

    



#endif // _ACTION_CONTAINER_HH

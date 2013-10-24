#include "action_container.hh"


void action_container::add_action(action_t action, std::vector<counter_idx_t> idxs) {
  action_map[action] = idxs;
}


void action_container::handle_incr_action(action_t action,  counter_t* counters) {
  std::vector<counter_idx_t> counter_idxs = action_map[action]; 
  std::vector<counter_idx_t>::iterator it;
  for(it = counter_idxs.begin(); it != counter_idxs.end(); it++){
    counters[*it]++;
  }
}

void action_container::handle_decr_action(action_t action,  counter_t* counters) {
  std::vector<counter_idx_t> counter_idxs = action_map[action]; 
  std::vector<counter_idx_t>::iterator it;
  for(it = counter_idxs.begin(); it != counter_idxs.end(); it++){
    counters[*it]--;
  }
}

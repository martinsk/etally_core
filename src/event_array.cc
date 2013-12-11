#include "types.hh"
#include "circular_queue.hh"
#include "event_array.hh"
#include "leaderboard.hh"

#include <iostream>
#include <algorithm>

std::unordered_map<std::vector<counter_idx_t>, idx_t, vector_hash<uint32_t> >  event_array::encode_map;
std::unordered_map<idx_t, std::vector<counter_idx_t> > event_array::decode_map;
std::unordered_map<idx_t, unsigned long>     event_array::reference_counters;
std::unordered_set<idx_t>  event_array::unused_idx;
idx_t  event_array::max_idx = 0;

leaderboard_lookup_counter_map event_array::lb_map;
lb_lookup_set_map  event_array::lb_lookup_map;

counter_array<unsigned long> event_array::insert_counters;


event_array::event_array(timestamp_t timespan, event_array* const tail)
  :  timespan(timespan),
     tail(tail){ }

event_array::~event_array() {
  
}


void event_array::event(struct event e) {
  queue.enqueue(e);
}

void event_array::event(const std::vector<counter_idx_t>& groups, timestamp_t insert_time) {

  idx_t event_idx;
  if(encode_map.count(groups) == 0) {

    event_idx = max_idx;

    max_idx++;
    encode_map[groups]    = event_idx;
    decode_map[event_idx] = groups;
    
  } else{
    event_idx = encode_map.find(groups)->second;
  }
  reference_counters[event_idx] ++;
  
  struct event e = {.timestamp = insert_time,
                    .event_idx = event_idx };
  
  event(e);
}

void event_array::update(timestamp_t now) {
  
  struct event e = queue.front();

  while( !queue.empty() && e.timestamp < (now - timespan)) {
    queue.dequeue();
    
    for(auto& s : decode_map[e.event_idx]) {
      counters[s]++;
      
      long insert_count = event_array::insert_counters[s];
      long adjusted_count = insert_count - counters[s];

      for(auto lb : event_array::lb_lookup_map[s]){
        auto& this_lb_map = event_array::lb_map[lb];
        if ( this_lb_map.count(timespan) ) {
          if( adjusted_count == 0)
            this_lb_map[timespan]->remove(s);
          else 
            this_lb_map[timespan]->add(s, adjusted_count );
        }
        
      }
    }
    
    if(tail) tail->event(e);
    else {
      reference_counters[e.event_idx]--;
      if(reference_counters[e.event_idx] == 0){
        reference_counters.erase(e.event_idx);
      }
    }
    e = queue.front();
  } 
}



unsigned long event_array::get_counter(idx_t idx) const{
  return counters[idx];
}



void event_array::sort() {
  
  struct {
    bool operator() (std::pair<uint32_t, uint32_t> i,
                     std::pair<uint32_t, uint32_t>j) { return (i>j);}
  } reverse_pair_sorter_instance;
  

  std::vector<std::pair<uint32_t, uint32_t> > vec;
  while(!queue.empty()) {
    vec.push_back(std::make_pair(-queue.front().timestamp, queue.front().event_idx));
    queue.dequeue();
  }
  std::sort(vec.begin(), vec.end(), reverse_pair_sorter_instance);

  for(auto& pair : vec){
    struct event e;
    e.timestamp = -pair.first;
    e.event_idx = pair.second;
    queue.enqueue(e);
  }
}
  
unsigned long event_array::length() const {
  return queue.length();
}


std::ostream& operator<<(std::ostream& out, const event_array& ea) {
  out << "{event_array, " <<  ea.queue << "} ";
  return out;
}


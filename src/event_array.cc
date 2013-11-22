#include "types.hh"
#include "circular_queue.hh"
#include "event_array.hh"
#include "leaderboard.hh"

#include <iostream>
#include <algorithm>

std::unordered_map<std::vector<counter_idx_t>, idx_t, uint32_vector_hasher>  event_array::encode_map;
std::unordered_map<idx_t, std::vector<counter_idx_t> > event_array::decode_map;
std::unordered_map<idx_t, unsigned long>     event_array::reference_counters;
std::unordered_set<idx_t>  event_array::unused_idx;
idx_t  event_array::max_idx = 0;

leaderboard_lookup_counter_map event_array::lb_map;
lb_lookup_set_map  event_array::lb_lookup_map;


event_array::event_array(timestamp_t timespan, event_array* const tail)
  :  timespan(timespan),
     tail(tail){ }

event_array::~event_array() {
  
}


void event_array::event(struct event e) {
  queue.enqueue(e);
}

void event_array::event(std::vector<counter_idx_t> groups, timestamp_t insert_time) {
  for(auto s : groups)
    increment_counter(s);


  int event_idx;
  if(encode_map.count(groups) == 0) {
    // std::cout << "adding to encode_map" << std::endl;

    event_idx = max_idx;
    // std::cout << "event_idx : " << event_idx << std::endl;

    max_idx++;
    encode_map[groups]    = event_idx;
    decode_map[event_idx] = groups;
    
  } else{
    event_idx = encode_map.find(groups)->second;
  }
  reference_counters[event_idx] ++;
 
  
  timestamp_t now = time(0);

  struct event e;
  e.timestamp = (insert_time == 0)? now: insert_time;
  e.event_idx = event_idx;
 
  event(e);

  update(now);  
}

void event_array::update(timestamp_t now) {
  
  struct event e = queue.front();


  while( !queue.empty() && e.timestamp < (now - timespan)) {
    queue.dequeue();
    
    for(auto& s : decode_map[e.event_idx]) {
      counters[s]--;

      for(auto lb : event_array::lb_lookup_map[s]){
        event_array::lb_map[lb][timespan]->add(s, counters[s]);
      }
      
      if(counters[s] == 0){
        counters.erase(s);
        for(auto lb : event_array::lb_lookup_map[s]){
          event_array::lb_map[lb][timespan]->remove(s);
        }
        //event_array::lb_lookup_map[s].erase(lb);
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

  if(tail) tail->update(now);

}

void event_array::increment_counter(counter_idx_t c) {
  
  if (counters.count(c) == 0) {
    counters[c] = 0;
  }
  counters[c] ++;
  
  for(auto lb : event_array::lb_lookup_map[c]) {
    event_array::lb_map[lb][timespan]->add(c, counters[c]);
  }
  
  if(tail) tail->increment_counter(c);
}

void event_array::sort() {
  std::vector<std::pair<uint32_t, uint32_t> > vec;
  while(!queue.empty()) {
    vec.push_back(std::make_pair(-queue.front().timestamp, queue.front().event_idx));
    queue.dequeue();
  }
  std::sort(vec.begin(), vec.end());

  for(auto& pair : vec){
    struct event e;
    e.timestamp = -pair.first;
    e.event_idx = pair.second;
    queue.enqueue(e);
  }
  update(time(0));
  if(tail) tail->sort();
}
  
unsigned long event_array::length() const {
  return queue.length();
}



void event_array::print() const {
  std::cout << queue << std::endl;
  
}





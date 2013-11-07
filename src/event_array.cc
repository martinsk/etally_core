#include "types.hh"
#include "event_array.hh"
#include "leaderboard.hh"

#include <iostream>

std::unordered_map<std::vector<count_name_t>, idx_t, string_vector_hasher>  event_array::encode_map;
std::unordered_map<idx_t, std::vector<count_name_t> > event_array::decode_map;
std::unordered_map<idx_t, unsigned long>     event_array::reference_counters;
std::unordered_set<idx_t>  event_array::unused_idx;
idx_t  event_array::max_idx = 0;

lb_double_map_t event_array::lb_map;
lb_idx_t event_array::lb_lookup_map;


event_array::event_array(timestamp_t timespan, event_array* const tail)
  :  data(new struct event[1]),
     max_size(2),
     size(0),
     back(0),
     front(0),
     timespan(timespan),
     tail(tail){ }

event_array::~event_array() {
  delete[] data;
}


void event_array::event(struct event e) {
  //  printf("event %d\n", e.event_idx);
  if (size == max_size) {
    // double the array and copy
    struct event* old_data = data;
    max_size *= 2;
    data = new struct event[max_size];
    for(uint32_t i = 0; i != size; i++) {
      data[i] = old_data[(back + i)%size];
    }
    back  = 0;
    front = size;    
    delete[] old_data;
  }

  data[front] = e;
  front++;

  front%=max_size;

  size++;
}

void event_array::event(std::vector<count_name_t> groups, timestamp_t insert_time) {
  for(auto s : groups)
    increment_counter(s);


  int event_idx;
  if(encode_map.count(groups) == 0) {
    std::cout << "adding to encode_map" << std::endl;

    event_idx = max_idx;
    max_idx++;
    encode_map[groups] = event_idx;
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
  struct event e = data[back];


  while(size!=0 && e.timestamp < (now - timespan)) {
    back ++;
    back %= max_size;
    size--;

    for(auto& s : decode_map[e.event_idx]) {
      counters[s]--;
      for(auto lb : event_array::lb_lookup_map[s]){
        event_array::lb_map[lb][timespan]->update_down(s, counters[s]);
      }
      if(counters[s] == 0){
        counters.erase(s);
        for(auto lb : event_array::lb_lookup_map[s]){
          event_array::lb_map[lb][timespan]->remove(s);
          event_array::lb_lookup_map[s].erase(lb);
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
    e = data[back];
  }
  
  if(tail) tail->update(now);

}

void event_array::increment_counter(count_name_t c) {
  
  if (counters.count(c) == 0) {
    counters[c] = 0;
  }
  counters[c] ++;
  
  for(auto lb : event_array::lb_lookup_map[c]) {
    event_array::lb_map[lb][timespan]->add(c, counters[c]);
  }
  
  std::cout  << "increment : " << c << " to " <<  counters[c] << std::endl;
  if(tail) tail->increment_counter(c);
}
  
int event_array::length() const {
  return size;
}



void event_array::print() const {
  for(uint32_t i = 0; i != size; i++) {
    struct event& e = data[(back + i)%max_size];
    printf("{%d, %d}, ", e.timestamp, e.event_idx);
  }
  printf("\n");
}

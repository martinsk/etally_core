#include "types.hh"
#include "circular_queue.hh"
#include "event_metric_array.hh"
#include "leaderboard.hh"

#include <iostream>
#include <algorithm>

std::unordered_map<std::vector<counter_idx_t>, idx_t, vector_hash<uint32_t> >  event_metric_array::encode_map;
std::unordered_map<idx_t, std::vector<counter_idx_t> >                       event_metric_array::decode_map;
std::unordered_map<idx_t, unsigned long>                                     event_metric_array::reference_counters;
std::unordered_set<idx_t>                                                    event_metric_array::unused_idx;
idx_t                                                                        event_metric_array::max_idx = 0;

std::unordered_map<counter_idx_t, std::unordered_map<unsigned int,  percentile* > >  event_metric_array::percentile_map;

leaderboard_lookup_counter_map event_metric_array::lb_map;
lb_lookup_set_map              event_metric_array::lb_lookup_map;

counter_array<unsigned long>        event_metric_array::insert_counters;
counter_array<unsigned long long>   event_metric_array::insert_sums;
counter_array<unsigned long long >  event_metric_array::insert_sqsums;


event_metric_array::event_metric_array(timestamp_t timespan, event_metric_array* const tail)
  :  timespan(timespan),
     tail(tail){ }

event_metric_array::~event_metric_array() { }


void event_metric_array::event(struct event_metric e) {
  queue.enqueue(e);
}

void event_metric_array::event(const std::vector<counter_idx_t>& groups,
                               metric_payload_t payload,
                               timestamp_t insert_time) {
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
  
  
  struct event_metric e = {.timestamp = insert_time,
                           .event_idx = event_idx  ,
                           .payload   = payload    };
  
  event(e);
}

void event_metric_array::update(timestamp_t now) {
  
  struct event_metric e = queue.front();

  while( !queue.empty() && e.timestamp < (now - timespan)) {
    queue.dequeue();
    
    for(auto& s : decode_map[e.event_idx]) {
      counters[s]++;
      sums[s]   += e.payload;
      sqsums[s] += e.payload*e.payload;;
      
      unsigned long insert_count      = event_metric_array::insert_counters[s];
      unsigned long adjusted_count  = insert_count - counters[s];

      for(auto lb : event_metric_array::lb_lookup_map[s]){
        auto& this_lb_map = event_metric_array::lb_map[lb];
        if ( this_lb_map.count(timespan) ) {
          if( adjusted_count == 0)
            this_lb_map[timespan]->remove(s);
          else 
            this_lb_map[timespan]->add(s, adjusted_count );
        }
      }

      if(event_metric_array::percentile_map.count(s)) {
        if (event_metric_array::percentile_map[s].count(timespan))
          percentile_map[s][timespan] -> remove(e.payload);
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



unsigned long event_metric_array::get_counter(idx_t idx) const{
  
  if ( reference_counters.count(idx) )
    return counters[idx];
  else 
    return 0;
}



void event_metric_array::sort() {
  
  struct {
    bool operator() (std::pair<std::pair<uint32_t, uint32_t>, uint32_t> i,
                     std::pair<std::pair<uint32_t, uint32_t>, uint32_t> j) { return (i>j);}
  } reverse_pair_sorter_instance;
  

  std::vector<std::pair<std::pair<uint32_t, uint32_t>, uint32_t> > vec;
  while(!queue.empty()) {
    vec.push_back(std::make_pair(std::make_pair(-queue.front().timestamp,
                                                queue.front().event_idx),
                                 queue.front().payload));
    queue.dequeue();
  }
  std::sort(vec.begin(), vec.end(), reverse_pair_sorter_instance);

  for(auto& pair : vec){
    struct event_metric e;
    e.timestamp = -pair.first.first;
    e.event_idx = pair.first.second;
    e.payload   = pair.second;
    queue.enqueue(e);
  }
}
  
unsigned long event_metric_array::length() const {
  return queue.length();
}


std::ostream& operator<<(std::ostream& out, const event_metric_array& ea) {
  out << "{event_metric_array, " <<  ea.queue << "} ";
  return out;
}


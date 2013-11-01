#include "types.hh"
#include "event_array.hh"


event_array::event_array(timestamp_t timespan, event_array* const tail)
  :  data(new struct event[1]),
     max_size(1),
     size(0),
     back(0),
     front(0),
     timespan(timespan),
     tail(tail){ }
event_array::~event_array() {
  delete[] data;
}


void event_array::event(struct event e) {
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

void event_array::event(idx_t event_idx, timestamp_t insert_time) {
  increment_counter(event_idx);
  timestamp_t now = time(0);

  struct event e;
  e.timestamp = (insert_time == 0)? now: insert_time;
  e.event_idx = event_idx;
 
  event(e);
  update(now);  
}

void event_array::update(timestamp_t now) {
  if(now == 0)  now = time(0);

  if(back == front) return;

  struct event e = data[back];
  while(back!=front && e.timestamp < now - timespan) {
    back ++;
    back %= max_size;
    size--;
    counters[e.event_idx]--;
    if(counters[e.event_idx] == 0){
      counters.erase(e.event_idx);
    }

    if(tail != NULL)
      tail->event(e);
    e = data[back];
  }

  if(tail!=NULL) 
    tail->update(now);
}

void event_array::increment_counter(idx_t event_idx) {
  if (counters.count(event_idx) == 0) {
    counters[event_idx] = 0;
  }
  counters[event_idx] ++;
  if(tail != NULL)
    tail->increment_counter(event_idx);
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

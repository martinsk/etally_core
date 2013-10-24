#include "types.hh"
#include "event_array.hh"


event_array::event_array(timestamp_t timespan, action_container actions, std::vector<event_scanner> scanners)
  :  data(new struct event[1]),
     max_size(1),
     size(0),
     back(0),
     front(0),
     timespan(timespan),
     actions(actions),
     scanners(scanners){ }
event_array::~event_array() {
  delete[] data;
}


void event_array::event(idx_t event_idx) {
  struct event e;
  timestamp_t now = time(0);
  e.timestamp = now;
  e.event_idx = event_idx;
 
  if (size == max_size) {
    // double the array and copy
    struct event* old_data = data;
    max_size *= 2;
    data = new struct event[max_size];
    for(uint32_t i = 0; i != size; i++) {
      data[i] = old_data[(back + i)%size];
    }
    back = 0;
    front = size;
    
    delete[] old_data;
  }

  data[front] = e;
  front++;
  front%=max_size;

  size++;

  update(now);
}


void event_array::add_scanner(event_scanner scanner) {
  scanners.push_back(scanner);
}


void event_array::update(timestamp_t now) {
  if(back == front) return;

  std::vector<event_scanner>::iterator it;
  for(it = scanners.begin(); it != scanners.end(); it++) {
    it->update(this, now);
  }
  
  struct event e = data[back];
  while(back!=front && e.timestamp < now - timespan) {
    back ++;
    back %= max_size;
    size--;
    e = data[back];
  }
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

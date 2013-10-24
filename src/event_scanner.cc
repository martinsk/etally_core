#include "event_scanner.hh"
#include "event_array.hh"


event_scanner::event_scanner(timestamp_t offset, idx_t start) 
  : idx(start),
    thresshold_offset(offset)   
{}
event_scanner::~event_scanner(){}


void event_scanner::update(event_array* work_array,
                           timestamp_t current_time) {
  timestamp_t thresshold     = current_time - thresshold_offset;
  struct event current_event = work_array->data[idx];

  while(idx != work_array->front && current_event.timestamp < thresshold){
    // update array
    
    idx ++;
    idx %= work_array->max_size;
    current_event = work_array->data[idx];
  }
}

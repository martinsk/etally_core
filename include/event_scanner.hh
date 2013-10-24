#ifndef _EVENT_SCANNER_HH
#define _EVENT_SCANNER_HH

#include "types.hh"

class event_array;

class event_scanner{
private:
  idx_t idx;
  timestamp_t thresshold_offset;

public:
  event_scanner(timestamp_t offset, idx_t start); 
  ~event_scanner();
  
  void update(event_array* work_array, timestamp_t current_time);
};


#endif // _EVENT_SCANNER_HH

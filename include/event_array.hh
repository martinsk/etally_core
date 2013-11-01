#ifndef _EVENT_ARRAY_HH
#define _EVENT_ARRAY_HH

#include <vector>
#include "event_scanner.hh"
#include "action_container.hh"


class event_array {
private:
  struct event* data;
  uint32_t max_size;
  uint32_t size;
  idx_t back;
  idx_t front;
  timestamp_t timespan;
  event_array* tail;

public:
  event_array(timestamp_t timespan, event_array* const tail = NULL);
  ~event_array();
  
  std::unordered_map<unsigned, unsigned long> counters;

  void event(struct event e);
  void event(idx_t event_idx, timestamp_t insert_time = 0);
  void update(timestamp_t now = 0);
  void increment_counter(idx_t idx);

  int length() const;

  void print() const;

private:    
  friend class event_scanner;
};


#endif // _EVENT_ARRAY_HH

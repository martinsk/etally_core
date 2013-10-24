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
  action_container actions;

  std::vector<event_scanner> scanners;

public:
  event_array(timestamp_t timespan, action_container actions, std::vector<event_scanner> scanners);
  ~event_array();
  
  void event(idx_t event_idx);
  void update(timestamp_t now);

  void add_scanner(event_scanner scanner);
  int length() const;

  void print() const;

private:    
  friend class event_scanner;
};


#endif // _EVENT_ARRAY_HH

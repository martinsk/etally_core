#ifndef _EVENT_ARRAY_HH
#define _EVENT_ARRAY_HH

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "circular_queue.hh"

#include "leaderboard.hh"

class leaderboard;

class uint32_vector_hasher {
public:
  std::size_t operator()(std::vector<uint32_t> const& vec) const {
    std::size_t ret = 0;
    for(auto& i : vec) {
      size_t hash = std::hash<uint32_t>()(i);
      ret ^= hash;
    }
    return ret;
  }
};



class event_array {
private:
  circular_queue<struct event> queue;
public:
  timestamp_t   timespan;
  event_array* const  tail;
  
  static std::unordered_map<std::vector<counter_idx_t>, idx_t, uint32_vector_hasher>  encode_map;
  static std::unordered_map<leaderboard_idx_t, std::vector<counter_idx_t> > decode_map;
  static std::unordered_map<idx_t, unsigned long>     reference_counters;
  static std::unordered_set<idx_t> unused_idx;
  static idx_t max_idx;

  static leaderboard_lookup_counter_map lb_map;
  static lb_lookup_set_map lb_lookup_map;


public:
  event_array(timestamp_t timespan, event_array* const tail = NULL);
  ~event_array();
  
  std::unordered_map<counter_idx_t, unsigned long> counters;

  void event(struct event e);
  void event(std::vector<counter_idx_t> groups, timestamp_t insert_time);
  void update(timestamp_t now);
  void increment_counter(idx_t c);

  void sort();

  unsigned long length() const;

  void print() const;
};


#endif // _EVENT_ARRAY_HH

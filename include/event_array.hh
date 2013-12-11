#ifndef _EVENT_ARRAY_HH
#define _EVENT_ARRAY_HH

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "vector_hash.hh"
#include "circular_queue.hh"
#include "counter_array.hh"

#include "leaderboard.hh"

class leaderboard;




class event_array {
private:
  circular_queue<struct event> queue;

public:
  counter_array<unsigned long> counters;


public:
  timestamp_t   timespan;
  event_array* const  tail;
  

  // simple bidirectional lookup for counter arrays - the hash function might be horible.
  static std::unordered_map<std::vector<counter_idx_t>, idx_t, vector_hash<uint32_t> >  encode_map;
  static std::unordered_map<leaderboard_idx_t, std::vector<counter_idx_t> > decode_map;
  static std::unordered_map<idx_t, unsigned long>     reference_counters;

  static std::unordered_set<idx_t> unused_idx;
  static idx_t max_idx;

  static leaderboard_lookup_counter_map lb_map;
  static lb_lookup_set_map lb_lookup_map;
 
public:
  static counter_array<unsigned long> insert_counters;
 

public:
  event_array(timestamp_t timespan, event_array* const tail = NULL);
  ~event_array();
  

  void event(struct event e);
  void event(const std::vector<counter_idx_t>& groups, timestamp_t insert_time);
  void update(timestamp_t now);
  unsigned long get_counter(idx_t idx) const;

  void sort();

  unsigned long length() const;

  friend std::ostream& operator<<(std::ostream& out, const event_array& ae);
};


#endif // _EVENT_ARRAY_HH

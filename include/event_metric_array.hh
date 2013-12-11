#ifndef _EVENT_METRIC_ARRAY_HH
#define _EVENT_METRIC_ARRAY_HH

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "circular_queue.hh"
#include "counter_array.hh"

#include "leaderboard.hh"

class leaderboard;

class uint32_vector_hasher {
public:
  std::size_t operator()(std::vector<uint32_t> const& vec) const {
    std::size_t ret = 0;
    for(auto& i : vec) {
      ret ^= std::hash<uint32_t>()(i);
    }
    return ret;
  }
};



class event_metric_array {
private:
  circular_queue<struct event_metric> queue;

public:
  counter_array<unsigned long> counters;
  counter_array<unsigned long long> sums;
  counter_array<unsigned long long> sqsums;

public:
  timestamp_t   timespan;
  event_metric_array* const  tail;
  

  // simple bidirectional lookup for counter arrays - the hash function might be horible.
  static std::unordered_map<std::vector<counter_idx_t>, idx_t, uint32_vector_hasher>  encode_map;
  static std::unordered_map<leaderboard_idx_t, std::vector<counter_idx_t> > decode_map;
  static std::unordered_map<idx_t, unsigned long>     reference_counters;

  static std::unordered_set<idx_t> unused_idx;
  static idx_t max_idx;

  static leaderboard_lookup_counter_map lb_map;
  static lb_lookup_set_map lb_lookup_map;
 
public:
  static counter_array<unsigned long> insert_counters;
  static counter_array<unsigned long long> insert_sums;
  static counter_array<unsigned long long> insert_sqsums;
 

public:
  event_metric_array(timestamp_t timespan,
                     event_metric_array* const tail = NULL);
  
  ~event_metric_array();
  

  void event(struct event_metric e);
  void event(const std::vector<counter_idx_t>& groups,
                    metric_payload_t payload,
                    timestamp_t insert_time);
  void update(timestamp_t now);
  unsigned long get_counter(idx_t idx) const;
  unsigned long get_sum(idx_t idx) const;
  unsigned long get_sqsum(idx_t idx) const;

  void sort();

  unsigned long length() const;

  friend std::ostream& operator<<(std::ostream& out, const event_metric_array& ae);
};


#endif // _EVENT_ARRAY_HH

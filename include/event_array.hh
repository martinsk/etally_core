#ifndef _EVENT_ARRAY_HH
#define _EVENT_ARRAY_HH

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "leaderboard.hh"

class leaderboard;

class string_vector_hasher {
public:
  std::size_t operator()(std::vector<std::string> const& ss) const {
    std::size_t ret = 0;
    for(auto& s : ss) {
      std::string str(s.c_str());
      size_t hash = std::hash<std::string>()(str);
      ret ^= hash;
    }
    return ret;
  }
};



class event_array {
private:
  struct event* data;
  uint32_t      max_size;
  uint32_t      size;
  idx_t         back;
  idx_t         front;
public:
  timestamp_t   timespan;
  event_array* const  tail;
  
  static std::unordered_map<std::vector<count_name_t>, idx_t, string_vector_hasher>  encode_map;
  static std::unordered_map<idx_t, std::vector<count_name_t> > decode_map;
  static std::unordered_map<idx_t, unsigned long>     reference_counters;
  static std::unordered_set<idx_t> unused_idx;
  static idx_t max_idx;

  static lb_double_map_t lb_map;
  static lb_idx_t lb_lookup_map;


public:
  event_array(timestamp_t timespan, event_array* const tail = NULL);
  ~event_array();
  
  std::unordered_map<std::string, unsigned long> counters;

  void event(struct event e);
  void event(std::vector<count_name_t> groups, timestamp_t insert_time = 0);
  void update(timestamp_t now);
  void increment_counter(std::string c);

  int length() const;

  void print() const;

private:    
  friend class event_scanner;
};


#endif // _EVENT_ARRAY_HH

#ifndef LEADERBOARD_HH
#define LEADERBOARD_HH
#include <vector>
#include <string>
#include <set>
#include <map>
#include <unordered_map>

#include "types.hh"

class event_array;

class leaderboard{
private:
  // std::map<count_name_t, unsigned long> score_map;
  // std::set<std::pair<unsigned long, count_name_t> > board;

  std::unordered_map<counter_idx_t, idx_t> idx_lookup;
  std::vector<std::pair<long, counter_idx_t> > board;

public:
  void add(counter_idx_t elm, long score);
  void remove(counter_idx_t elm);

  bool contains(counter_idx_t elm) const;

  unsigned int size() const;
  std::vector<std::pair< long, counter_idx_t> > get_range(idx_t from, idx_t to) const;

  void print();

private:
  void bubble_up(idx_t idx);
  void bubble_down(idx_t idx);

  void swap(idx_t elm1, idx_t elm2);
};

#endif // LEADERBOARD_HH

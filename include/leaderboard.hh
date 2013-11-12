#ifndef LEADERBOARD_HH
#define LEADERBOARD_HH
#include <vector>
#include <string>
#include <set>
#include <map>

#include "types.hh"

class event_array;

class leaderboard{
public:
  std::map<count_name_t, unsigned long> score_map;
  std::set<std::pair<unsigned long, count_name_t> > board;

public:
  void add(const count_name_t elm, long score);
  void remove(const count_name_t elm);

  bool contains(const count_name_t elm) const;

  void print();
};

#endif // LEADERBOARD_HH

#ifndef LEADERBOARD_HH
#define LEADERBOARD_HH
#include <vector>
#include <string>
#include <unordered_map>

#include "types.hh"

class event_array;

class leaderboard{
protected:
  std::vector<std::pair<count_name_t, unsigned long> > board;

private:
  std::unordered_map<count_name_t, idx_t> map;

public:
  void add(count_name_t elm, long score);
  void remove(count_name_t elm);

  void update_up(count_name_t elm, long score);  
  void update_down(count_name_t elm, long score);  

  void print();

private:
  void swap(count_name_t n0, count_name_t n1);
};

#endif // LEADERBOARD_HH

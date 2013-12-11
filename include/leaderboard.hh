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
  inline void bubble_up(idx_t idx);
  inline void bubble_down(idx_t idx);

  inline void swap(idx_t elm1, idx_t elm2);
};



void leaderboard::bubble_up(idx_t idx) {
  if(idx != 0) {
    if (board[idx].first > board[idx -1].first) {
      swap(idx, idx-1);
      bubble_up(idx-1);
    }
  }
}



void leaderboard::bubble_down(idx_t idx)  {
  if(idx != board.size() -1) {
    if (board[idx].first < board[idx +1].first) {
      swap(idx, idx+1);
      bubble_down(idx+1);
    }
  }
}




void leaderboard::swap(idx_t idx1, idx_t idx2)  {
  idx_lookup[board[idx1].second] = idx2;
  idx_lookup[board[idx2].second] = idx1;
  auto tmp = board[idx1];
  board[idx1] = board[idx2];
  board[idx2] = tmp;   
}


#endif // LEADERBOARD_HH

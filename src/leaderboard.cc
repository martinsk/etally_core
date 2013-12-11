
#include <iostream>
#include <algorithm>
#include "leaderboard.hh"
#include "event_array.hh"


void
leaderboard::add(counter_idx_t elm, long score) {
  if(!contains(elm)) {
    board.push_back(std::make_pair(score, elm));
    idx_lookup[elm] = board.size()-1;
    bubble_up(idx_lookup[elm]);
  } 
  else {
    long oldscore = board[idx_lookup[elm]].first;
    board[idx_lookup[elm]].first = score;
    if(oldscore < score) bubble_up(idx_lookup[elm]);
    else bubble_down(idx_lookup[elm]);
  }

}

void 
leaderboard::remove(counter_idx_t elm){
  
  idx_t idx = idx_lookup[elm];
  swap(idx, board.size()-1);
  bubble_down(idx);
  board.pop_back();

  // board[idx_lookup[elm]].first = 0;
  // bubble_down(idx_lookup[elm]);
  idx_lookup.erase(elm);
  // board.pop_back();
}


bool
leaderboard::contains(counter_idx_t elm) const {
  return idx_lookup.count(elm) != 0;
}

unsigned int
leaderboard::size() const {
  return board.size();
}

std::vector<std::pair< long, counter_idx_t> >
leaderboard::get_range(idx_t from, idx_t to) const {
  std::vector<std::pair<long, counter_idx_t> > ret;
  for(idx_t idx = std::min(from, (idx_t)board.size()-1);
      idx < std::min(to, (idx_t)board.size()); idx++){
    ret.push_back(board[idx]);
  }
  return ret;
}



void leaderboard::print() {
  for(auto p : board) {
    std::cout << p.second << ":" << p.first << std::endl;  
  }
}




#include <iostream>
#include <algorithm>
#include "leaderboard.hh"
#include "event_array.hh"


void
leaderboard::add(std::string elm, long score) {
  if(!contains(elm)) {
    board.push_back(make_pair(score, elm));
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
leaderboard::remove(std::string elm){
  board[idx_lookup[elm]].first = -1;
  bubble_down(idx_lookup[elm]);
  idx_lookup.erase(elm);
  board.pop_back();
}


bool
leaderboard::contains(const count_name_t elm) const {
  return idx_lookup.count(elm) != 0;
}

unsigned int
leaderboard::size() const {
  return board.size();
}

std::vector<std::pair< long, count_name_t> >
leaderboard::get_range(idx_t from, idx_t to) const {
  std::vector<std::pair<long, count_name_t> > ret;
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


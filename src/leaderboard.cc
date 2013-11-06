#include <iostream>
#include "leaderboard.hh"
#include "event_array.hh"


void
leaderboard::add(std::string elm, long score) {
  board.push_back(std::make_pair(elm, score));
  map[elm] = board.size() -1;
  update_up(elm, score);
}

void 
leaderboard::remove(std::string elm){
  idx_t idx      = map[elm];
  board[idx]     = board.back();
  board.pop_back();
  update_down(board[idx].first, board[idx].second);
}


void leaderboard::swap(count_name_t n0, count_name_t n1) {
  idx_t idx_0 = map[n0];
  idx_t idx_1 = map[n1];

  auto tmp = board[idx_1];
  board[idx_1] = board[idx_0];
  board[idx_0] = tmp;

  map[n0] = idx_1;
  map[n1] = idx_0;
}

void leaderboard::update_up(count_name_t elm, long score) {
  board[map[elm]].second = score;
  if(map[elm] == 0) return;
  long score_above = board[ map[elm] - 1 ].second;
  count_name_t name_above = board[ map[elm] - 1 ].first;
  if(score > score_above){
    swap(elm, name_above);
    update_up(elm, score);
  }
}

void leaderboard::update_down(count_name_t elm, long score) {
  if(map[elm] == board.size() -1) return;
  long score_below = board[map[elm] + 1].second;
  count_name_t name_below = board[ map[elm] + 1 ].first;
  if(score < score_below){
    swap(elm, name_below);
    update_down(elm, score);
  }
}

void leaderboard::print() {
  for(auto p : board) {
    std::cout << p.first << ":" << p.second << std::endl;  
  }
}


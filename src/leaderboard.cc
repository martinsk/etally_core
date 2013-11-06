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

void leaderboard::update_up(count_name_t elm, long score) {
  if(map[elm] == 0) return;
  long score_above = board[map[elm] - 1].second;
  if(score > score_above){
    auto tmp = board[map[elm]];
    board[map[elm] - 1] = board[map[elm]];
    board[map[elm]] = tmp;
    update_up(elm, score);
  }
}

void leaderboard::update_down(count_name_t elm, long score) {
  if(map[elm] == board.size() -1) return;
  long score_below = board[map[elm] + 1].second;
  if(score < score_below){
    auto tmp = board[map[elm]];
    board[map[elm] + 1] = board[map[elm]];
    board[map[elm]] = tmp;
    update_down(elm, score);
  }
}

void leaderboard::print() {
  for(auto p : board) {
    std::cout << p.first << ":" << p.second << std::endl;  
  }
}


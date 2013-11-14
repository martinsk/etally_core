#include <iostream>
#include "leaderboard.hh"
#include "event_array.hh"


void
leaderboard::add(std::string elm, long score) {
  if(contains(elm)) {
    remove(elm);
  }

  score_map[elm] = score;
  board.insert(std::make_pair(-score, elm));
}

void 
leaderboard::remove(std::string elm){
  unsigned long old_score = score_map[elm];
  board.erase(std::make_pair(-old_score, elm));
  //  print();
}


bool
leaderboard::contains(const count_name_t elm) const {
  return score_map.count(elm) != 0;
}


void leaderboard::print() {
  for(auto p : board) {
    std::cout << p.second << ":" << p.first << std::endl;  
  }
}


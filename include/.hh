#ifndef LEADERBOARD_HH
#define LEADERBOARD_HH

class leaderboard{
protected:
  std::vector<idx_t> board;
  event_array* parent;

private:
  std::unordered_map<idx_t, unsigned int> inverse_index ;

public:
  leaderboard(event_array* parent);

  void add(idx_t elm);
  void remove(idx_t, elm);
  void update_up(idx_t elm);  
  void update_down(idx_t elm);  
};

#endif // LEADERBOARD_HH

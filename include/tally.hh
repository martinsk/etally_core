#ifndef _TALLY_HH
#define _TALLY_HH

#include "types.hh"
#include "event_array.hh"
#include "event_metric_array.hh"
#include "leaderboard.hh"
#include "idx_assigner.hh"


class tally {
private:  
  std::vector<unsigned> intervals;
  std::vector<unsigned> leaderboard_intervals;
  std::vector<unsigned> percentile_intervals;
  
  std::vector<event_array*> event_arrays;

  std::vector<event_metric_array*> event_metric_arrays;

public:
  idx_assigner count_leaderboard_idx_assigner;
  idx_assigner count_identifier_idx_assigner;
  
  idx_assigner metric_leaderboard_idx_assigner;
  idx_assigner metric_identifier_idx_assigner;

  bool started = false;
  
  circular_queue<std::pair<std::vector<counter_idx_t>,timestamp_t> > count_buffer_queue;
  circular_queue<std::pair<std::pair<std::vector<counter_idx_t>,timestamp_t>, metric_payload_t> > metric_buffer_queue;

public:
  tally(std::vector<unsigned> intervals,
        std::vector<unsigned> leaderboard_intervals,
        std::vector<unsigned> percentile_intervals);
  ~tally();

  void handle_count_event(const std::vector< counter_idx_t>& counters, timestamp_t timestamp);
  void handle_metric_event(const std::vector< counter_idx_t>& counters, timestamp_t timestamp, metric_payload_t payload);
  void update(timestamp_t ts);
  void sort();

  // count calls
  std::vector<std::pair<unsigned int, int> > count_get_interval_counters(counter_idx_t) const;
  std::vector<std::pair<long, counter_idx_t> > count_get_leaderboard(leaderboard_idx_t leaderboard_idx,
                                                                     unsigned interval,
                                                                     int page, 
                                                                     int page_size) const;
  const std::vector<unsigned>& count_get_leaderboard_intervals() const {return leaderboard_intervals;} 



  // metric calls
  std::vector<std::pair<metric_counter_stats, timestamp_t> > metric_get_interval_counters(counter_idx_t idx) const;
  std::vector<std::pair<unsigned int, unsigned int> >  metric_get_percentile(counter_idx_t idx, float percent) const;

  std::vector<std::pair<long, counter_idx_t> > metric_get_leaderboard(leaderboard_idx_t leaderboard_idx,
                                                                      unsigned interval,
                                                                      int page, 
                                                                      int page_size) const;
  const std::vector<unsigned>& metric_get_leaderboard_intervals() const {return leaderboard_intervals;} 

  const std::vector<unsigned>& metric_get_percentile_intervals() const {return percentile_intervals;} 
};

#endif

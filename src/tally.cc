
#include <chrono>
#include <algorithm>
#include <iostream>
#include <queue>  
#include <string>
#include <unordered_map>


#include "tally.hh"


tally::tally(std::vector<unsigned> intervals,
             std::vector<unsigned> leaderboard_intervals,
             std::vector<unsigned> percentile_intervals)
  : intervals(intervals),
    leaderboard_intervals(leaderboard_intervals),
    percentile_intervals(percentile_intervals)
{
  for(auto i = intervals.begin(); i != intervals.end(); i++) {
    if(event_arrays.empty())  event_arrays.push_back(new event_array(*i));
    else event_arrays.push_back(new event_array(*i, event_arrays.back()));

    if(event_metric_arrays.empty())  event_metric_arrays.push_back(new event_metric_array(*i));
    else event_metric_arrays.push_back(new event_metric_array(*i, event_metric_arrays.back()));
  }
  std::reverse(event_arrays.begin(),        event_arrays.end());
  std::reverse(event_metric_arrays.begin(), event_metric_arrays.end());

}

tally::~tally() {}

void
tally::handle_count_event(const std::vector< counter_idx_t>& counters,
                    timestamp_t timestamp) {  
  for(auto counter : counters) {
    event_array::insert_counters[counter] ++;
    // add counter to leaderboard
    long insert_count = event_array::insert_counters[counter];
    
    for(auto lb : event_array::lb_lookup_map[counter]) {
      for(auto& event_array : event_arrays) {
        long adjusted_count = insert_count - event_array->counters[counter];
        if ( event_array::lb_map[lb].count(event_array->timespan) )
          event_array::lb_map[lb][event_array->timespan]->add(counter, adjusted_count );
      }
    }

  }

  event_arrays.front()->event(counters, timestamp);
}

void
tally::update(timestamp_t ts) {
  for(auto event_array : event_arrays)
    event_array->update(ts);

  for(auto event_metric_array : event_metric_arrays)
    event_metric_array->update(ts);
}

void
tally::sort() {
  time_t now = time(0);
  for(auto& er : event_arrays){
    er->sort();
    er->update(now);
  }
}

std::vector< std::pair<unsigned int, int> >
tally::count_get_interval_counters(counter_idx_t counter_idx) const {
  std::vector<std::pair<unsigned int, int> > ret;
  if(counter_idx < event_array::insert_counters.length()) {
    unsigned long insert_count = event_array::insert_counters[counter_idx];
    for(auto& evr : event_arrays)
      ret.push_back(std::make_pair(evr->timespan,  insert_count - evr->get_counter(counter_idx) ));
  }
  else 
    for(auto& evr : event_arrays)
      ret.push_back(std::make_pair(evr->timespan,  0 ));
  
  return ret;
}

std::vector<std::pair<long, counter_idx_t> >
tally::count_get_leaderboard(leaderboard_idx_t leaderboard_idx,
                       unsigned interval, 
                       int page, 
                       int page_size) const {

  if(event_array::lb_map.count(leaderboard_idx) != 0
     && event_array::lb_map[leaderboard_idx].count(interval) != 0) {
    auto leaderboard_instance = event_array::lb_map[leaderboard_idx][interval];
    return leaderboard_instance->get_range( page*page_size, page_size + page*page_size);
  }else
    return std::vector<std::pair<long, counter_idx_t> >();
}



void
tally::handle_metric_event(const std::vector< counter_idx_t>& counters,
                           timestamp_t timestamp, metric_payload_t payload) {  
  for(auto counter : counters) {
    event_metric_array::insert_counters[counter] ++;
    event_metric_array::insert_sums[counter]   += payload;
    event_metric_array::insert_sqsums[counter] += payload*payload;

    // add counter to leaderboard
    long insert_count = event_metric_array::insert_counters[counter];
    
    for(auto lb : event_metric_array::lb_lookup_map[counter]) {
      for(auto& event_metric_array : event_metric_arrays) {
        long adjusted_count = insert_count - event_metric_array->counters[counter];
        
        if ( event_metric_array::lb_map[lb].count(event_metric_array->timespan) )
          event_metric_array::lb_map[lb][event_metric_array->timespan]->add(counter, adjusted_count );
      }
    }
  
    if(event_metric_array::percentile_map.count(counter))
      for(auto& event_array : event_arrays) {
        if (event_metric_array::percentile_map[counter].count(event_array->timespan))
          event_metric_array::percentile_map[counter][event_array->timespan] -> insert(payload);
      }
  }

  event_metric_arrays.front()->event(counters, payload, timestamp);
}

std::vector< std::pair<struct metric_counter_stats, timestamp_t> >
tally::metric_get_interval_counters(counter_idx_t counter_idx) const {
  
  std::vector<std::pair<struct metric_counter_stats,timestamp_t> > ret;
  if(counter_idx < event_metric_array::insert_counters.length()) {
      unsigned long insert_count      = event_metric_array::insert_counters[counter_idx];
      unsigned long long insert_sum   = event_metric_array::insert_sums[counter_idx];
      unsigned long long insert_sqsum = event_metric_array::insert_sqsums[counter_idx];
    for(auto evr : event_metric_arrays){

      unsigned long adjusted_count  = insert_count - evr->counters[counter_idx];
      unsigned long adjusted_sum    = insert_sum   - evr->sums[counter_idx];
      unsigned long adjusted_sqsum  = insert_sqsum - evr->sqsums[counter_idx];

      struct metric_counter_stats stats = {.count = adjusted_count,
                                           .sum   = adjusted_sum,
                                           .sqsum = adjusted_sqsum};
      
      ret.push_back(std::make_pair(stats, evr->timespan));
    }
  }
  else 
    for(auto& evr : event_metric_arrays){
      struct metric_counter_stats stats = {.count = 0,
                                           .sum   = 0,
                                           .sqsum = 0};
      
      ret.push_back(std::make_pair(stats, evr->timespan));
    }
  
  return ret;
}


std::vector<std::pair<unsigned int, unsigned int> >  tally::metric_get_percentile(counter_idx_t idx, float percent) const {  
  std::vector<std::pair<unsigned int, unsigned int> > ret;

  if(event_metric_array::percentile_map.count(idx ))
    for(auto interval : metric_get_percentile_intervals()){ 
      unsigned int count = event_metric_array::percentile_map[idx][interval]->find_percentile(percent);
      ret.push_back(std::make_pair(interval, count));
    }
  return ret;
}

std::vector<std::pair<long, counter_idx_t> >
tally::metric_get_leaderboard(leaderboard_idx_t leaderboard_idx,
                              unsigned interval, 
                              int page, 
                              int page_size) const {

  if(event_metric_array::lb_map.count(leaderboard_idx) != 0
     && event_metric_array::lb_map[leaderboard_idx].count(interval) != 0) {
    auto leaderboard_instance = event_metric_array::lb_map[leaderboard_idx][interval];
    return leaderboard_instance->get_range( page*page_size, page_size + page*page_size);
  }else
    return std::vector<std::pair<long, counter_idx_t> >();
}

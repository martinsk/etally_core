#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <time.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 

#include "erl_interface.h"
#include "ei.h"

#include <chrono>
#include <algorithm>
#include <iostream>
#include <queue>  
#include <string>
#include <unordered_map>

#include "types.hh"
#include "tally.hh"
#include "event_metric_array.hh"
#include "leaderboard.hh"
#include "idx_assigner.hh"

#include "erl_utils.hh"


inline std::string term2str(ETERM* ch_str) {
  unsigned char* str = ERL_BIN_PTR(ch_str);
  return std::string(str, str +  ERL_BIN_SIZE(ch_str));
}

void metric_bind_leaderboards(ETERM* binding_list, tally& tally_srv){
  struct {
    tally* tally_srv;
    void parse(ETERM* term) {
      std::string counter_id = term2str(erl_element(1, term));
      std::string lb_id      = term2str(erl_element(2, term));
      leaderboard_idx_t leaderboard_idx = tally_srv->metric_leaderboard_idx_assigner.get_idx(lb_id);
      counter_idx_t counter_idx = tally_srv->metric_identifier_idx_assigner.get_idx(counter_id);
      if(event_metric_array::lb_map.count(leaderboard_idx) == 0) {
        for(auto i : tally_srv->metric_get_leaderboard_intervals()) {
          event_metric_array::lb_map[leaderboard_idx][i] = new leaderboard;
        }
      }
      event_metric_array::lb_lookup_map[counter_idx].insert(leaderboard_idx);
    }
  } binding_list_parser;
  binding_list_parser.tally_srv = &tally_srv;
  parse_term_list(binding_list, binding_list_parser);
}


void bind_percentiles(ETERM* percentile_list, tally& tally_srv){
  struct {
    tally* tally_srv;
    void parse(ETERM* term) {
      std::string counter_id = term2str(term);
      counter_idx_t counter_idx = tally_srv->metric_identifier_idx_assigner.get_idx(counter_id);
      if(event_metric_array::percentile_map.count(counter_idx) == 0) {
        for(auto i : tally_srv->metric_get_percentile_intervals()) {
          event_metric_array::percentile_map[counter_idx][i] = new percentile(900);
        }
      }
    }
  } percentile_list_parser;
  percentile_list_parser.tally_srv = &tally_srv;
  parse_term_list(percentile_list, percentile_list_parser);
}

void metric_handle_event_timestamp(ErlMessage& emsg, tally& tally_srv) {

  ETERM *tuplep           = erl_element(ERL_TUPLE_SIZE(emsg.msg), emsg.msg);
  ETERM *event_list       = erl_element(2, tuplep);
  ETERM *binding_list     = erl_element(3, tuplep);
  ETERM *percent_list     = erl_element(4, tuplep); 
  ETERM *ts               = erl_element(5, tuplep);
  ETERM *payload          = erl_element(6, tuplep);

  struct {
    tally* tally_srv;
    typedef counter_idx_t return_type;
    return_type construct(ETERM* term) {
      counter_name_t counter_name = term2str(term);
      counter_idx_t counter_idx = tally_srv->metric_identifier_idx_assigner.get_idx(counter_name);
      return counter_idx;
    }
  } counter_idx_constructor;
  counter_idx_constructor.tally_srv = &tally_srv;

  std::vector< counter_idx_t> counters = construct_vector_from_term(event_list, counter_idx_constructor);
  std::sort(counters.begin(), counters.end());

  metric_bind_leaderboards( binding_list, tally_srv );



  bind_percentiles( percent_list, tally_srv );
  
  auto& buffer_queue = tally_srv.metric_buffer_queue;
  while((!buffer_queue.empty()) && (buffer_queue.front().second < ERL_INT_VALUE(ts))){
    tally_srv.handle_metric_event(buffer_queue.front().first.first,
                           buffer_queue.front().first.second,
                           buffer_queue.front().second );
    buffer_queue.dequeue();
  }

  tally_srv.handle_metric_event(counters, ERL_INT_UVALUE(ts), ERL_INT_UVALUE(payload));
  erl_free_compound(tuplep); 
}


void metric_handle_event(ErlMessage& emsg, tally& tally_srv) {
  ETERM *tuplep       = erl_element(3, emsg.msg);
  ETERM *event_list   = erl_element(2, tuplep);
  ETERM *binding_list = erl_element(3, tuplep);
  ETERM *payload      = erl_element(4, tuplep);

 
  std::vector< counter_idx_t> counters;
  while(!ERL_IS_NIL(event_list)){
    ETERM* head = ERL_CONS_HEAD(event_list);
    counter_name_t counter_name = term2str(head);
    counter_idx_t counter_idx = tally_srv.metric_identifier_idx_assigner.get_idx(counter_name);
    counters.push_back(counter_idx);
    event_list =  ERL_CONS_TAIL(event_list);
  }
  std::sort(counters.begin(), counters.end());

  metric_bind_leaderboards( binding_list, tally_srv );
            
  if(tally_srv.started) tally_srv.handle_metric_event(counters, time(0), ERL_INT_UVALUE(payload));
  else tally_srv.metric_buffer_queue.enqueue(make_pair(make_pair(counters,
                                                                 time(0)), ERL_INT_UVALUE(payload)));
            
  erl_free_compound(tuplep); 
}

void metric_handle_get_counter(ErlMessage& emsg, tally& tally_srv, int fd){
  ETERM *fromp  = erl_element(2, emsg.msg);
  ETERM* tuplep = erl_element(3, emsg.msg);
            
  counter_name_t counter_name = term2str(erl_element(2, tuplep));
  counter_idx_t counter_idx =  tally_srv.metric_identifier_idx_assigner.get_idx(counter_name);
            
  ETERM* counter_list = erl_mk_empty_list();
  for(auto& cc : tally_srv.metric_get_interval_counters(counter_idx)) {
    ETERM* tuple[4];
    tuple[0] = erl_mk_uint(cc.second);
    tuple[1] = erl_mk_ulonglong(cc.first.count);
    tuple[2] = erl_mk_ulonglong(cc.first.sum);
    tuple[3] = erl_mk_ulonglong(cc.first.sqsum);
    ETERM* hd = erl_mk_tuple(tuple, 4);
    ETERM* new_counter_list = erl_cons(hd, counter_list);
    erl_free_term(hd);
    erl_free_term(counter_list);
    counter_list = new_counter_list;
    erl_free_term(tuple[0]);
    erl_free_term(tuple[1]);
    erl_free_term(tuple[2]);
    erl_free_term(tuple[3]);
  }

  ETERM* percentile_list = erl_mk_empty_list();
  std::vector<float> percentiles = {.50, .75, .80, .90, .95, .99, .999};
  for(auto& percent : percentiles) {
    ETERM* interval_list = erl_mk_empty_list();
    for(auto& res : tally_srv.metric_get_percentile(counter_idx, percent)) {
      ETERM* pair[2];
      pair[0] = erl_mk_uint(res.first);
      pair[1] = erl_mk_uint(res.second);
      ETERM* hd = erl_mk_tuple(pair, 2);
      ETERM* new_interval_list = erl_cons(hd, interval_list);
      //erl_free_term(interval_list);
      interval_list = new_interval_list;
      //erl_free_term(hd);
      // erl_free_term(pair[0]);
      // erl_free_term(pair[1]);
    }
    ETERM* pair[2];
    pair[0] = erl_mk_float(percent);
    pair[1] =  interval_list;
    ETERM* pair_tuple = erl_mk_tuple(pair, 2);
    // erl_free_term(pair[0]);
    // erl_free_term(pair[1]);
 
    ETERM* new_percentile_list = erl_cons(pair_tuple, percentile_list); 
    erl_free_term(pair_tuple);

    erl_free_term(interval_list);
    percentile_list = new_percentile_list;
 
 }


  ETERM* tuple[2];
  tuple[0] = erl_mk_atom("tally");

  ETERM* pair[2];
  pair[0] = counter_list;
  pair[1] = percentile_list;
  tuple[1] = erl_mk_tuple(pair, 2);
  ETERM* resp = erl_mk_tuple(tuple, 2);
  
  erl_free_term(tuple[0]);
  erl_free_term(tuple[1]);
  
  erl_free_term(pair[0]);
  erl_free_term(pair[1]);
            
  erl_send(fd, fromp, resp);

  erl_free_compound(tuplep); 
  erl_free_term(fromp); 
  erl_free_compound(resp); 
}

void metric_handle_get_leaderboard(ErlMessage& emsg, tally& tally_srv, int fd) {
  ETERM *fromp   = erl_element(2, emsg.msg);
  ETERM* tuplep  = erl_element(ERL_TUPLE_SIZE(emsg.msg), emsg.msg);
  ETERM* lb_id_p = erl_element(2, tuplep);
            
  std::string  lb_id  = term2str(lb_id_p);
  unsigned int lb_dim = ERL_INT_UVALUE(erl_element(3, tuplep));

  erl_free_term(lb_id_p);

  int page = 0;
  int page_size = 10;

  ETERM* page_p = erl_element(4, tuplep);
  if (page_p) page = ERL_INT_UVALUE(page_p);

            
  leaderboard_idx_t leaderboard_idx = tally_srv.metric_leaderboard_idx_assigner.get_idx(lb_id);
            
  ETERM* list = erl_mk_empty_list();
  if(event_metric_array::lb_map.count(leaderboard_idx) != 0 && event_metric_array::lb_map[leaderboard_idx].count(lb_dim) != 0){
    auto board = tally_srv.metric_get_leaderboard(leaderboard_idx, lb_dim, page,  page_size);
    for(auto entry = board.begin(); (entry != board.end()); entry++) {
      counter_name_t counter_name = tally_srv.metric_identifier_idx_assigner.get_token(entry->second);
      ETERM* tuple[2];
      tuple[0] = erl_mk_binary(counter_name.c_str(), counter_name.length());
      tuple[1] = erl_mk_uint(entry->first);
      ETERM* hd = erl_mk_tuple(tuple, 2);
      ETERM* new_list = erl_cons(hd, list);
      erl_free_term(list);
      list = new_list;
      erl_free_term(hd);
      erl_free_term(tuple[0]);
      erl_free_term(tuple[1]);
    }
  }

  ETERM* tuple[2];
  tuple[0] = erl_mk_atom("tally");
  tuple[1] = list;
  ETERM* resp = erl_mk_tuple(tuple, 2);
  erl_free_term(tuple[0]);
  erl_free_term(tuple[1]);
            
  erl_send(fd, fromp, resp);

  erl_free_term(fromp); 
  erl_free_compound(tuplep); 
  erl_free_compound(resp); 
}



void metric_handle_list_leaderboards(ErlMessage& emsg, tally& tally_srv, int fd) { 
  ETERM *fromp = erl_element(2, emsg.msg);
  ETERM* list  = erl_mk_empty_list();
  
  for(auto board : event_array::lb_map) {
    for(auto i : tally_srv.metric_get_leaderboard_intervals()) {
      ETERM* tuple[2];
      std::string leaderboard_id = tally_srv.metric_leaderboard_idx_assigner.get_token(board.first);
      tuple[0] = erl_mk_binary(leaderboard_id.c_str(), leaderboard_id.length());
      tuple[1] = erl_mk_uint(i);
      list = erl_cons(erl_mk_tuple(tuple, 2), list);
    }
  }
  
  ETERM* tuple[2];
  tuple[0] = erl_mk_atom("tally");
  tuple[1] = list;
  
  ETERM* resp = erl_mk_tuple(tuple, 2);  
  erl_free_term(tuple[0]);
  erl_free_term(tuple[1]);

  erl_send(fd, fromp, resp);

  erl_free_term(fromp); 
  erl_free_compound(resp); 
}

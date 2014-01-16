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
#include "event_array.hh"
#include "leaderboard.hh"
#include "idx_assigner.hh"

#include "erl_utils.hh"

#include "count_handlers.hh"
#include "metric_handlers.hh"

#include "test_suite.hh"


#define BUFSIZE 1024

bool is_call_funcname(std::string func_name, ETERM* message);

#define IS_CALL_SYSTEM_START(X)             (is_call_funcname("system_start", X))

#define IS_CALL_COUNT_ORDER(X)             (is_call_funcname("count_order", X))
#define IS_CALL_COUNT_HANDLE_EVENT(X)      (is_call_funcname("count_event", X))
#define IS_CALL_COUNT_HANDLE_EVENT_TZ(X)   (is_call_funcname("count_event_tz", X))
#define IS_CALL_COUNT_GET_COUNTER(X)       (is_call_funcname("count_get_counter", X))
#define IS_CALL_COUNT_GET_LEADERBOARD(X)   (is_call_funcname("count_get_leaderboard", X))
#define IS_CALL_COUNT_LIST_LEADERBOARDS(X) (is_call_funcname("count_list_leaderboards", X))

#define IS_CALL_METRIC_HANDLE_EVENT(X)      (is_call_funcname("metric_event", X))
#define IS_CALL_METRIC_ORDER(X)             (is_call_funcname("metric_order", X))
#define IS_CALL_METRIC_HANDLE_EVENT_TZ(X)   (is_call_funcname("metric_event_tz", X))
#define IS_CALL_METRIC_GET_COUNTER(X)       (is_call_funcname("metric_get_counter", X))
#define IS_CALL_METRIC_GET_LEADERBOARD(X)   (is_call_funcname("metric_get_leaderboard", X))
#define IS_CALL_METRIC_LIST_LEADERBOARDS(X) (is_call_funcname("metric_list_leaderboards", X))


int my_listen(int port);

std::string term2str(ETERM* ch_str) {
  unsigned char* str = ERL_BIN_PTR(ch_str);
  return std::string(str, str +  ERL_BIN_SIZE(ch_str));
}


std::ostream& operator<<(std::ostream& out, const struct event e) {
  out << "{event, " << e.timestamp << ", " << e.event_idx << "}";
  return out;
}

std::ostream& operator<<(std::ostream& out, const struct event_metric e) {
  out << "{metric_event, " << e.timestamp << ", " << e.event_idx << ", " << e.payload << "}";
  return out;
}

void handle_start(ErlMessage& emsg, tally& tally_srv) {
  std::cout << "handle start" << std::endl;
  
  while(!tally_srv.count_buffer_queue.empty()){
    tally_srv.handle_count_event(tally_srv.count_buffer_queue.front().first,
                                 tally_srv.count_buffer_queue.front().second);
    tally_srv.count_buffer_queue.dequeue();
  }

  while(!tally_srv.metric_buffer_queue.empty()){
    tally_srv.handle_metric_event(tally_srv.metric_buffer_queue.front().first.first,
                                  tally_srv.metric_buffer_queue.front().first.second,
                                  tally_srv.metric_buffer_queue.front().second);


    tally_srv.metric_buffer_queue.dequeue();
  }

  tally_srv.started = true;
}



int main(int argc, char **argv) {
  std::cout << "stating tally at UnixTimeStamp: " << time(0) << std::endl; 
  

  counter_array<int> c_arr;

  std::cout << "c_arr[1] : "<< c_arr[1] << std::endl;

  // assert(test_suite());

  if(argc != 5) {
    std::cout << "usage: ./tally_srv <ip> <localname> <longname> <cookie>" << std::endl;
    std::cout << "example: ./etally_srv \"127.0.0.1\" \"msk-dev\" \"tally@msk-dev.local\" \"secretcookie\"" << std::endl;
    return -1;
  }
  
  struct in_addr addr;                     /* 32-bit IP number of host */
  int listen;                              /* Listen socket */
  int fd;                                  /* fd to Erlang node */
  ErlConnect conn;                         /* Connection data */

  int receive_status;                      /* Result of receive */
  unsigned char buf[BUFSIZE];              /* Buffer for incoming message */
  ErlMessage emsg;                         /* Incoming message */

  int port            =    3456;
  char *erl_secret    = argv[4];
  char erl_node[]     = "tally";
  char *erl_localname = argv[2];
  char *erl_longname  = argv[3];
   
  const unsigned int hour  = 60*60;
  const unsigned int day   = 24*hour;
  const unsigned int week  = 7*day;
  const unsigned int month = 4 *week;

  std::vector<unsigned> intervals = {5*60,
                                     hour,   2*hour,
                                     day,    2*day,  3*day, 4*day, 5*day, 6*day,
                                     week,   2*week,
                                     month,  2*month};
  
  std::vector<unsigned> leaderboard_intervals = {hour,
                                                 day,
                                                 week,
                                                 month};

  std::vector<unsigned> percentile_intervals = {hour,
                                                 day,
                                                 week,
                                                 month};
  
  std::reverse(intervals.begin(), intervals.end());
  tally tally_srv(intervals,
                  leaderboard_intervals,
                  percentile_intervals);

  erl_init(NULL, 0);
  
  addr.s_addr = inet_addr(argv[1]); 
  if (erl_connect_xinit(erl_localname, erl_node, erl_longname, &addr, erl_secret, 0) == -1)
    erl_err_quit("erl_connect_xinit failed");  
  if ((listen = my_listen(port)) <= 0)
    erl_err_quit("my_listen failed ( likely unable to connect to socket )");
  if (erl_publish(port) == -1)
    erl_err_quit("erl_publish failed");
    
  while(true) {
    
    if ((fd = erl_accept(listen, &conn)) == ERL_ERROR)  erl_err_quit("erl_accept failed");

    int count = 0;
    time_t previous_ts = time(0);
    int loop = 1;                            /* Loop flag */
    while (loop) {
      receive_status = erl_receive_msg(fd, buf, BUFSIZE, &emsg);
      
      if (receive_status == ERL_TICK) { /* ignore */ }
      else if (receive_status == ERL_ERROR) {
        std::cerr << "[warning] connection to remote node dropped - will try to reconnect" << std::endl;
        loop= 0;
      } else {
        if (emsg.type == ERL_REG_SEND) {
          time_t current_ts = time(0); 
          count ++;
          if(current_ts != previous_ts) {
            tally_srv.update(current_ts);
            //            std::cout << "count =  " << count << std::endl;
            count = 0;
          }
          if (IS_CALL_SYSTEM_START           (emsg.msg)) {
            std::cout << "start call" << std::endl;
            handle_start(emsg, tally_srv);
          }
          else if (IS_CALL_COUNT_ORDER            (emsg.msg)) { tally_srv.sort(); }
          else if (IS_CALL_COUNT_HANDLE_EVENT_TZ  (emsg.msg)) count_handle_event_timestamp  (emsg, tally_srv);
          else if (IS_CALL_COUNT_HANDLE_EVENT     (emsg.msg)) count_handle_event            (emsg, tally_srv);
          else if (IS_CALL_COUNT_GET_COUNTER      (emsg.msg)) count_handle_get_counter      (emsg, tally_srv, fd);
          else if (IS_CALL_COUNT_GET_LEADERBOARD  (emsg.msg)) count_handle_get_leaderboard  (emsg, tally_srv, fd);
          else if (IS_CALL_COUNT_LIST_LEADERBOARDS(emsg.msg)) count_handle_list_leaderboards(emsg, tally_srv, fd);

          else if (IS_CALL_METRIC_ORDER            (emsg.msg)) { tally_srv.sort(); }
          else if (IS_CALL_METRIC_HANDLE_EVENT_TZ  (emsg.msg)) metric_handle_event_timestamp  (emsg, tally_srv);
          else if (IS_CALL_METRIC_HANDLE_EVENT     (emsg.msg)) metric_handle_event            (emsg, tally_srv);
          else if (IS_CALL_METRIC_GET_COUNTER      (emsg.msg)) metric_handle_get_counter      (emsg, tally_srv, fd);
          else if (IS_CALL_METRIC_GET_LEADERBOARD  (emsg.msg)) metric_handle_get_leaderboard  (emsg, tally_srv, fd);
          else if (IS_CALL_METRIC_LIST_LEADERBOARDS(emsg.msg)) metric_handle_list_leaderboards(emsg, tally_srv, fd);
          
          erl_free_term(emsg.to); 
          erl_free_term(emsg.from); 
          erl_free_term(emsg.msg);

          previous_ts = current_ts; 
        }
      }
    }
    erl_close_connection(fd);
   }
}

  
bool is_call_funcname(std::string func_name, ETERM* call_term) {
  ETERM  *tuplep = erl_element(ERL_TUPLE_SIZE(call_term), call_term);
  ETERM  *fnp    = erl_element(1, tuplep   );
  bool res = (strncmp(ERL_ATOM_PTR(fnp), func_name.c_str(), strlen(func_name.c_str())) == 0);
  erl_free_term(tuplep);
  erl_free_term(fnp);
  return res;
}

int my_listen(int port) {
  int listen_fd;
  struct sockaddr_in addr;
  int on = 1;

  if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return (-1);

  setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

  memset((void*) &addr, 0, (size_t) sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(listen_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
    return (-1);

  listen(listen_fd, 10);
  return listen_fd;
}


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

#include <iostream>
#include <queue>  
#include <string>
#include <unordered_map>

#include "types.hh"
#include "event_array.hh"
#include "leaderboard.hh"


#define BUFSIZE 1024

bool is_call_funcname(std::string func_name, ETERM* message);
#define IS_CALL_HANDLE_EVENT(X)      (is_call_funcname("event", X))
#define IS_CALL_HANDLE_EVENT_TZ(X)      (is_call_funcname("event_tz", X))
#define IS_CALL_GET_COUNTER(X)       (is_call_funcname("get_counter", X))
#define IS_CALL_GET_LEADERBOARD(X)   (is_call_funcname("get_leaderboard", X))
#define IS_CALL_LIST_LEADERBOARDS(X) (is_call_funcname("list_leaderboards", X))


int my_listen(int port);


std::string term2str(ETERM* ch_str) {
  unsigned char* str = ERL_BIN_PTR(ch_str);
  return std::string(str, str +  ERL_BIN_SIZE(ch_str));
}

std::ostream& operator<<(std::ostream& out,  const struct event e)  {
  out << "{" << e.event_idx << ", " << e.timestamp << " } ";
  return out;
}

std::ostream& operator<<(std::ostream& out,  const circular_queue<struct event>& queue)  {
  for(int i = 0; i != queue.size; i ++){ 
    int idx = (queue.back_p - queue.size + i)%queue.container_size;
    out << queue.data[idx] <<" ";
  }
  return out;
}


std::ostream& operator<<(std::ostream& out,  const circular_queue<int>& queue)  {
  for(int i = 0; i != queue.size; i ++){ 
    int idx = (queue.back_p - queue.size + i)%queue.container_size;
    out << queue.data[idx] <<" ";
  }
  return out;
}







int main(int argc, char **argv) {


  

  // tally_core ip localname longname secret
  if(argc != 5) {
    std::cout << "usage: ./tally_core <ip> <localname> <longname> <cookie>" << std::endl;
  }

  


  struct in_addr addr;                     /* 32-bit IP number of host */
  int listen;                              /* Listen socket */
  int fd;                                  /* fd to Erlang node */
  ErlConnect conn;                         /* Connection data */

  int loop = 1;                            /* Loop flag */
  int got;                                 /* Result of receive */
  unsigned char buf[BUFSIZE];              /* Buffer for incoming message */
  ErlMessage emsg;                         /* Incoming message */

  int port = 3456;// atoi(argv[1]);
  char *erl_secret    = argv[4];
  char erl_node[]     = "tally";
  char *erl_localname = argv[2];
  char *erl_longname  = argv[3];
   
 
  const unsigned int hour  = 60*60;
  const unsigned int day   = 24*hour;
  const unsigned int week  = 7*day;
  const unsigned int month = 4 *week;
  std::vector<unsigned> intervals = {10,     20,
                                     hour,   2*hour,
                                     day,    2*day,
                                     week,   2*week,
                                     month,  2*month};

  std::reverse(intervals.begin(), intervals.end());
  std::vector<event_array*> event_arrays;
  for(unsigned i : intervals) {
    if(event_arrays.empty()) event_arrays.push_back(new event_array(i));
    else event_arrays.push_back(new event_array(i, event_arrays.back()));
  }
  std::reverse(event_arrays.begin(), event_arrays.end());

  erl_init(NULL, 0);
    
  addr.s_addr = inet_addr(argv[1]);
 
  if (erl_connect_xinit(erl_localname, erl_node, erl_longname, &addr, erl_secret, 0) == -1)
    erl_err_quit("erl_connect_xinit failed");

  if ((listen = my_listen(port)) <= 0)
    erl_err_quit("my_listen failed ( likely unable to connect to socket )");
  
  if (erl_publish(port) == -1)
    erl_err_quit("erl_publish failed");
  
  if ((fd = erl_accept(listen, &conn)) == ERL_ERROR)
    erl_err_quit("erl_accept failed");
  
  
  while(true){
      
    loop = 1;

    while (loop) {
      // update the datastructure
      event_arrays.front()->update( time(0) );
      
      got = erl_receive_msg(fd, buf, BUFSIZE, &emsg);
      if (got == ERL_TICK) { /* ignore */ }
      else if (got == ERL_ERROR) {
        //      loop = 0;
      } else {
      
        if (emsg.type == ERL_REG_SEND) {
          if(IS_CALL_HANDLE_EVENT(emsg.msg)){
            ETERM* tuplep, *event_list, *binding_list;

            tuplep       = erl_element(3, emsg.msg);
            event_list   = erl_element(2, tuplep);
            binding_list = erl_element(3, tuplep);

            std::vector< std::string> counters;
            while(!ERL_IS_NIL(event_list)){
              ETERM* head = ERL_CONS_HEAD(event_list);
              counters.push_back(term2str(head));
              std::sort(counters.begin(), counters.end());
              event_list =  ERL_CONS_TAIL(event_list);
            }


            while(!ERL_IS_NIL(binding_list)){
              ETERM* head = ERL_CONS_HEAD(binding_list);
              std::string counter_id = term2str(erl_element(1, head));
              std::string lb_id      = term2str(erl_element(2, head));

              // std::cout << "leaderboard bind : "<< counter_id << " to " << lb_id << std::endl;

              if(event_array::lb_map.count(lb_id) == 0) {
                for(auto i : intervals) {
                  event_array::lb_map[lb_id][i] = new leaderboard;
                }
              }
              event_array::lb_lookup_map[counter_id].insert(lb_id);
              binding_list =  ERL_CONS_TAIL(binding_list);
            }
            
            event_arrays.front()->event(counters, time(0));
            erl_free_compound(tuplep); 
          }
          else if(IS_CALL_HANDLE_EVENT_TZ(emsg.msg)){
            ETERM* tuplep, *event_list, *binding_list, *tz;
           
            tuplep       = erl_element(4, emsg.msg);
            event_list   = erl_element(2, tuplep);
            binding_list = erl_element(3, tuplep);
            tz           = erl_element(4, tuplep);

            std::vector< std::string> counters;
            while(!ERL_IS_NIL(event_list)){
              ETERM* head = ERL_CONS_HEAD(event_list);
              counters.push_back(term2str(head));
              std::sort(counters.begin(), counters.end());
              event_list =  ERL_CONS_TAIL(event_list);
            }

            while(!ERL_IS_NIL(binding_list)){
              ETERM* head = ERL_CONS_HEAD(binding_list);
              std::string counter_id = term2str(erl_element(1, head));
              std::string lb_id      = term2str(erl_element(2, head));
              
              // std::cout << "leaderboard bind : "<< counter_id << " to " << lb_id << std::endl;

              if(event_array::lb_map.count(lb_id) == 0) {
                for(auto i : intervals) {
                  event_array::lb_map[lb_id][i] = new leaderboard;
                }
              }
              event_array::lb_lookup_map[counter_id].insert(lb_id);
              binding_list =  ERL_CONS_TAIL(binding_list);
            }
            
            event_arrays.front()->event(counters,ERL_IS_INTEGER(tz));
            erl_free_compound(tuplep); 
          }
          else if (IS_CALL_GET_COUNTER(emsg.msg)) {
            ETERM *fromp;
            fromp = erl_element(2, emsg.msg);

            ETERM* tuplep;
            tuplep = erl_element(3, emsg.msg);
            
            std::string id_str = term2str(erl_element(2, tuplep));
            
            ETERM* list = erl_mk_empty_list();
            for(auto& events : event_arrays) {
            
              ETERM* tuple[2];
              tuple[0] = erl_mk_uint(events->timespan);
              tuple[1] = erl_mk_ulonglong(events->counters[id_str]);
              list = erl_cons(erl_mk_tuple(tuple, 2), list);
            }

            ETERM* tuple[2];
            tuple[0] = erl_mk_atom("tally");
            tuple[1] = list;
            ETERM* resp = erl_mk_tuple(tuple, 2);
                
            erl_send(fd, fromp, resp);
            erl_free_term(fromp); 
            erl_free_term(resp); 
          }
          else if (IS_CALL_GET_LEADERBOARD(emsg.msg)) {
            ETERM *fromp  = erl_element(2, emsg.msg);
            ETERM* tuplep = erl_element(3, emsg.msg);
            std::string  lb_id  = term2str(erl_element(2, tuplep));
            unsigned int lb_dim = ERL_INT_UVALUE(erl_element(3, tuplep));
            ETERM* list = erl_mk_empty_list();

            if(event_array::lb_map.count(lb_id) != 0 && event_array::lb_map[lb_id].size() != 0){
              auto& board = event_array::lb_map[lb_id][lb_dim]->board;
              for(auto entry = board.begin(); entry != board.end(); entry++) {
                ETERM* tuple[2];
                tuple[0] = erl_mk_binary(entry->second.c_str(), entry->second.length());
                tuple[1] = erl_mk_uint(entry->first);
                
                list = erl_cons(erl_mk_tuple(tuple, 2), list);
              }
            }

            ETERM* tuple[2];
            tuple[0] = erl_mk_atom("tally");
            tuple[1] = list;
            ETERM* resp = erl_mk_tuple(tuple, 2);

            erl_send(fd, fromp, resp);
            erl_free_term(fromp); 
            erl_free_compound(resp); 
          }

          else if (IS_CALL_LIST_LEADERBOARDS(emsg.msg)) {
            ETERM *fromp = erl_element(2, emsg.msg);
            ETERM* list  = erl_mk_empty_list();
            for(auto board : event_array::lb_map) {
              for(auto i : intervals) {
                ETERM* tuple[2];
                tuple[0] = erl_mk_binary(board.first.c_str(), board.first.length());
                tuple[1] = erl_mk_uint(i);
                list = erl_cons(erl_mk_tuple(tuple, 2), list);
              }
            }
            ETERM* tuple[2];
            tuple[0] = erl_mk_atom("tally");
            tuple[1] = list;
            ETERM* resp = erl_mk_tuple(tuple, 2);
            
            erl_send(fd, fromp, resp);
            erl_free_term(fromp); 
            erl_free_compound(resp); 
          }

          erl_free_term(emsg.to); 
          erl_free_term(emsg.from); 
          erl_free_term(emsg.msg);
        
        }
      }
      
    } /* while */
    erl_close_connection(fd);
  }
}

  
bool is_call_funcname(std::string func_name, ETERM* call_term) {
  ETERM  *tuplep = erl_element(3, call_term);
  ETERM  *fnp    = erl_element(1, tuplep   );
  
  // printf("testing : %s \n", ERL_ATOM_PTR(fnp));
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

  listen(listen_fd, 100);
  return listen_fd;
}


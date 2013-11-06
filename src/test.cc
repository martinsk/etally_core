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


#include "types.hh"
#include "event_array.hh"
#include "leaderboard.hh"


#define BUFSIZE 1024

bool is_call_funcname(char func_name[], ETERM* message);
#define IS_CALL_HANDLE_EVENT(X)  (is_call_funcname("handle_event", X))
#define IS_CALL_GET_COUNTER(X)   (is_call_funcname("get_counter", X))


int my_listen(int port);

int main(int argc, char **argv) {

  leaderboard lb;
  lb.print();
  std::cout  << "loaded" << std::endl;
  lb.add("Martin", 10);
  lb.add("Maja", 20);
  lb.update_up("Martin", 21);
  

  lb.print();





  struct in_addr addr;                     /* 32-bit IP number of host */
  int listen;                              /* Listen socket */
  int fd;                                  /* fd to Erlang node */
  ErlConnect conn;                         /* Connection data */

  int loop = 1;                            /* Loop flag */
  int got;                                 /* Result of receive */
  unsigned char buf[BUFSIZE];              /* Buffer for incoming message */
  ErlMessage emsg;                         /* Incoming message */

  int port = 3456;// atoi(argv[1]);
  char erl_secret[]    = "secretcookie";
  char erl_node[]      = "cnode";
  char erl_localname[] = "msk-dev";
  char erl_longname[]  = "cnode@msk-dev.local";
   
 
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
    printf("push back %d \n", i);
    if(event_arrays.empty()) event_arrays.push_back(new event_array(i));
    else event_arrays.push_back(new event_array(i, event_arrays.back()));
  }
  std::reverse(event_arrays.begin(), event_arrays.end());

  erl_init(NULL, 0);
    
  addr.s_addr = inet_addr("127.0.0.1");
 
  if (erl_connect_xinit(erl_localname, erl_node, erl_longname, &addr, erl_secret, 0) == -1) erl_err_quit("erl_connect_xinit failed");
  if ((listen = my_listen(port)) <= 0) erl_err_quit("my_listen failed ( likely unable to connec  t to socket )");
  if (erl_publish(port) == -1) erl_err_quit("erl_publish failed");
 
  while(true){
    if ((fd = erl_accept(listen, &conn)) == ERL_ERROR) erl_err_quit("erl_accept failed");
   
    while (loop) {
      got = erl_receive_msg(fd, buf, BUFSIZE, &emsg);
      if (got == ERL_TICK) { /* ignore */ }
      else if (got == ERL_ERROR) {
        loop = 0;
      } else {
      
        if (emsg.type == ERL_REG_SEND) {
          if(IS_CALL_HANDLE_EVENT(emsg.msg)){
            ETERM* tuplep, *event_list;

            tuplep     = erl_element(3, emsg.msg);
            event_list = erl_element(2, tuplep);
          
            std::vector< std::string> counters;

            while(!ERL_IS_NIL(event_list)){
              ETERM* head = ERL_CONS_HEAD(event_list);
              unsigned char*  counter = ERL_BIN_PTR(head);
              counters.push_back(std::string(counter, counter + strlen((char*)counter)));
              std::sort(counters.begin(), counters.end());
              event_list =  ERL_CONS_TAIL(event_list);
              //            delete counter;
            }
          
            event_arrays.front()->event(counters);
          
            erl_free_compound(tuplep); 
          }
          else if (IS_CALL_GET_COUNTER(emsg.msg)) {
            ETERM* resp;
            ETERM *fromp;
            fromp = erl_element(2, emsg.msg);

            event_arrays.front()->update(time(0));

            ETERM* tuplep, *counter_id;
            tuplep     = erl_element(3, emsg.msg);
            counter_id = erl_element(2, tuplep);
          
            unsigned char* counter = ERL_BIN_PTR(counter_id);
            std::string id_str(counter, counter + strlen((char*)counter));
            
            ETERM* list = erl_mk_empty_list();
            for(auto& events : event_arrays) {
            
              ETERM* tuple[2];
              tuple[0] = erl_mk_uint(events->timespan);
              tuple[1] = erl_mk_ulonglong(events->counters[id_str]);
              list = erl_cons(erl_mk_tuple(tuple, 2), list);
            }

            int c = 0;
            resp = list;
            erl_send(fd, fromp, resp);
            erl_free_term(fromp); 
            erl_free_term(resp); 
          }
          erl_free_term(emsg.to); 
          erl_free_term(emsg.from); 
          erl_free_term(emsg.msg);
        
        }
      }
    } /* while */
  }
}
  
bool is_call_funcname(char func_name[], ETERM* call_term) {
  ETERM  *tuplep = erl_element(3, call_term);
  ETERM  *fnp    = erl_element(1, tuplep   );
  
  // printf("testing : %s \n", ERL_ATOM_PTR(fnp));
  bool res = (strncmp(ERL_ATOM_PTR(fnp), func_name, strlen(func_name)) == 0);
  
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


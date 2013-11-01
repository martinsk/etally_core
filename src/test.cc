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
#include "event_scanner.hh"
#include "event_array.hh"
#include "action_container.hh"


#define BUFSIZE 1024

bool is_call_funcname(char func_name[], ETERM* message);
#define IS_CALL_HANDLE_EVENT(X)  (is_call_funcname("handle_event", X))
#define IS_CALL_GET_COUNTER(X)   (is_call_funcname("get_counter", X))


int my_listen(int port);

int main(int argc, char **argv) {
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
   
  erl_init(NULL, 0);

  addr.s_addr = inet_addr("127.0.0.1");
  if (erl_connect_xinit(erl_localname, erl_node, erl_longname, &addr, erl_secret, 0) == -1)
    erl_err_quit("erl_connect_xinit failed");

  /* Make a listen socket */
  if ((listen = my_listen(port)) <= 0)
    erl_err_quit("my_listen failed ( likely unable to connect to socket )");

  if (erl_publish(port) == -1)
    erl_err_quit("erl_publish failed");

  if ((fd = erl_accept(listen, &conn)) == ERL_ERROR)
    erl_err_quit("erl_accept failed");
  fprintf(stderr, "Connected to %s\n\r", conn.nodename);

  action_container actions;

  event_array events4(60*60);
  event_array events2(3, &events4);
  event_array events1(1, &events2);


  for(int i = 0; i != 100000000; i++)
    events1.event(1);


  while (loop) {

    got = erl_receive_msg(fd, buf, BUFSIZE, &emsg);

    if (got == ERL_TICK) {
      /* ignore */
    } else if (got == ERL_ERROR) {
      loop = 0;
    } else {
      
      if (emsg.type == ERL_REG_SEND) {
        // printf("message to handle\n");
        
        if(IS_CALL_HANDLE_EVENT(emsg.msg)){
          ETERM* tuplep, *event_list;
          tuplep = erl_element(3, emsg.msg);
          event_list = erl_element(2, tuplep);
          std::vector< std::string> counters;
          while(!ERL_IS_NIL(event_list)){
            unsigned char*  counter = ERL_BIN_PTR(ERL_CONS_HEAD(event_list));
            printf("%s \n", counter);
            counters.push_back(std::string(counter, counter + strlen((char*)counter)));
            event_list = ERL_CONS_TAIL(event_list);
          }
          events1.event(1);
          // Std::cout << "events4 " << std::endl;
          // events4.print();
          // char format_str[] = "{cnode, ok}";
          // resp = erl_format(format_str);
          // erl_send(fd, fromp, resp);
          // erl_free_term(fromp); 
          // erl_free_term(resp); 
        }
        else if (IS_CALL_GET_COUNTER(emsg.msg)) {
          ETERM* resp;
          ETERM *fromp;
          fromp = erl_element(2, emsg.msg);
          // char* counter_identifier;
          //int c = do_call_handle_get_counter(counter_identifier);
          int c = 0;
          char format_str[] = "{cnode, ok}";

          events1.update(time(0));
          std::cout << "events1 " << std::endl;
          
          for(auto& x: events1.counters) {
            std::cout << x.first << ": " << x.second << std::endl;
          }
          std::cout << "events2 " << std::endl;
          // events1.print();
          for(auto& x: events2.counters) {
            std::cout << x.first << ": " << x.second << std::endl;
          }
           std::cout << "events4 " << std::endl;
          // events2.print();
          for(auto& x: events4.counters) {
            std::cout << x.first << ": " << x.second << std::endl;
          }

          resp = erl_format(format_str, c);
          erl_send(fd, fromp, resp);
          erl_free_term(fromp); 
          erl_free_term(resp); 
        }

        // if (strncmp(ERL_ATOM_PTR(fnp), "handle_event", 9) == 0) {
        //   ETERM *fromp, *tuplep, *fnp, *argp, *resp;
        //   int res;
        //   fromp = erl_element(2, emsg.msg);
        //   tuplep = erl_element(3, emsg.msg);
        //   fnp = erl_element(1, tuplep);
        //   argp = erl_element(2, tuplep);
        //   events.event(ERL_INT_VALUE(argp));
        //   resp = erl_format("{cnode, ok}");
        //   erl_send(fd, fromp, resp);
        //   erl_free_term(fromp); erl_free_term(tuplep);
        //   erl_free_term(fnp); erl_free_term(argp);
        // } else if (strncmp(ERL_ATOM_PTR(fnp), "get_counter", 6) == 0) {
        //   ETERM *fromp, *tuplep, *fnp, *argp, *resp;
        //   int res;
        //   fromp = erl_element(2, emsg.msg);
        //   tuplep = erl_element(3, emsg.msg);
        //   fnp = erl_element(1, tuplep);
        //   argp = erl_element(2, tuplep);
        //   //res = bar(ERL_INT_VALUE(argp));
        //   resp = erl_format("{undefined}");
        //   erl_send(fd, fromp, resp);
        //   erl_free_term(resp);
        //   erl_free_term(fromp); erl_free_term(tuplep);
        //   erl_free_term(fnp); erl_free_term(argp);
        // }           
        
        erl_free_term(emsg.from); 
        erl_free_term(emsg.msg);
      }
    }
  } /* while */
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


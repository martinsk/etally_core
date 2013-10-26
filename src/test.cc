#include <cstdio>
#include <cstdlib>
#include <time.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "erl_interface.h"
#include "ei.h"

#include <iostream>
#include <queue>  
#include <string>


#include "types.hh"
#include "event_scanner.hh"
#include "event_array.hh"
#include "action_container.hh"



#define BUFSIZE 1000


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

  listen(listen_fd, 5);
  return listen_fd;
}


int main(int argc, char **argv) {
  int port;                                /* Listen port number */
  int listen;                              /* Listen socket */
  int fd;                                  /* fd to Erlang node */
  ErlConnect conn;                         /* Connection data */

  int loop = 1;                            /* Loop flag */
  int got;                                 /* Result of receive */
  unsigned char buf[BUFSIZE];              /* Buffer for incoming message */
  ErlMessage emsg;                         /* Incoming message */

  ETERM *fromp, *tuplep, *fnp, *argp, *resp;
  int res;

  port = atoi(argv[1]);

  erl_init(NULL, 0);

  if (erl_connect_init(1, "secretcookie", 0) == -1)
    erl_err_quit("erl_connect_init");

  /* Make a listen socket */
  if ((listen = my_listen(port)) <= 0)
    erl_err_quit("my_listen");

  if (erl_publish(port) == -1)
    erl_err_quit("erl_publish");

  if ((fd = erl_accept(listen, &conn)) == ERL_ERROR)
    erl_err_quit("erl_accept");
  fprintf(stderr, "Connected to %s\n\r", conn.nodename);



  action_container actions;
  std::vector<counter_idx_t> counter_idxs;
  counter_idxs.push_back(1);
  counter_idxs.push_back(1);
  actions.add_action(1, counter_idxs);

  event_scanner scan3sec(3, 0);

  std::vector<event_scanner> scanners;
  scanners.push_back(scan3sec);

  event_array events(4, actions, scanners);

  while (loop) {

    got = erl_receive_msg(fd, buf, BUFSIZE, &emsg);
    if (got == ERL_TICK) {
      /* ignore */
    } else if (got == ERL_ERROR) {
      loop = 0;
    } else {
      
      if (emsg.type == ERL_REG_SEND) {
        fromp = erl_element(2, emsg.msg);
        tuplep = erl_element(3, emsg.msg);
        fnp = erl_element(1, tuplep);
        argp = erl_element(2, tuplep);
        
        if (strncmp(ERL_ATOM_PTR(fnp), "increment", 9) == 0) {
          events.event(ERL_INT_VALUE(argp));
          resp = erl_format("{cnode}");
          erl_send(fd, fromp, resp);
        } else if (strncmp(ERL_ATOM_PTR(fnp), "get_counter", 6) == 0) {
          //res = bar(ERL_INT_VALUE(argp));
          resp = erl_format("{undefined}");
          erl_send(fd, fromp, resp);
          erl_free_term(resp);
        }
        
        erl_free_term(emsg.from); erl_free_term(emsg.msg);
        erl_free_term(fromp); erl_free_term(tuplep);
        erl_free_term(fnp); erl_free_term(argp);
      }
    }
  } /* while */
}

  


//   action_container actions;
//   vector<counter_idx_t> counter_idxs;
//   counter_idxs.push_back(1);
//   counter_idxs.push_back(1);
//   actions.add_action(1, counter_idxs);

//   event_scanner scan3sec(3, 0);

//   vector<event_scanner> scanners;
//   scanners.push_back(scan3sec);

//   event_array events(4, actions, scanners);
  
//   time_t before = time(0);
//   time_t after; 

//   long long c = 0;
//   do {
//     events.event(0);
//     //    sleep(1);
//     c++;
//     after = time(0);
//     //    events.print();  

//   } while(after - before < 15);
  
//   std::time_t t = std::time(0);  // t is an integer type
//   std::cout << c << " events passed\n";
  
//   return 0;
// }

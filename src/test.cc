#include <cstdio>
#include <cstdlib>
#include <time.h>
#include <stdint.h>

#include <iostream>
#include <queue>  
#include <string>


#include "types.hh"
#include "event_scanner.hh"
#include "event_array.hh"
#include "action_container.hh"

using namespace std;


int main(int argc, char** argv) {
  int size = atoi(argv[1]);
  printf(" -- STARTING --\n");


  action_container actions;
  vector<counter_idx_t> counter_idxs;
  counter_idxs.push_back(1);
  counter_idxs.push_back(1);
  actions.add_action(1, counter_idxs);

  event_scanner scan3sec(3, 0);

  vector<event_scanner> scanners;
  scanners.push_back(scan3sec);

  event_array events(4, actions, scanners);
  
  time_t before = time(0);
  time_t after; 

  long long c = 0;
  do {
    events.event(0);
    //    sleep(1);
    c++;
    after = time(0);
    //    events.print();  

  } while(after - before < 15);
  
  std::time_t t = std::time(0);  // t is an integer type
  std::cout << c << " events passed\n";
  
  return 0;
}

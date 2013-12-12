

#include <cassert>

#include <iostream>
#include <chrono>

#include "types.hh"
#include "tally.hh"

#include "circular_queue.hh"
#include "event_array.hh"
#include "leaderboard.hh"
#include "idx_assigner.hh"

#define assert_equal(x, y) \
  assert((x == y)  || !(std::cerr << "False: " << x << " != " << y << std::endl))
#define assert_not_equal(x, y) \
  assert((x != y) || !(std::cerr << "False: " << x << " == " << y << std::endl))
#define assert_false(x) assert_equal(x, false)

void test_queue1() {
  circular_queue<int> queue;
  for(int i = 0; i != 10; i++) {
    queue.enqueue(i);
    assert_equal(queue.back(), i);    
  }

  for(int i = 0; i != 10; i++){
    assert_false(queue.empty());
    assert_equal(queue.front(), i);
    queue.dequeue();
  }
}

void test_queue2() {
  circular_queue<int> queue;
  for(int i = 0; i != 100; i++)
    queue.enqueue(i);

  for(int i = 0; i != 50; i++){
    assert_false(queue.empty());
    assert_equal(queue.front(),i);
    queue.dequeue();
  }

  for(int i = 0; i != 100; i++)
    queue.enqueue(i);

  for(int i = 50; i != 100; i++){
    assert_false(queue.empty());
    assert_equal(queue.front(), i);
    queue.dequeue();
  }

  for(int i = 0; i != 100; i++){
    assert_false(queue.empty());
    assert_equal(queue.front(),i);
    queue.dequeue();
  }

}

void test_suite_queue() {
  test_queue1();
  test_queue2();
}

void test_event_array1() {  
  event_array ea(10);
  std::vector<counter_idx_t> e1 = {1,2,3};
  std::vector<counter_idx_t> e2 = {1,2,4};
  assert_equal(ea.get_counter(1), 0);
  assert_equal(ea.get_counter(2), 0);
  assert_equal(ea.get_counter(3), 0);
  assert_equal(ea.get_counter(4), 0);
  assert_equal(ea.length(), 0);

  ea.event(e1, 1000);
  
  assert_equal(ea.get_counter(1), 1);
  assert_equal(ea.get_counter(2), 1);
  assert_equal(ea.get_counter(3), 1);
  assert_equal(ea.get_counter(4), 0);
  assert_equal(ea.length(), 1);

  ea.update(1000);

  ea.event(e1, 1000);
  ea.event(e2, 1001);

  assert_equal(ea.get_counter(1), 3);
  assert_equal(ea.get_counter(2), 3);
  assert_equal(ea.get_counter(3), 2);
  assert_equal(ea.get_counter(4), 1);

  assert_equal(ea.length(), 3);

  ea.update(1007);

  assert_equal(ea.get_counter(1), 3);
  assert_equal(ea.get_counter(2), 3);
  assert_equal(ea.get_counter(3), 2);
  assert_equal(ea.get_counter(4), 1);
  assert_equal(ea.length(), 3);

  ea.update(1011);

  assert_equal(ea.get_counter(1), 1);
  assert_equal(ea.get_counter(2), 1);
  assert_equal(ea.get_counter(3), 0);
  assert_equal(ea.get_counter(4), 1);
  assert_equal(ea.length(), 1);
    
}

void test_event_array2() {  
  event_array ea_tail(20);
  event_array ea_head(10, &ea_tail);
  std::vector<counter_idx_t> e1 = {1,2,3};    

  ea_head.event(e1, 1000);
  
  assert_equal(ea_head.get_counter(1), 1);
  assert_equal(ea_head.get_counter(2), 1);
  assert_equal(ea_head.get_counter(3), 1);
  assert_equal(ea_head.length(), 1);

  assert_equal(ea_tail.get_counter(1), 1);
  assert_equal(ea_tail.get_counter(2), 1);
  assert_equal(ea_tail.get_counter(3), 1);
  assert_equal(ea_tail.length(), 0);
    
  ea_head.update(1011);

  assert_equal(ea_head.get_counter(1), 0);
  assert_equal(ea_head.get_counter(2), 0);
  assert_equal(ea_head.get_counter(3), 0);
  assert_equal(ea_head.length(), 0);

  assert_equal(ea_tail.get_counter(1), 1);
  assert_equal(ea_tail.get_counter(2), 1);
  assert_equal(ea_tail.get_counter(3), 1);
  assert_equal(ea_tail.length(), 1);

  ea_head.update(1021);

  assert_equal(ea_head.get_counter(1), 0);
  assert_equal(ea_head.get_counter(2), 0);
  assert_equal(ea_head.get_counter(3), 0);
  assert_equal(ea_head.length(), 0);

  assert_equal(ea_tail.get_counter(1), 0);
  assert_equal(ea_tail.get_counter(2), 0);
  assert_equal(ea_tail.get_counter(3), 0);
  assert_equal(ea_tail.length(), 0);

}

void test_event_array_sort() {  
  event_array ea(10);
  std::vector<counter_idx_t> e1 = {1,2,3};    

  ea.event(e1, 1010);
  ea.event(e1, 1000);
  ea.event(e1, 1010);
  ea.event(e1, 1000);

  assert_equal(ea.length(), 4);
  ea.update(1011);
  assert_equal(ea.length(), 4);
  std::cerr << ea << std::endl;
  ea.sort();
  std::cerr << ea << std::endl;
  
  assert_equal(ea.length(), 4);
    ea.update(1011);
  assert_equal(ea.length(), 2);
}




void test_suite_event_array() {
  test_event_array1();
  test_event_array2();
  test_event_array_sort();
}

// void test_btree1() {
//   btree<int> tree;
//   tree.insert(1);
//   assert_equal(tree.contains(1), true);
//   assert_equal(tree.contains(2), false);
//   assert_equal(tree.size(), 1);
// }

// void test_btree2() {
//   btree<int> tree;
//   tree.insert(1);
//   tree.insert(2);
//   tree.insert(3);
//   assert_equal(tree.contains(1), true);
//   assert_equal(tree.contains(2), true);
//   assert_equal(tree.contains(3), true);
//   assert_equal(tree.contains(4), false);
//   assert_equal(tree.size(), 3);
//   std::cerr << tree << std::endl;
// }

// void test_btree3() {
//   btree<int> tree;
//   tree.insert(1);
//   tree.insert(2);
//   tree.insert(3);
//   assert_equal(tree.contains(1), true);
//   assert_equal(tree.contains(2), true);
//   assert_equal(tree.contains(3), true);
//   assert_equal(tree.size(), 3);
//   tree.remove(3);
//   assert_equal(tree.contains(1), true);
//   assert_equal(tree.contains(2), true);
//   assert_equal(tree.contains(3), false);
//   assert_equal(tree.size(), 2);
//   tree.remove(2);
//   tree.remove(1);
//   assert_equal(tree.contains(1), false);
//   assert_equal(tree.contains(2), false);
//   assert_equal(tree.contains(3), false);
//   assert_equal(tree.size(), 0);
// }

// void test_btree4() {
//   btree<std::pair<int, long> > tree;
//   std::pair<int, long> elm = std::make_pair<int, long>(1, 1);
//   tree.insert(elm);
//   assert_equal(tree.contains(elm), true);
//   tree.remove(elm);
//   assert_equal(tree.size(), 0);
// }


// void test_btree5() {
//   btree<int > tree;
//   tree.insert(10);
//   tree.insert(2);
//   tree.insert(5);
//   tree.insert(4);
//   tree.insert(3);
//   std::cerr << tree << std::endl;
//   tree.tour();
// }

// void test_btree6() {
//   btree<int > tree;
//   for(int i = 0; i != 10; i++)
//     tree.insert(i);

//   std::cerr << std::endl;

//   tree.tour();
//   auto r = tree.range(2,5);
//   std::cerr << std::endl;
//   std::cerr << std::endl;
 
//   for(int i : r)
//     std::cerr << i << " " << std::endl;
// }




// void test_suite_btree() {
//   test_btree1();
//   test_btree2();
//   test_btree3();
//   test_btree4();
//   test_btree5();
//   test_btree6();
// }


std::chrono::time_point<std::chrono::system_clock> time_now() {
  return std::chrono::system_clock::now();
}

void test_suite_set() {
  
  std::vector<int> lengths = {1000,10000,100000,1000000};
  for (int length : lengths){
    std::cerr << "lenght " << length << std::endl;
    std::set<std::pair<int, int> > s;  
    auto before = time_now();  
  for(int i =0; i != length; i++)
    s.insert(std::make_pair(i,i));
  
  std::cerr << std::endl << (std::chrono::duration<double>(time_now() - before).count()) << std::endl;
  
  before = time_now();
  
  long sum = 0;

  auto i = s.begin();
  std::advance(i, length-100);
  for(; i != s.end(); i++)
    sum += i->first;
 
  std::cerr << sum << "  " << (std::chrono::duration<double>(time_now() - before).count()) << std::endl;

  before = time_now();
  sum = 0;
  
  for(auto i = s.begin(); i != s.end(); i++)
    sum += i->first;
  
  std::cerr << sum << "  " << (std::chrono::duration<double>(time_now() - before).count()) << std::endl;
  
  }
  
}




bool test_suite() {
  std::cerr << "info: starting tests" << std::endl;
  test_suite_queue();
  test_suite_event_array();
  //  test_suite_btree();
  test_suite_set();

  std::cerr << "info: test suites passed" << std::endl;
  return true;
}


#ifndef _DYNAMIC_ROUND_ROBIN_ARRAY_HH
#define _DYNAMIC_ROUND_ROBIN_ARRAY_HH

template<typename Ty>
class dynamic_round_robin_array {
  Ty* data;
  int max_size;
  int size;
  int start;

public:
  
  dynamic_round_robin_array();
  ~dynamic_round_robin_array();
  
  void push_back(int value);   
  Ty top() const;
  void pop_front();
  int length() const;

  void print() const;
};


/************************************************************
 * IMPLEMENTATION
 ************************************************************/

template<typename Ty>
dynamic_round_robin_array<Ty>::dynamic_round_robin_array() {
  data = new Ty[1];
  size = 0;
  max_size = 1;
  start = 0;
}
  
template<typename Ty>
dynamic_round_robin_array<Ty>::~dynamic_round_robin_array() {
  delete[] data;
}

template<typename Ty>
void dynamic_round_robin_array<Ty>::push_back(int value) {
  if (size == max_size) {
    // double the array and copy
    Ty* old_data = data;
    max_size *= 2;
    data = new Ty[max_size];
    for(int i = 0; i != size; i++) {
      data[i] = old_data[(start + i)%size];
    }
    start = 0;
    delete[] old_data;
  }
  data[(start + size)%max_size] = value;
  size ++;
}


template<typename Ty>
Ty dynamic_round_robin_array<Ty>::top() const {
  return data[start];
}
  
template<typename Ty>
void dynamic_round_robin_array<Ty>::pop_front() {
  start++ ;
  start %= max_size;
  size --;
}
  
template<typename Ty>
int dynamic_round_robin_array<Ty>::length() const {
  return size;
}

template<typename Ty>
void dynamic_round_robin_array<Ty>::print() const {
  for(int i = 0; i != size; i++) {
    printf("%d ", data[(start + i)%max_size]);
  }
  printf("\n");
}

#endif // _DYNAMIC_ROUND_ROBIN_ARRAY_HH

#ifndef CIRCULAR_QUEUE_HH
#define CIRCULAR_QUEUE_HH

template<typename Ty>
class circular_queue {
private:
  
  Ty* data;
  unsigned long back_p;
  unsigned long size;
  unsigned long container_size;

public:
  circular_queue();
  ~circular_queue();

  Ty front();
  const Ty& front() const;
  Ty back();
  const Ty& back() const;
  void enqueue(const Ty& elm);
  void dequeue();

  unsigned int long length() const {return size; };
  bool empty() {return (size == 0); }

private:

  bool is_full() {return size == container_size;}
  void double_container();


public:
  friend std::ostream& operator<<(std::ostream& out,  const circular_queue<Ty>& queue) ;

};




template<typename Ty>
circular_queue<Ty>::circular_queue()
  : data(new Ty[1]), 
    back_p(0),
    size(0),
    container_size(1)
{}

template<typename Ty>
circular_queue<Ty>::~circular_queue() {
  delete[] data;
}

template<typename Ty>
Ty circular_queue<Ty>::front() {
  return data[(back_p - size)%container_size];
}

template<typename Ty>
const Ty& circular_queue<Ty>::front() const {
  return data[(back_p - size)%container_size];
}

template<typename Ty>
Ty circular_queue<Ty>::back() {
  return data[ back_p ];
}

template<typename Ty>
const Ty& circular_queue<Ty>::back() const {
  return data[ back_p ];
}

template<typename Ty>
void circular_queue<Ty>::enqueue(const Ty& elm)  {
  if(is_full()) double_container();

  data[back_p] = elm;
  size++;
  back_p ++;
  back_p %= container_size;
}


template<typename Ty>
void circular_queue<Ty>::dequeue()  {
  size--;
}


template<typename Ty>
void circular_queue<Ty>::double_container()  {
  Ty* old = data;

  container_size *= 2;
  data = new Ty[container_size];
  for(int i = 0; i != size; i++){
    data[i] = old[(back_p - i) % size];
  }

  back_p = size;
  delete[] old;
}










#endif // CIRCULAR_QUEUE_HH

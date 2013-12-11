#ifndef COUNTER_ARRAY_HH
#define COUNTER_ARRAY_HH
#include "types.hh"

template<typename Ty>
class counter_array{
private:
  std::vector<Ty> data;

public:
  Ty& operator[](idx_t idx) {
    if( data.size() <= idx) {
      data.resize(idx+1);
    }
    return data[idx];
  }

  const Ty operator[](idx_t idx) const {
    return data.at(idx);    
  }

  size_t length() const {
    return data.size();
  }
};

#endif

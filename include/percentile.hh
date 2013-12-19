#ifndef PERCENTILE_HH
#define PERCENTILE_HH

#include <cmath>
#include <cstdlib>
#include <climits>

#include <vector>

#include "types.hh"


class percentile {
private:
  std::vector<uint32_t> vec;
  const unsigned int arr_len;
  const double base = 1.0f;
  
public:
  percentile(unsigned int arr_len);
  
private:
  inline unsigned int value_to_idx(unsigned int value) const;
  inline unsigned int idx_to_value(unsigned int value) const;

public:
  unsigned int find_percentile(float percent) const;

  inline void insert(unsigned int value);
  inline void remove(unsigned int value);

};


void percentile::insert(unsigned int value) {
  vec[value_to_idx(value)] ++;
}

void percentile::remove(unsigned int value) {
  vec[value_to_idx(value)] --;
}



unsigned int percentile::value_to_idx(unsigned int value) const {
  if(value < 100) return value;
  else if (value < 100*std::pow(base, arr_len - 100) ) {
    long double log_n = std::log(value/100.0);
    long double log_101 = std::log(base);    
    return static_cast<unsigned int >(std::ceil( (log_n/log_101))) + 100 ;
  }else return arr_len;
}


unsigned int percentile::idx_to_value(unsigned int idx) const {
  if (idx < 100) return idx;
  else if (idx == arr_len) return  UINT_MAX;
  else {
    return std::pow(base, idx-100)*100.0; 
  }
}
 

#endif// PERCENTILE_HH

#include "percentile.hh"



percentile::percentile(unsigned int arr_len) 
  : vec(std::vector<uint32_t>(arr_len)),
    arr_len(arr_len) {}


unsigned int percentile::find_percentile(float percent) const {
  int i = 0;

  int total = 0;
  for(auto v : vec) total += v;
  
  long sum = 0;
  while(i != vec.size() && sum < percent*total) {
    sum  += vec[i];
    i++;
  }
  return idx_to_value(i);
}





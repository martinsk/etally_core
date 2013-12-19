#ifndef _VECTOR_HASH_HH
#define _VECTOR_HASH_HH

template<typename Ty>
class vector_hash{
public:
  std::size_t operator()(std::vector<Ty> const& vec) const {
    std::size_t ret = 0;
    for(auto& i : vec) {
      ret ^= std::hash<Ty>()(i);
    }
    return ret;
  }
};

#endif

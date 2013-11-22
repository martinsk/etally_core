#include "idx_assigner.hh"

bool idx_assigner::has_idx(const std::string& token) const {
  return (token_lookup.count(token) != 0);
}

unsigned int idx_assigner::get_idx(std::string& token){
  if(has_idx(Token))
    return idx_lookup[token];
  else {
    unsigned int idx = assign_idx(token);
    idx_lookup[token] = idx;
    token_lookup[idx] = token;
    return idx_lookup[token];
  }
}

std::string idx_assigner::get_token(unsigned int idx){
  return token_lookup[token];  
}



void idx_assigner::release_idx(unsigned int idx){
  unused.push(idx);
}


bool idx_assigner::assign_idx(const std::string& token) {
  if(unused.empty()) {
    unsigned ret = max_idx;
    max_idx++;
    return ret;
  }
  else {
    unsigned ret = unused.top();
    unused.pop();
    return ret;
  }
}




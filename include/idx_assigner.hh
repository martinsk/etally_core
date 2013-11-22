#ifndef IDX_ASSIGNER_HH
#define IDX_ASSIGNER_HH

#include <stack>
#include <unordered_map>

class idx_assigner{
private:
  unsigned int max_idx = 0;
  std::stack<unsigned int> unused;
  std::unordered_map<std::string, idx> idx_lookup;
  std::unordered_map<idx, std::string> token_lookup;
  
  
public:
  bool has_idx(const std::string& token) const;
  unsigned int get_idx(std::string& token);
  std::string get_token(unsigned int idx);
  void release_idx(unsigned int idx);

private:
  unsigned int assign_idx(const std::string& token);
};

#endif //IDX_ASSIGNER_HH



// ./tally_srv "127.0.0.1" "livestats01" "tally@livestats01.east-cloud01.tigertext.me" "QWJCDLYIBTECKABSLLND"

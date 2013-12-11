#ifndef ERL_UTILS_HH
#define ERL_UTILS_HH

template<typename Ty> 
std::vector<typename Ty::return_type> construct_vector_from_term(ETERM *list, Ty constructor) {
  std::vector<typename Ty::return_type>  vec;
  while( !ERL_IS_NIL(list) ) {
    ETERM* head = ERL_CONS_HEAD(list);
    vec.push_back(constructor.construct(head));
    list = ERL_CONS_TAIL(list);
  }
  return vec;
}

template<typename Ty> 
void parse_term_list(ETERM *list, Ty constructor) {
  while( !ERL_IS_NIL(list) ) {
    ETERM* head = ERL_CONS_HEAD(list);
    constructor.parse(head);
    list = ERL_CONS_TAIL(list);
  }
}

#endif //ERL_UTILS_HH

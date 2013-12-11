#ifndef COUNT_HANDLERS_HH
#define COUNT_HANDLERS_HH



void count_handle_event_timestamp  (ErlMessage& emsg, tally& tally_srv);
void count_handle_event            (ErlMessage& emsg, tally& tally_srv);
void count_handle_get_counter      (ErlMessage& emsg, tally& tally_srv, int fd);
void count_handle_get_leaderboard  (ErlMessage& emsg, tally& tally_srv, int fd);
void count_handle_list_leaderboards(ErlMessage& emsg, tally& tally_srv, int fd);


void count_bind_leaderboards(ETERM* binding_list, tally& tally_srv);




#endif // COUNT_HANDLERS_HH

#ifndef METRIC_HANDLERS_HH
#define METRIC_HANDLERS_HH



void metric_handle_event_timestamp  (ErlMessage& emsg, tally& tally_srv);
void metric_handle_event            (ErlMessage& emsg, tally& tally_srv);
void metric_handle_get_counter      (ErlMessage& emsg, tally& tally_srv, int fd);
void metric_handle_get_leaderboard  (ErlMessage& emsg, tally& tally_srv, int fd);
void metric_handle_list_leaderboards(ErlMessage& emsg, tally& tally_srv, int fd);

void metric_bind_leaderboards(ETERM* binding_list, tally& tally_srv);




#endif // METRIC_HANDLERS_HH

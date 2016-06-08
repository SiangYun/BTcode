#ifndef bittorrent_tracker_flowlevel
#define bittorrent_tracker_flowlevel

#include "bittorrent_tracker.h"
#include "bittorrent_app_flowlevel.h"
#include "bittorrent_data.h"

class BitTorrentAppFlowlevel;
class PostmanTimer;

typedef struct tracker_list_entry {
	long id;
	BitTorrentAppFlowlevel *ptr;
};



class BitTorrentTrackerFlowlevel : public BitTorrentTracker 
{

public:

	// Constructor
	BitTorrentTrackerFlowlevel(long file_size, long chunk_size);
	
	long reg_peer(BitTorrentAppFlowlevel *p);	
	
	void del_peer(long id);

	vector<long> get_peer_set(int req_num_of_peers);
	
	void sendmsg(BitTorrentData data, long sender, long receiver);

	void close_con(long sender_id, long receiver);
	
	long return_rarest_chunk();


	
private:
	
	vector<tracker_list_entry*> peer_ids_;	
	
	long peer_counter;
	
	long return_index(long pid);

};


#endif

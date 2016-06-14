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
	BitTorrentTrackerFlowlevel(long file_size, long chunk_size, long highlight_size);
	
	long reg_peer(BitTorrentAppFlowlevel *p);	                  // �Ȧs�ϥΪ�
	 
	void del_peer(long id);                                       // �R���ϥΪ�

	vector<long> get_peer_set(int req_num_of_peers);              // �o��ϥΪ̶��X
	
	void sendmsg(BitTorrentData data, long sender, long receiver);// �ǰe�T��

	void close_con(long sender_id, long receiver);                // �����s�u
	
	long return_rarest_chunk();                                   // �^�ǵ}���϶�


	
private:
	
	vector<tracker_list_entry*> peer_ids_;	// �ϥΪ�ID�}�C
	
	long peer_counter;                      // �p��ϥΪ�
	
	long return_index(long pid);            // �^�ǯ���

};


#endif

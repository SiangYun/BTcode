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
	
	long reg_peer(BitTorrentAppFlowlevel *p);	                  // 暫存使用者
	 
	void del_peer(long id);                                       // 刪除使用者

	vector<long> get_peer_set(int req_num_of_peers);              // 得到使用者集合
	
	void sendmsg(BitTorrentData data, long sender, long receiver);// 傳送訊息

	void close_con(long sender_id, long receiver);                // 結束連線
	
	long return_rarest_chunk();                                   // 回傳稀有區塊


	
private:
	
	vector<tracker_list_entry*> peer_ids_;	// 使用者ID陣列
	
	long peer_counter;                      // 計算使用者
	
	long return_index(long pid);            // 回傳索引

};


#endif

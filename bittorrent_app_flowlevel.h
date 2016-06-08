#ifndef bittorrent_app_flowlevel_h
#define bittorrent_app_flowlevel_h

#include <vector>
#include "string.h"
#include "node.h"
#include "rng.h"
#include "bittorrent_app.h"
#include "bittorrent_tracker_flowlevel.h"


class BitTorrentTrackerFlowlevel;
class BitTorrentAppFlowlevel;
class UploadTimer;

class UploadQueueEntry : public TclObject {

public:
	UploadQueueEntry(BitTorrentData data_, double delay_, long sender_cid_, long receiver_cid_,peer_list_entry *sender_)
	: data(data_), 
	delay(delay_), 
	sender_cid(sender_cid_),
	receiver_cid(receiver_cid_),
	sender(sender_) {};
	
	BitTorrentData data;
	
	double delay;
	
	long sender_cid;
	long receiver_cid;
	
	peer_list_entry * sender;
};



class BitTorrentAppFlowlevel : public BitTorrentApp
{
friend class UploadTimer;
friend class BitTorrentTrackerFlowlevel;

public:
	BitTorrentAppFlowlevel(int seed, long capacity, BitTorrentTrackerFlowlevel *global=NULL);

	~BitTorrentAppFlowlevel();

	bool get_ids_from_tracker();
	void check_connections();
	void close_con(long pid);
	void timeout();


private:	
	void start();
	void stop();

	bool not_in_peer_set(int peer_id);
	
	peer_list_entry* make_new_peer_list_entry(long pid);

	void send(BitTorrentData tmp, int cID);
			
	long num_of_cons();
		
	void check_choking();

	void handle_request(BitTorrentData bittorrentData, int connectionID);

	
	// Pointer to Tracker
	BitTorrentTrackerFlowlevel *tracker_;
		
	vector<UploadQueueEntry> UploadQueue;
		
	double last_upload_time;
	double upload_quota;
};



class UploadTimer:public TimerHandler
{
protected:
	BitTorrentAppFlowlevel *app_;

public:
	UploadTimer(BitTorrentAppFlowlevel* app) : TimerHandler(), app_(app) {}
	inline virtual void expire(Event *);

};

#endif //ns_BitTorrentApp_h

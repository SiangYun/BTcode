#ifndef bittorrent_app_h
#define bittorrent_app_h

#include <vector>
#include "string.h"
#include "node.h"
#include "rng.h"
#include "bittorrent_params.h"
#include "bittorrent_tracker.h"
#include "bittorrent_data.h"
#include "bittorrent_connection.h"

class BitTorrentTracker;
class BitTorrentConnection;
class BitTorrentDataBuf;
class ChokingTimer;
class TrackerRequestTimer;
class LeavingTimer;
class ConnectionTimeout;
class KeepAliveTimer;


/////////////////////
// Request List Entry
/////////////////////
typedef struct request_list_entry {
	int chunk_id;
	long length;	// [Bytes]
	long rid;
};


//////////////////////////////
// Active Connection Parameter
//////////////////////////////
typedef struct act_con_paras {
	int id;
	double grad;
	double d;
};


//////////////////////////////
// Unchoke Candidate
//////////////////////////////
struct service_info{
	int id;
	double service_rate;
	int index;
};


/////////////////////
// Peer Set List Entry
/////////////////////
typedef struct peer_list_entry {
	int id;
	Node *node;
	int connected;		// -1: not connected, 0: one peer asks for connection, 1: connected
	int con_id;
	int *chunk_set;
	double x_up;		// my upload rate [Bytes/s]
	double x_down;		// my download rate [Bytes/s]
	long *uploaded_bytes;	// uploaded bytes between check_choking intervals
	long *downloaded_bytes;	// downloaded bytes between check_choking intervals
	
	// state info
	bool am_choking;
	bool peer_choking;
	bool am_interested;
	bool peer_interested;
	
	// info for uploads
	vector<request_list_entry *> peer_requests;
	long uploaded_requested_bytes;
	
	// info for downloads
	vector<request_list_entry *> my_requests;
	long downloaded_requested_bytes;
	
	int opt_unchoke_prob;
	int snubbed;
	
	long peer_upload_quota;
	bool greedy;
	
	ConnectionTimeout *connection_timeout_;	// last time when I heard from peer
	KeepAliveTimer *keep_alive_timer_;	// timer to send KEEP ALIVE msgs
	double last_unchoke;	// last time when I unchoked this peer
	double last_choke;	// last time when I choked this peer
	double last_msg_recv;	// last time when I received a msg from peer
	double last_msg_sent;	// last time when I sent a msg to the peer
	
	
	double last_remote_unchoke;
	
	int slow_start;		// indicates if choking algorithm is in slow start (number of smallest sub-pices) or not (-1)
	bool send_one_chunk;	// indicates if the new request from peer is for other chunk than old one
	
	bool queue_request;	// requests are queued at the application. only first request after an unchoke is put in tcp buffer
	
	long super_seeding_chunk_id;
};



class BitTorrentApp : public TclObject
{

friend class BitTorrentTracker;

friend class BitTorrentTrackerFlowlevel;

public:
	BitTorrentApp(int seed, long capacity, BitTorrentTracker *global = NULL, Node *here = NULL);
	
	~BitTorrentApp();
	
	// stop method
	virtual void stop();
	
	void reset();

	// decides to whom to upload (called by ChokingTimer)
	virtual void check_choking();	
	
	// receive method called by agent when a packet arrives
	void recv(int nbytes, int connectionID);
	
	// receive TCP FIN from remote agent
	void close_connection(int connectionID);
	
	// TCP Agent signals all data sent
	void upcall_send(int connectionID);

	// checks if connection is inactive for too long
	virtual void timeout();
	
	// sends KEEP ALIVE msg
	void send_keep_alive();
	
	void tracker_request();

	// gets ids from tracker and combines it with old peer set
	virtual bool get_ids_from_tracker();
	
	// decides to open new connections
	virtual void check_connections();
	
	// node on which application sits
	Node *node_;
	

protected:

// FUNCS
	// Tcl command interpreter
	virtual int command(int argc, const char*const* argv);
	
	virtual void delay_bind_init_all();
	virtual int delay_bind_dispatch(const char *varName, const char *localName, TclObject *tracer);
	
	// start method
	virtual void start();
		
	// Tracker communication:
	
	// checks if peer_id is in the peer set
	bool not_in_peer_set(int peer_id);
	
	// makes a new entry for the peer list with default values
	peer_list_entry* make_new_peer_list_entry(Node *new_node);
	
	void delete_peer_from_list(int cID);	
	
	
	// Peer communication:
	
	// build two way TCP connections between peers:
	int connect(Node *node_dst, BitTorrentData msg);
	int connect_step_2(BitTorrentApp *dst, int conID, Agent *dst_tcp_, BitTorrentConnection* remote_con);
	int connect_step_3(BitTorrentApp *dst, int conID, int dst_conID, Agent *dst_tcp_,  BitTorrentConnection* dst_con_);
	
	// send msg to peer
	virtual void send(BitTorrentData tmp, int cID);
	
	// returns true if peer is interested in peer at position peer_set_index in the peer set, false otherwise.
	bool check_interest(int peer_set_index);
		
	// after receiving an unchoke msg., peer requests rarest chunk
	void make_request(int cID, int no_of_requests);
	
	// computes which chunk to request
	int chunk_selection(int cID);
	
	// checks if chunk index is requested also from another peer
	bool downloading_from_other_peer(int chunk_index, int sender_index);
	
	// called when download of a chunk is completed
	void chunk_complete(int chunk_id, int cID);
	
	void choke_peer(int peer_set_index);
	
	void unchoke_peer(int peer_set_index);
	
	void compute_leaving_time();
	
	// reschedules the timeout when i heard something over this connection
	void resched_timeout(int index);
	
	// return ptr to BitTorrentConnection for a connection id
	BitTorrentConnection * conid2conptr (int cID);
	
	// set the chunk set of a peer randomly
	void set_rand_chunk_set();
	
	void set_rand_chunk_set_N(int N);
	
	void have_rarest_chunk();

	// returns the number of open connections
	virtual long num_of_cons();

	long num_of_interested_peers();


	void log_statistics();
	
	// Create Messages:
	BitTorrentData createKeepAliveMessage();
	BitTorrentData createHandshakeMessage();
	BitTorrentData createBitfieldMessage();
	BitTorrentData createHaveMessage(int chunk_id);
	BitTorrentData createInterestedMessage();
	BitTorrentData createNotInterestedMessage();
	BitTorrentData createChokeMessage();
	BitTorrentData createUnchokeMessage();
	BitTorrentData createRequestMessage(int chunk_id, long length, long begin, double unchoke_time);
	BitTorrentData createPieceMessage(int chunk_id, long length, long begin);
	BitTorrentData createCancelMessage(int chunk_id, long length, long begin);
	
	// handle the receive of a message:
	void handle_recv_msg(BitTorrentData bittorrentData, int connectionID);
	void handle_handshake(BitTorrentData bittorrentData, int connectionID);
	void handle_bitfield(BitTorrentData bittorrentData, int connectionID);
	virtual void handle_request(BitTorrentData bittorrentData, int connectionID);
	void handle_piece(BitTorrentData bittorrentData, int connectionID);
	
	
	void super_seeding_have(long peer_index, int connectionID);
	void super_seeding_choking();

	
// VARS
	// ID of peer
	long id;

	// indicates if application is started or not
	bool running;
	
	// indicates if application is done, i.e. downloaded the file and stopped and waiting to be deleted
	int done;
	
	// vector containing pointers to all TCP connection infos
	// (this is not integrated into the peer_list_entry bit linked by conID var)
	vector<BitTorrentConnection *> conList_;
	
	// vector containing pointers to peer set entries
	vector<peer_list_entry *> peer_set_;
	
	// Pointer to Tracker
	BitTorrentTracker *tracker_;
	
	// 0-1 vector if chunk is existing or not
	int *chunk_set;
	
	// seed guarantees good start up by sending modified BITFIELD. Seed sends to the first N_C peers bitfield with one chunk only 
	int seed_start_up;
	
	// vector which shows the number of downloaded bytes for each chunk
	long *download;		// [Bytes]
	
	// vector which shows the number of downloaded bytes for each chunk
	long *requested;	// [Bytes]
	
	// 0: if download incomplete, 1: if download complete
	bool seed;
	
	// 1: if in super seeding mode, 0: otherwise
	int super_seeding;
	
	// virtual chunk set for super-seeding
	int *super_seeding_chunk_set;
	
	// track the minimum number of sent out HAVEs for a chunk
	int super_seeding_min_count;
	
	// virtual chunk set for super-seeding
	int super_seeding_pending_chunks;
	
	// 1: if in end game mode, 0: otherwise
	bool end_game_mode;
	
	// upload capacity
	long C;		// [Bytes/s]
	
	// upload quota (capacity * time interval - send_bytes_in_this_interval)
	long upload_quota;
	
	// total download rate
	double x_down_total;
	
	// number of active connections (active = chunk data is transfered)
	int act_cons;
	
	// identifier for connection id
	int con_counter;
	
	// number of peers to unchoke
	int unchokes;
	
	// indicates wether this peer gets first interested msg (enables CheckChokingTimer)
	bool first_interest;
	
	// indicates wether this peer gets first request msg (enables DataSendTimer)
	bool first_request;
	
	// indicates wether i do an optimistic unchoke during this choking
	int opt_unchoke_counter;
	
	// id of the peer which is optimistically unchoked
	int opt_unchoke_pid;
	
	// time interval between control of choking/unchoking peers
	// BiTorrent:	10s [BitTorrent Specification, wiki.theory.org]
	// CP : 4s
	int choking_interval;
	
	// request_id is used to check if the received PIECE msg belongs to the right REQUEST
	long request_id;
	
	// TRUE, if peer has not requested a chunk. FALSE, otherwise
	bool first_chunk_selection;

	// number of peers the tracker should return 
	int num_from_tracker;
	
	// minimum number of peer to not do rerequesting
	int min_peers;
	
	// number of peers at which to stop initiating new connections
	int max_initiate;

	// maximum number of open connections
	int max_open_cons;
	
	// number of pipelined requests
	int pipelined_requests;
	
	// user behaviour when peer leaves the network
	int leave_option;
	
	// switch to choose unchoking algorithm
	int choking_algorithm;
	
	// propagation delay of first hop [ms]
	int delay;
	
// Random Generator
	RNG rand_;
	
	
//TIMERS
	ChokingTimer *choking_timer_;
	TrackerRequestTimer *tracker_request_timer_;
	LeavingTimer *leaving_timer_;
	
// Analysis
	// time at which peer enters the p2p network
	double start_time;
	
	// time at which peer completes the download of the first chunk
	double first_chunk_time;
	
	// time at which peer completes the download
	double download_finished;
	
	// time at which peer leaves the p2p network
	double stop_time;
	
	// counts the number of bytes send 
	long total_bytes_queued;
	
	// counts the number of bytes send 
	long total_bytes_queued_plus_header;
	
	// count the number of data bytes send
	long data_bytes_queued;
	
	// altruism = count the number of data bytes send without own interest in the other peer 
	long altruistic_bytes_sent;
	
	// counts the number of received total bytes
	long total_bytes_recv;
	
	// count the number of received data bytes
	long data_bytes_recv;
	
	// number of get_ids_from_tracker requests
	long tracker_requests;
	
	// name of trace file
	char p2ptrace_file[256];
	
	bool reported_max_time;
};



// Timer:

class ChokingTimer:public TimerHandler
{
protected:
	BitTorrentApp *app_;

public:
	ChokingTimer(BitTorrentApp* app) : TimerHandler(), app_(app) {}
	inline virtual void expire(Event *);

};


class TrackerRequestTimer:public TimerHandler
{
protected:
	BitTorrentApp *app_;

public:
	TrackerRequestTimer(BitTorrentApp* app) : TimerHandler(), app_(app) {}
	inline virtual void expire(Event *);

};



class LeavingTimer:public TimerHandler
{
protected:
	BitTorrentApp *app_;

public:
	LeavingTimer(BitTorrentApp* app) : TimerHandler(), app_(app) {}
	inline virtual void expire(Event *);

};



class ConnectionTimeout:public TimerHandler
{
protected:
	BitTorrentApp *app_;

public:
	ConnectionTimeout(BitTorrentApp* app) : TimerHandler(), app_(app) {}
	inline virtual void expire(Event *);

};



class KeepAliveTimer:public TimerHandler
{
protected:
	BitTorrentApp *app_;

public:
	KeepAliveTimer(BitTorrentApp* app) : TimerHandler(), app_(app) {}
	inline virtual void expire(Event *);

};

#endif //ns_BitTorrentApp_h

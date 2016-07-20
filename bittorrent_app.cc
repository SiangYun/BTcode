
#include "bittorrent_app.h"

#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

#define MAX(x, y) ((x)>(y) ? (x) : (y))
#define MIN(x, y) ((x)<(y) ? (x) : (y))

static class BitTorrentAppClass : public TclClass
{
public:
	BitTorrentAppClass() : TclClass("BitTorrentApp") {}
	TclObject* create(int argc, const char*const* argv)
	{
		if (argc != 8)
		{
			cout << "app_static-1" << endl;
			return NULL;
		}
		else
		{for(int i = 0; i < argc; i++)
			cout << "app argv: " << i << " " << argv[i] << endl;
			cout << "app_static-2" << endl;
			return (new BitTorrentApp(
				atoi(argv[4]),
				atol(argv[5]),
				(BitTorrentTracker *)TclObject::lookup(argv[6]),
				(Node *)TclObject::lookup(argv[7])
				));
		}
	}
} class_bittorrent_app;


bool operator<(const service_info& a, const service_info& b) {
	cout << "app_1" << endl;
	return a.service_rate < b.service_rate;
}


// to compare pointers of service info
class CompareServices {
public:
	bool operator()(const service_info *i1,const service_info *i2) const {
		cout << "app_2" << endl;
		return *i1<*i2;
	}
};

//==============================================================
//    CONSTRUCTOR
//==============================================================
BitTorrentApp::BitTorrentApp(int seed_init, long capacity, BitTorrentTracker *global, Node *here) {

static int peerNu1;
peerNu1++;
peerNu = peerNu1;
	cout << "app_CONSTRUCTOR3 : " << peerNu << endl;
	if (global != NULL) {
		cout << "app_4" << endl;
		tracker_ = global;
	}
	
	node_ = here;
	if (here != NULL) {
		cout << "app_5" << endl;
		node_->setBTApp(this);
		id = node_->address();
	}

	running = false;
	
	if (seed_init == 1)
	{
		cout << "app_6" << endl;
		seed = true;
	}
	else
	{
		cout << "app_7" << endl;
		seed = false;
	}
	end_game_mode = false;
	C = capacity;	

	super_seeding_pending_chunks = 0;
	super_seeding_min_count = 0;
	
	// init chunk set to 0
	if (tracker_ != NULL) {
		cout << "app_8" << endl;
		chunk_set = new int[tracker_->N_C];
		download = new long[tracker_->N_C];
		requested = new long[tracker_->N_C];
		highlight_download = new long[tracker_->N_C/9]; //
		
		if (seed)
		{
			cout << "app_9" << endl;
			super_seeding_chunk_set = new int[tracker_->N_C];
		}
		
	
		for (int i=0; i<tracker_->N_C; i++) {
			if (seed) {
				cout << "app_10" << endl;
				chunk_set[i] = 1;
				super_seeding_chunk_set[i] = 0;

			} else {
				cout << "app_11" << endl;
				chunk_set[i] = 0;
				download[i] = 0;
				requested[i] = 0;
				if(i < tracker_->N_C/9) //
					highlight_download[i] = 0; //
			}
		}
	}
		
	seed_start_up = 0;

	act_cons = 0;
	con_counter =0;
	
	first_interest = false;
	first_request = false;
	
	opt_unchoke_counter = OPTIMISTIC_UNCHOKE_INTERVAL;
	opt_unchoke_pid = -1;
		
	choking_timer_ = new ChokingTimer(this);
	tracker_request_timer_ = new TrackerRequestTimer(this);
	leaving_timer_ = new LeavingTimer(this);
	
	start_time = -1.0;
	first_chunk_time = 0.0;
	download_finished = -1.0;
	stop_time = -1.0;
	total_bytes_queued = 0;
	total_bytes_queued_plus_header = 0;
	data_bytes_queued = 0;
	altruistic_bytes_sent = 0;
	total_bytes_recv = 0;
	data_bytes_recv = 0;
	tracker_requests = 0;
	request_id = 0;
	
	// highlight info
	highlight_downsize = 0; 
	bit_rate = tracker_-> B_R *8;             // bits
	rest_bandwidth = (C - bit_rate); // bits
	delayTime_Highlight = 0;
	highlight_size = tracker_-> S_H / 1024.0; //KB

	first_chunk_selection = TRUE;
	
	if (seed) {
		cout << "app_12" << endl;
		reported_max_time = TRUE;
	} else {
		cout << "app_13" << endl;
		reported_max_time = FALSE;
	}

	distBandwidth(tracker_-> S_H/ 1024.0, highlight_size, rest_bandwidth);// Distribution of Bandwidth with Length (OK.)
};



double BitTorrentApp::calDownload_size(long highlight_size, long rest_bandwidth, long length)
{
	//highlight_size = tracker_-> S_H;
	//rest_Mbps = capacity - bit_rate;
	
	if (!seed)
	{
		//while(highlight_size > 0)
		//{
			//delayTime_Highlight =  (bit_rate*1-rest_Mbps*(stop_time-start_time))/rest_Mbps;
			highlight_size = highlight_size - rest_bandwidth;
			
			//cout << " highlight_size :" << highlight_size << endl;
		//}
	}
};

double BitTorrentApp::distBandwidth(long S_H, long highlight_size, long rest_bandwidth)   //(OK.)
{
	//double distRest_bandwidth;
	return distRest_bandwidth = rest_bandwidth * (highlight_size/S_H); //bits
};



//==============================================================
//    DESTRUCTOR
//==============================================================
BitTorrentApp::~BitTorrentApp()
{
	cout << "app_DESTRUCTOR14" << endl;
	if (node_ != NULL) {
		cout << "app_15" << endl;
		node_->setBTApp(NULL);
	}
	
	delete[] chunk_set;
	delete[] download;
	delete[] requested;
	delete[] highlight_download;
	
	// cancel all pending (1) timeouts
	if  (leaving_timer_->status() == 1) {
		cout << "app_16" << endl;
		leaving_timer_->cancel();
	}
		
	delete this->choking_timer_;
	delete this->tracker_request_timer_;
	delete leaving_timer_;
};



//==============================================================
//    DESTRUCTOR
//==============================================================
void BitTorrentApp::delay_bind_init_all()
{
	cout << "app_DESTRUCTOR17" << endl;
	delay_bind_init_one("done_");
	delay_bind_init_one("delay");

	delay_bind_init_one("unchokes");
	delay_bind_init_one("choking_interval");
	delay_bind_init_one("choking_algorithm");
	delay_bind_init_one("pipelined_requests");
	delay_bind_init_one("num_from_tracker");
	delay_bind_init_one("min_peers");
	delay_bind_init_one("max_initiate");
	delay_bind_init_one("max_open_cons");
	delay_bind_init_one("super_seeding");
	
	delay_bind_init_one("leave_option");

};



//==============================================================
//    DESTRUCTOR
//==============================================================
int BitTorrentApp::delay_bind_dispatch(const char *varName, const char *localName, TclObject *tracer)
{
	cout << "app_DESTRUCTOR18" << endl;
	if (delay_bind(varName, localName, "done_", &done, tracer)) return TCL_OK;
	if (delay_bind(varName, localName, "delay", &delay, tracer)) return TCL_OK;
	
	if (delay_bind(varName, localName, "unchokes", &unchokes, tracer)) return TCL_OK;
	if (delay_bind(varName, localName, "choking_interval", &choking_interval, tracer)) return TCL_OK;
	if (delay_bind(varName, localName, "choking_algorithm", &choking_algorithm, tracer)) return TCL_OK;
	if (delay_bind(varName, localName, "pipelined_requests", &pipelined_requests, tracer)) return TCL_OK;
	if (delay_bind(varName, localName, "num_from_tracker", &num_from_tracker, tracer)) return TCL_OK;
	if (delay_bind(varName, localName, "min_peers", &min_peers, tracer)) return TCL_OK;
	if (delay_bind(varName, localName, "max_initiate", &max_initiate, tracer)) return TCL_OK;
	if (delay_bind(varName, localName, "max_open_cons", &max_open_cons, tracer)) return TCL_OK;
	if (delay_bind(varName, localName, "super_seeding", &super_seeding, tracer)) return TCL_OK;
	
	if (delay_bind(varName, localName, "leave_option", &leave_option, tracer)) return TCL_OK;

	return TCL_ERROR;
};



//==============================================================
//     START
//==============================================================
void BitTorrentApp::start()
{
	cout << "app_START19" << endl;
	//start_time = ceil(Scheduler::instance().clock());
	start_time = Scheduler::instance().clock();

	//delayTime_Highlight = start_time;  // ***
	
	running = true;

#ifdef BT_DEBUG
	cout << "[@" << Scheduler::instance().clock() << "] [" << id << "] enters" << endl;
#endif	

	tracker_->reg_peer(node_);	
	tracker_request_timer_->resched(0.0);	
};



//==============================================================
//    STOP
//==============================================================
void BitTorrentApp::stop() {
	cout << "app_STOP20" << endl;
	if (running) {
		cout << "app_21" << endl;
		running = false;

		stop_time = Scheduler::instance().clock();
		
		log_statistics();
		
	#ifdef BT_DEBUG
		cout << "[@" << Scheduler::instance().clock() << "] [" << id << "] leaves" << endl;
	#endif
		
		// delete me from tracker
		tracker_->del_peer(node_);
		
		// cancel all pending (1) timeouts
		if  (tracker_request_timer_->status() == 1) {
			cout << "app_22" << endl;
			tracker_request_timer_->cancel();
		}
		if (choking_timer_->status() == 1) {
			cout << "app_23" << endl;
			choking_timer_->cancel();
		}

		// delete old peer_set and con_list
		for (unsigned int i=0; i<peer_set_.size(); i++) {
		
			if (!peer_set_[i]->am_choking) {
				cout << "app_24" << endl;
				choke_peer(i);
			}
			
			delete [] peer_set_[i]->chunk_set;
			delete [] peer_set_[i]->uploaded_bytes;
			delete [] peer_set_[i]->downloaded_bytes;
			
			// cancel all pending (1) timeouts
			if  (peer_set_[i]->connection_timeout_->status() == 1) {
				cout << "app_25" << endl;
				peer_set_[i]->connection_timeout_->cancel();
			}
			if (peer_set_[i]->keep_alive_timer_->status() == 1) {
				cout << "app_26" << endl;
				peer_set_[i]->keep_alive_timer_->cancel();
			}
			
			delete peer_set_[i]->connection_timeout_;
			delete peer_set_[i]->keep_alive_timer_;
			for (unsigned int j=0; j<peer_set_[i]->peer_requests.size(); j++) {
				delete peer_set_[i]->peer_requests[j];
			}
			peer_set_[i]->peer_requests.clear();
			for (unsigned int j=0; j<peer_set_[i]->my_requests.size(); j++) {
				delete peer_set_[i]->my_requests[j];
			}
			peer_set_[i]->my_requests.clear();
			
			delete peer_set_[i];
		}
		
		peer_set_.clear();
		
		// close connections on the conList_
		Tcl& tcl = Tcl::instance();
		for (unsigned int i=0; i<conList_.size(); i++) {
			
			tcl.evalf("%s close", conList_[i]->tcp_->name());

			conList_[i]->CallCloser();
		
		}
		
		conList_.clear();
	}
	
	done = 1;

#ifdef BT_TELL_TCL
	if (leave_option >= 0) {
		cout << "app_27" << endl;
		// tell tcl script that i am done
		Tcl& tcl = Tcl::instance();
		tcl.evalf("done");
	}
#endif	

};



//==============================================================
//     LOG STATISTICS
//==============================================================
void BitTorrentApp::log_statistics() 
{
	cout << "app_LOG STATISTICS28" << endl;

	// write results in trace file
	fstream file_op(p2ptrace_file ,ios::out|ios::app);
		
	file_op << 
		"ID: " << id << '\n' << 
		"Start time: " << start_time << '\n' << 
		"First Chunk time: " << first_chunk_time << '\n' << 
		"Download Finished: " << download_finished << '\n' << 
		"Stop time: " << stop_time << '\n' << 
		"Total Download time: " << download_finished - start_time << '\n' << 
		"Highlight Size: " << highlight_size << " KB" << '\n' <<
		"Network: " << C/1024.0/1024.0 << " Mbps" << '\n' <<
		"Bit Rate: " << bit_rate/1024.0/1024.0 << " Mbps" << '\n' <<
		"Rest Banswidth: " << rest_bandwidth/1024.0/1024.0 << " Mbps" << '\n' <<
		"time for downloading: " << delayTime_Highlight << '\n' <<
		"distBandwidth : " << distRest_bandwidth/1024.0/1024.0  << " Mbps" << '\n' <<
		"***total_bytes_recv: " << total_bytes_recv << '\n' <<
		endl;
		
	file_op.close();
}

void BitTorrentApp::log_delay() 
{
	//cout << "app_LOG STATISTICS28" << endl;

	// write results in trace file
	fstream file_op(p2ptrace_file ,ios::out|ios::app);
		
	file_op << 
		" ************************************************* " << '\n' <<
		"ID: " << id << '\n' << 
		"Start time: " << start_time << '\n' << 
		"First Chunk time: " << first_chunk_time << '\n' << 
		"Highlight Size: " << highlight_size << " KB" << '\n' <<
		"time for downloading: " << delayTime_Highlight << '\n' <<
		
		"***total_bytes_recv: " << total_bytes_recv << '\n' <<
		"***total_bytes_recv_queued: " << total_bytes_queued << '\n' <<
		//"***nbytes_recv: " << nbytes << '\n' <<
		"***data_bytes_recv: " << data_bytes_recv << '\n' <<
		"**data_bytes_queued: " << data_bytes_queued << '\n' <<
		"distBandwidth : " << distRest_bandwidth/1024.0/1024.0  << " Mbps" << '\n' <<
		" ----------------------------------------------------- " << '\n' <<
		endl;
		
	file_op.close();
}

//==============================================================
//     TRACKER REQUEST
//==============================================================
void BitTorrentApp::tracker_request() {
	cout << "app_TRACKER REQUEST29" << endl;
	if (num_of_cons() < min_peers) {
		cout << "app_30" << endl;
		get_ids_from_tracker();
	}
	check_connections();
	
	tracker_request_timer_->resched(REREQUEST_INTERVAL);
}
	
	
	
//==============================================================
//     GET IDS FROM TRACKER
//==============================================================
bool BitTorrentApp::get_ids_from_tracker() {
	cout << "app_GET IDS FROM TRACKER31" << endl;
	vector<Node *> new_set_;
	peer_list_entry *new_peer;
	unsigned int length;
	
	// update counter
	tracker_requests++;
		
	// get ids from tracker
	new_set_ = tracker_->get_peer_set(num_from_tracker);

	if (new_set_.empty() == false) {
		cout << "app_32" << endl;
		length = new_set_.size();

		// check if new peer id is already in peer set, otherwise add
		for (unsigned int i=0; i<length; i++) {
			if (not_in_peer_set(new_set_[i]->address())) {
				cout << "app_33" << endl;
				new_peer = make_new_peer_list_entry(new_set_[i]);
				peer_set_.push_back(new_peer);
			}
		}
	}
	
	new_set_.clear();
	
	return true;
};



//==============================================================
//    NOT IN PEER SET
//==============================================================
bool BitTorrentApp::not_in_peer_set(int peer_id){

	cout << "app_NOT IN PEER SET34" << endl;
	int i, length;
	
	// check if peer_id is not own id
	if (id==peer_id) {
		cout << "app_35" << endl;
		return false;
	}

	// check if peer set is empty
	length=peer_set_.size();
	if (length!=0) {
		cout << "app_36" << endl;
		// check if peer_id is already in the peer set
		for (i=0; i<length; i++) {
			if (peer_set_[i]->id == peer_id) {
				cout << "app_37" << endl;
				return false;
			}
		}
	}
	return true;
};



//==============================================================
//    MAKE NEW PEER LIST ENTRY
//==============================================================
peer_list_entry* BitTorrentApp::make_new_peer_list_entry(Node *new_node) {
	cout << "app_MAKE NEW PEER LIST ENTRY38" << endl;
	int i;
	peer_list_entry *new_peer = new peer_list_entry;
	
	if (new_node != NULL)
	{ 
		cout << "app_39" << endl;
		new_peer->id = new_node->address(); 
	}
	new_peer->node = new_node;
	new_peer->connected = -1;
	new_peer->con_id = -1;
	new_peer->chunk_set = new int[tracker_->N_C];
	for (i=0; i<tracker_->N_C; i++) {
		new_peer->chunk_set[i]=-1; 
	}
	new_peer->x_up=0.0;
	new_peer->x_down=0.0;
	new_peer->uploaded_bytes = new long[ROLLING_AVERAGE];
	new_peer->downloaded_bytes = new long[ROLLING_AVERAGE];
	for (i=0; i<ROLLING_AVERAGE; i++) {
		new_peer->uploaded_bytes[i] = 0;
		new_peer->downloaded_bytes[i] = 0;
	}
	new_peer->am_choking=true;
	new_peer->peer_choking=true;
	new_peer->am_interested=false;
	new_peer->peer_interested=false;
	new_peer->uploaded_requested_bytes=0;
	new_peer->downloaded_requested_bytes=0;
	new_peer->opt_unchoke_prob=0;
	new_peer->snubbed=0;
	new_peer->peer_upload_quota=0;
	new_peer->greedy = false;
	new_peer->last_unchoke = -1.0;
	new_peer->last_choke = -1.0;
	new_peer->last_msg_recv = -1.0;
	new_peer->last_msg_sent = 0.0;
	new_peer->connection_timeout_ = new ConnectionTimeout(this);
	new_peer->keep_alive_timer_ = new KeepAliveTimer(this);
	new_peer->slow_start = -1;
	new_peer->send_one_chunk = false;
	new_peer->queue_request=false;
	new_peer->super_seeding_chunk_id=-1;
	return new_peer;
};



//==============================================================
//     DELETE PEER FROM LIST
//==============================================================
void BitTorrentApp::delete_peer_from_list(int cID) {
	cout << "app_DELETE PEER FROM LIST40" << endl;
	// delete entry from peer list
	for (unsigned int i=0; i<peer_set_.size(); i++) {
	
		if (cID == peer_set_[i]->con_id) {
			cout << "app_41" << endl;
			if (!peer_set_[i]->am_choking) {
				cout << "app_42" << endl;
				act_cons--;
				peer_set_[i]->am_choking = true;
				peer_set_[i]->uploaded_requested_bytes = 0;
				peer_set_[i]->x_up = 0.0;
				peer_set_[i]->last_choke = Scheduler::instance().clock();
			}
			
			if (!peer_set_[i]->my_requests.empty())	{
				cout << "app_43" << endl;
				// count bytes requested from that peer
				for (unsigned int j=0; j<peer_set_[i]->my_requests.size(); j++) {
					if (j==0) {
						cout << "app_44" << endl;
						requested[peer_set_[i]->my_requests[j]->chunk_id] = requested[peer_set_[i]->my_requests[j]->chunk_id] - peer_set_[i]->my_requests[j]->length + peer_set_[i]->downloaded_requested_bytes;
						
					}
					else {
						cout << "app_45" << endl;
						requested[peer_set_[i]->my_requests[j]->chunk_id] -= peer_set_[i]->my_requests[j]->length;
					}
				}
			}
			
			// clear request queues
			for (unsigned int j=0; j<peer_set_[i]->my_requests.size(); j++) {
				delete peer_set_[i]->my_requests[j];
			}			
			peer_set_[i]->my_requests.clear();
			
			
			for (unsigned int j=0; j<peer_set_[i]->peer_requests.size(); j++) {
				delete peer_set_[i]->peer_requests[j];
			}
			peer_set_[i]->peer_requests.clear();

			delete [] peer_set_[i]->chunk_set;
			delete [] peer_set_[i]->uploaded_bytes;
			delete [] peer_set_[i]->downloaded_bytes;
		
			// cancel all pending (1) timeouts
			if  (peer_set_[i]->connection_timeout_->status() == 1) {
				cout << "app_46" << endl;
				peer_set_[i]->connection_timeout_->cancel();
			}
			if (peer_set_[i]->keep_alive_timer_->status() == 1) {
				cout << "app_47" << endl;
				peer_set_[i]->keep_alive_timer_->cancel();
			}
		
			delete peer_set_[i]->connection_timeout_;
			delete peer_set_[i]->keep_alive_timer_;
			
						
			delete peer_set_[i];
			peer_set_.erase(peer_set_.begin() + i);
			
			return;
		}
	}	
}


//==============================================================
//     CINID2CONPTR
//==============================================================
BitTorrentConnection* BitTorrentApp::conid2conptr (int cID){
	cout << "app_CINID2CONPTR48" << endl;
	vector<BitTorrentConnection *>::iterator it;
	for (it = conList_.begin(); it != conList_.end(); it++) {		
		if ((*it)->conID() == cID) {
			cout << "app_49" << endl;
			return *it;
		}
	}
	
	return NULL;
};



//==============================================================
//     CONNECT
//==============================================================
int BitTorrentApp::connect(Node *node_dst, BitTorrentData msg)
{	
	cout << "app_CONNECT50" << endl;
	BitTorrentApp *dst = node_dst->getBTApp();
	if (dst == NULL)
	{
		cout << "app_51" << endl;
		return -1;
	}
	int act_con_id = con_counter++;
	
	// build TCP Agents on src side
	BitTorrentConnection* new_con = new BitTorrentConnection(node_, dst, this, NULL, act_con_id, -1, NULL);
	conList_.push_back(new_con);

	// build TCP Agents on dst side
	if (dst->connect_step_2(this, act_con_id, new_con->tcp_, new_con) != -1)
	{
		cout << "app_52" << endl;
		send(msg, act_con_id);
		return act_con_id;
	}
	else {	
		cout << "app_53" << endl;
		conList_.pop_back();
		delete new_con;
		return -1;
	}
};



//==============================================================
//     CONNECT STEP 2
//==============================================================
int BitTorrentApp::connect_step_2(BitTorrentApp *dst, int dst_conID, Agent *dst_tcp_, BitTorrentConnection* dst_con) {
	cout << "app_CONNECT STEP 2/54" << endl;
	int act_con_id = con_counter++;
		
	for (unsigned int i=0; i<conList_.size(); i++) {
			
		// peer is identified by node->address, i.e. one peer per node!!!
		if (dst->node_->address() == conList_[i]->dst_address()) {
			cout << "app_55" << endl;
			return -1;
		}
	}
	
	BitTorrentConnection* new_con = new BitTorrentConnection(node_, dst, this, dst_tcp_, act_con_id, dst_conID, dst_con);
	
	conList_.push_back(new_con);
	
	new_con->tcp_->listen();

	dst->connect_step_3(this, dst_conID, act_con_id, new_con->tcp_, new_con);
	
	return 1;
};



//==============================================================
//    CONNECT STEP 3
//==============================================================
int BitTorrentApp::connect_step_3(BitTorrentApp *dst, int conID, int dst_conID, Agent *dst_tcp_, BitTorrentConnection* dst_con_) {

	cout << "app_CONNECT STEP 3/56" << endl;
	BitTorrentConnection *ptr = conid2conptr(conID);
	
	if (ptr == NULL) {
		cout << "app_57" << endl;
		return -1;
	}
	
	ptr->dst_tcp_ = dst_tcp_;
	ptr->set_dst_conID(dst_conID);
	ptr->dst_con = dst_con_;
	
	Tcl& tcl = Tcl::instance();

	tcl.evalf("%s set dst_addr_ [%s set agent_addr_]",
		ptr->tcp_->name(),dst_tcp_->name());
	
	tcl.evalf("%s set dst_port_ [%s set agent_port_]",
		ptr->tcp_->name(),dst_tcp_->name());
	
	tcl.evalf("%s set dst_addr_ [%s set agent_addr_]",
		dst_tcp_->name(),ptr->tcp_->name());
	
	tcl.evalf("%s set dst_port_ [%s set agent_port_]",
		dst_tcp_->name(),ptr->tcp_->name());
		
	return 1;
};



//==============================================================
//     SEND
//==============================================================
void BitTorrentApp::send(BitTorrentData tmp, int cID)
{	
	cout << "app_ SEND58" << endl;
	BitTorrentConnection *ptr = conid2conptr(cID);

	

	// connection was close from remote peer. delete from peer set and return
	if ( cID == -2) {

		cout << "app_59" << endl;
		
cout << "[@" << Scheduler::instance().clock() << "] [" << this->node_->address() << "] send on dead con " << cID << endl;

		// close connections on the conList_
		Tcl& tcl = Tcl::instance();
		tcl.evalf("%s close", ptr->tcp_->name());
		ptr->CallCloser();
		delete_peer_from_list(cID);
		return;
	}
	
	
	if (ptr == NULL) {
		cout << "app_60" << endl;
cout << "[@" << Scheduler::instance().clock() << "] [" << this->node_->address() << "] ERROR: Peer sends to cID " << cID << " mid= " << tmp.message_id_ << endl;

		fprintf(stderr,"[%g] [%s] in send: No connection with connectionID %d\n", Scheduler::instance().clock(), name(), cID);
		
		
		
		// delete connections from conList_
		delete_peer_from_list(cID);
		
		return;
	}
	
	
	if (ptr->dst_tcp_ == NULL) {
		cout << "app_61" << endl;
		fprintf(stderr,"[%g] [%s] No dst connection with connectionID %d\n", Scheduler::instance().clock(), name(), cID);
		return;
	}	
	
	// update KEEP ALIVE timer
	for (int i=0; i < int(peer_set_.size()); i++) {
		if (cID == peer_set_.at(i)->con_id) {
			cout << "app_62" << endl;
			peer_set_[i]->last_msg_sent = Scheduler::instance().clock();
			
			peer_set_[i]->keep_alive_timer_->resched(KEEPALIVE_INTERVAL);
			
			break;
		}
	}

	
	BitTorrentData *msg = new BitTorrentData(tmp);
	BitTorrentDataBuf *p = new BitTorrentDataBuf(msg);
	
	ptr->buf_.insert(p);
	
	ptr->tcp_->sendmsg(tmp.length_);

	// update total byte counter
	total_bytes_queued += tmp.length_;

	//delayTime_Highlight = Scheduler::instance().clock();//*****

	total_bytes_queued_plus_header += tmp.length_ + long(ceil(double(tmp.length_)/1460.0)*40);
};



//==============================================================
//    RECV
//==============================================================
void BitTorrentApp::recv(int nbytes, int cID)
// obtained from tcpapp.cc and modified to fit multiple connections
{

	// exception handling: close agent and delte from list if
	// 1 - peer is not running
	// 2 - cID is unknown
	// 3 - remote BT con is NULL
	
	cout << "app_RECV63" << endl;
	if (!running) {
		cout << "app_64" << endl;
		cout << "[@" << Scheduler::instance().clock() << "] [" << this->node_->address() << "] ERROR: Peer not running, but recv something" << endl;
		
		return;
	}
	

	BitTorrentConnection *tmp = conid2conptr(cID);
	
	if(tmp == NULL) {
		cout << "app_65" << endl;
		cout << "[@" << Scheduler::instance().clock() << "] [" << this->node_->address() << "] ERROR: In recv, but no connection to " << cID << endl;
		fprintf(stderr, "in recv: No connection with cID %d\n", cID);
		
		delete_peer_from_list(cID);

		return;
	}
	
	// check if remote TCP agent exists
	if (tmp->dst_tcp_ == NULL) {
		cout << "app_66" << endl;
		cout << "dst is down" << endl;
		return;
	}
	
	
	if (tmp->dst_con == NULL) {
		cout << "app_67" << endl;
	//cout <<  "[@" << Scheduler::instance().clock() << "] dst_con is NULL " << endl;
		
		//close_connection(cID);
		return;
	}

		
	while (nbytes + tmp->curbytes_ > 0) {
		cout << "app_68" << endl;
		// if no old data, get new
		if (tmp->curdata_ == NULL) {
			cout << "app_69" << endl;
			tmp->curdata_ = tmp->dst_con->buf_.detach();
			
			if (tmp->curdata_ == NULL) {
				cout << "app_70" << endl;
				fprintf(stderr, "Error: Where is my message?\n");	
				close_connection(cID);
				return;
			}
		}	
		
		// received exactly one message
		if (nbytes + tmp->curbytes_ == tmp->curdata_->data()->length_) {
			cout << "app_71" << endl;
			handle_recv_msg(*(tmp->curdata_->data()), cID);
			delete tmp->curdata_;
			tmp->curdata_ = NULL;
			nbytes=0;
			tmp->curbytes_=0;
			return;
		}
		
		// received less than one message
		else if (nbytes + tmp->curbytes_ < tmp->curdata_->data()->length_) {
			cout << "app_72" << endl;
			tmp->curbytes_ += nbytes;
			nbytes=0;
			return;
		}
		
		// received more than one message
		else {	
			cout << "app_73" << endl;
			handle_recv_msg(*(tmp->curdata_->data()), cID);
			nbytes = nbytes + tmp->curbytes_ - tmp->curdata_->data()->length_;
			tmp->curbytes_=0;
			delete tmp->curdata_;
			tmp->curdata_ = NULL;
		}
	}
};



//==============================================================
//     RECV TCP FIN
//==============================================================
void BitTorrentApp::close_connection(int cID) {
	cout << "app_RECV TCP FIN74" << endl;
	vector<BitTorrentConnection *>::iterator it;
	for (it = conList_.begin(); it != conList_.end(); it++) {
		if ((*it)->conID() == cID) {
			cout << "app_75" << endl;
			// delete the peer from the peer list
			delete_peer_from_list(cID);
		
			// close connections on the conList_
			if ((*it)->tcp_ != NULL) {
				cout << "app_76" << endl;
				Tcl& tcl = Tcl::instance();
				tcl.evalf("%s close", (*it)->tcp_->name());
				(*it)->CallCloser();
			}
			
			conList_.erase(it);
			
			return;
		}
	}
}



//==============================================================
//     TCP send all data
//==============================================================
void BitTorrentApp::upcall_send(int cID) {
	cout << "app_TCP send all data77" << endl;
 	vector<BitTorrentConnection *>::iterator it;
 	for (it = conList_.begin(); it != conList_.end(); it++) {
 		if ((*it)->conID() == cID) {
			cout << "app_78" << endl;
			for (unsigned int i=0; i<peer_set_.size(); i++) {
				if (cID == peer_set_[i]->con_id) {
					cout << "app_79" << endl;
					if (peer_set_[i]->peer_requests.empty()) {
						// no request queued at app layer
						cout << "app_80" << endl;
						peer_set_[i]->queue_request = FALSE;
					
					} else {
						cout << "app_81" << endl;
						request_list_entry *next_request;
						
						next_request =  peer_set_[i]->peer_requests[0];
					
						peer_set_[i]->peer_requests.erase(peer_set_[i]->peer_requests.begin()); 
						
						peer_set_[i]->queue_request = TRUE;
						
						peer_set_[i]->uploaded_bytes[0] += next_request->length;

						// found queued request and give it to tcp
						send(createPieceMessage( next_request->chunk_id, next_request->length, next_request->rid), cID);
						
						if (super_seeding == 1 && peer_set_[i]->uploaded_bytes[0] == tracker_->S_C) {
							cout << "app_82" << endl;
							peer_set_[i]->super_seeding_chunk_id = next_request->chunk_id;
							
							//super_seeding_chunk_set[bittorrentData.chunk_index_] = -1;
							peer_set_[i]->uploaded_bytes[0] = 0;
							
							choke_peer(i);
					
							if (act_cons <= unchokes) {
								cout << "app_83" << endl;
								check_choking();
							}
					
						}
						
						delete next_request;	
					}
					
					return;
				}
			}
 		}
 	}
}



//==============================================================
//     HANDLE RECV MSG
//==============================================================
void BitTorrentApp::handle_recv_msg(BitTorrentData bittorrentData, int connectionID)
{
	cout << "app_HANDLE RECV MSG84" << endl;
	// update total received bytes counter
	total_bytes_recv +=bittorrentData.length_;

	
	if (!running) {
		cout << "app_85" << endl;
		return;
	}

	int sender_index = -1;
	for (unsigned int i=0; i<peer_set_.size(); i++) {
		if (connectionID == peer_set_[i]->con_id) {
			cout << "app_86" << endl;
			sender_index = i;
			
			// re-new timout settings
			resched_timeout(i);
			break;
		}
	}

	int message_type =bittorrentData.message_id_;

	switch(message_type)
	{
	
	case HANDSHAKE:
	{
		cout << "app_87" << endl;
#ifdef BT_DEBUG
		cout << "[@" << Scheduler::instance().clock() << "] [" << id << "] HANDSHAKE [" << bittorrentData.source_ip_address_ << "]" << endl;
#endif

		handle_handshake(bittorrentData, connectionID);
		break;	
	}
	

	case BITFIELD:
	{
		cout << "app_88" << endl;
#ifdef BT_DEBUG
		cout << "[@" << Scheduler::instance().clock() << "] [" << id << "] BITFIELD [" << bittorrentData.source_ip_address_ << "]" << endl;
#endif

		handle_bitfield(bittorrentData, connectionID);
		break;
	}
	
	
	case HAVE:
	{
		cout << "app_89" << endl;
#ifdef BT_DEBUG
		cout << "[@" << Scheduler::instance().clock() << "] [" << id << "] HAVE " << bittorrentData.chunk_index_ <<" [" << bittorrentData.source_ip_address_ << "]" << endl;
#endif

		for (unsigned int i=0; i<peer_set_.size(); i++) {
			if (connectionID == peer_set_[i]->con_id) {
				cout << "app_90" << endl;
				peer_set_[i]->chunk_set[bittorrentData.chunk_index_] = 1;
				
				// look if i am interested in this chunk
				if (peer_set_[i]->am_interested == 0 && chunk_set[bittorrentData.chunk_index_] == 0) {
					cout << "app_91" << endl;
					peer_set_[i]->am_interested=1;
					send(createInterestedMessage(), connectionID);
				}
				
				// if i am unchoked, interested in this chunk and request queue not full, send also a reuqest
				if (!peer_set_[i]->peer_choking 
				&& chunk_set[bittorrentData.chunk_index_] == 0
				&& int(peer_set_[i]->my_requests.size()) < pipelined_requests) {
					cout << "app_92" << endl;
					make_request(connectionID, pipelined_requests - int(peer_set_[i]->my_requests.size()));
				}
				
				// if I am in super-seeding mode, incr seeding chunk set
				if (super_seeding == 1) {
					cout << "app_93" << endl;
					if (super_seeding_chunk_set[bittorrentData.chunk_index_] != -2) {
						cout << "app_94" << endl;
						super_seeding_pending_chunks--;
						super_seeding_chunk_set[bittorrentData.chunk_index_]= -2;
					} else {
						cout << "app_95" << endl;
						// find peer which forwarded that piece
						for (unsigned int j=0; j<peer_set_.size(); j++) {
							
							if ((peer_set_[j]->super_seeding_chunk_id == bittorrentData.chunk_index_) && (peer_set_[i]->id != peer_set_[j]->id)) {
								cout << "app_96" << endl;
								peer_set_[j]->super_seeding_chunk_id= -1;
								super_seeding_have(j, peer_set_[j]->con_id);
								break;
							}
							
						}
					}
										
					if (super_seeding_pending_chunks <= unchokes) {
						cout << "app_97" << endl;
						super_seeding_have(i, connectionID);
					}
				}
			}
		}
		break;
	}
	
	
	case INTERESTED:
	{
		cout << "app_98" << endl;
#ifdef BT_DEBUG
		cout << "[@" << Scheduler::instance().clock() << "] [" << id << "] INTERESTED [" << bittorrentData.source_ip_address_ << "]" << endl;
#endif

		for (unsigned int i=0; i<peer_set_.size(); i++) {
			if (connectionID == peer_set_[i]->con_id) {
				
				cout << "app_99" << endl; 
				peer_set_[i]->peer_interested = true;
			}
		}
		
		// if first interest ever, resched choking timer
		if (first_interest==false) {
			cout << "app_100" << endl;
			first_interest=true;
			choking_timer_->resched(0.0);
		} 
		// if less then unchokes, resched choking timer in super-seeding mode
		else if (super_seeding == 1 && act_cons < unchokes) {
			cout << "app_101" << endl;
			check_choking();
			
		}

		break;
	}
	
	
	case NOT_INTERESTED:
	{
		cout << "app_102" << endl;
#ifdef BT_DEBUG
		cout << "[@" << Scheduler::instance().clock() << "] [" << id << "] NOT_INTERESTED [" << bittorrentData.source_ip_address_ << "]" << endl;
#endif

		for (unsigned int i=0; i<peer_set_.size(); i++) {
			if (connectionID == peer_set_[i]->con_id) {
				cout << "app_103" << endl;
				peer_set_[i]->peer_interested = false;
				if (!peer_set_[i]->am_choking) {
					cout << "app_104" << endl;
					choke_peer(i);
				}
			}
		}
		break;
	}
	
	
	case CHOKE:
	{
		cout << "app_105" << endl;
#ifdef BT_DEBUG
		cout << "[@" << Scheduler::instance().clock() << "] [" << id << "] CHOKE [" << bittorrentData.source_ip_address_ << "]" << endl;
#endif

		for (unsigned int i=0; i<peer_set_.size(); i++) {
			if (connectionID == peer_set_[i]->con_id) {
				cout << "app_106" << endl;
				// report when i am already choked
				if (peer_set_[i]->peer_choking) {
					cout << "app_107" << endl;
					fprintf(stderr, "Peer is already choked\n");
					
					cout << "[@" << Scheduler::instance().clock() << "[" << id << "] was already choked by [" << bittorrentData.source_ip_address_ << "]" << " " << peer_set_[i]->con_id <<  endl;
				}
			
				// peer chokes me
				peer_set_[i]->peer_choking = true;
				
				if (!peer_set_[i]->my_requests.empty())	{
					cout << "app_108" << endl;
					// count bytes requested from that peer
					for (unsigned int j=0; j<peer_set_[i]->my_requests.size(); j++) {
						if (j==0) {
							cout << "app_109" << endl;
							requested[peer_set_[i]->my_requests[j]->chunk_id] = requested[peer_set_[i]->my_requests[j]->chunk_id] - peer_set_[i]->my_requests[j]->length + peer_set_[i]->downloaded_requested_bytes;
							
						}
						else {
							cout << "app_110" << endl;
							requested[peer_set_[i]->my_requests[j]->chunk_id] -= peer_set_[i]->my_requests[j]->length;
						}
					}
				}
				
				// set byte counter to zero
				peer_set_[i]->downloaded_requested_bytes=0;

				// clear request queues
				for (unsigned int j=0; j<peer_set_[i]->my_requests.size(); j++) {
					delete peer_set_[i]->my_requests[j];
				}
				peer_set_[i]->my_requests.clear();
			}
		}
		break;
	}
	
	
	case UNCHOKE:
	{
		cout << "app_111" << endl;
#ifdef BT_DEBUG
		cout << "[@" << Scheduler::instance().clock() << "] [" << id << "] UNCHOKE [" << bittorrentData.source_ip_address_ << "]" << endl;
#endif

		for (unsigned int i=0; i<peer_set_.size(); i++) {
			if (connectionID == peer_set_[i]->con_id) {
				cout << "app_112" << endl;
				peer_set_[i]->peer_choking = false;
				peer_set_[i]->last_remote_unchoke = bittorrentData.timestamp;
				if (peer_set_[i]->am_interested) {
					cout << "app_113" << endl;
					make_request(connectionID, pipelined_requests);
				} else {
					cout << "app_114" << endl;
					send(createNotInterestedMessage(), peer_set_[i]->con_id);
				}
				break;
			}
		}
		break;
	}

	
	case REQUEST:
	{
		cout << "app_115" << endl;
#ifdef BT_DEBUG		
		cout << "[@" << Scheduler::instance().clock() << "] [" << id << "] REQUEST " << bittorrentData.chunk_index_ <<" [" << bittorrentData.source_ip_address_ << "] (" << bittorrentData.req_piece_begin_ << ")" <<endl;
#endif
		handle_request(bittorrentData, connectionID);
		break;
	}
	
	
	case PIECE:
	{
		cout << "app_116" << endl;
#ifdef BT_DEBUG
		cout << "[@" << Scheduler::instance().clock() << "] [" << id << "] PIECE " << bittorrentData.chunk_index_ << ": "<< bittorrentData.req_piece_length_ << "B (" << bittorrentData.req_piece_begin_ <<") [" << bittorrentData.source_ip_address_ << "]" << endl;
#endif
		handle_piece(bittorrentData, connectionID);
		break;
	}
	
	
	case CANCEL:
	{
		cout << "app_117" << endl;
#ifdef BT_DEBUG
		cout << "[@" << Scheduler::instance().clock() << "] [" << id << "] CANCEL " << bittorrentData.chunk_index_ << " [" << bittorrentData.source_ip_address_ << "]" << endl;
#endif

		for (unsigned int i=0; i<peer_set_.size(); i++) {
			
			if (connectionID == peer_set_[i]->con_id && !peer_set_[i]->peer_requests.empty()) {
				cout << "app_118" << endl;
				vector<request_list_entry *>::iterator it;
				for( it = peer_set_[i]->peer_requests.begin(); it != peer_set_[i]->peer_requests.end(); it++ ) {	
					if ((*it)->chunk_id == bittorrentData.chunk_index_) {
						cout << "app_119" << endl;
						delete *it;
						
						peer_set_[i]->peer_requests.erase(it);
						
						break;
					}	
				}
			}
		}
		break;
	}
	
	
	
	case KEEPALIVE:
	{
		cout << "app_120" << endl;
#ifdef BT_DEBUG
		cout << "[@" << Scheduler::instance().clock() << "] [" << id << "] KEEP ALIVE " << " [" << bittorrentData.source_ip_address_ << "]" << endl;
#endif
		// Timeout is already rescheduled. Nothing to do.

		break;
	}

	default:
	{
		cout << "app_121" << endl;
		fprintf(stderr, "Message Type %i unknown! \n", message_type);
	}
	
	}	// end switch
	
};



//==============================================================
//     CHECK INTEREST
//==============================================================
bool BitTorrentApp::check_interest(int peer_set_index) {
	cout << "app_CHECK INTEREST122" << endl;
	for (int i=0; i<tracker_->N_C; i++) {
		if (peer_set_[peer_set_index]->chunk_set[i] == 1 && chunk_set[i] == 0) {
			
			cout << "app_123" << endl;
			return true;
		}
	}
	return false;
};



//==============================================================
//     CHECK CONNECTIONS
//==============================================================
void BitTorrentApp::check_connections() {
	cout << "app_CHECK CONNECTIONS124" << endl;
	int con_reply;

	if (conList_.size() < unsigned(max_initiate)) {		
		cout << "app_125" << endl;
		vector<peer_list_entry*>::iterator it;
		it = peer_set_.begin();
	
		for (it = peer_set_.begin(); it != peer_set_.end(); it++) {
			
			if (conList_.size() > unsigned(max_initiate)){
				cout << "app_126" << endl;
				break;
			}

			if ((*it)->connected == -1) {
				cout << "app_127" << endl;
				con_reply = connect((*it)->node, createHandshakeMessage());
				
				if (con_reply != -1) {
					cout << "app_128" << endl; 
					(*it)->connected = 0;
						
					(*it)->connection_timeout_->resched(TIMEOUT_CHECK_INTERVAL);
					(*it)->last_msg_recv = Scheduler::instance().clock();
					(*it)->con_id = con_reply;
				}	
			}
		}
	}
};



//==============================================================
//     CHECK CHOKING
//==============================================================
void BitTorrentApp::check_choking() {
	cout << "app_CHECK CHOKING129" << endl;
#ifdef BT_DEBUG
	cout << "[@" << Scheduler::instance().clock() << " [" << id << "] check choking" << endl;
#endif

	////////////////////////////////
	// BitTorrent Choking Algorithm
	////////////////////////////////
	
	if (super_seeding == 1) {
		cout << "app_130" << endl;
		super_seeding_choking();
		return;
	}
	
	// If I have less neighbors than peers I like to unchoke -> unchoke all peers:
	if (signed(peer_set_.size()) <= unchokes) {
		cout << "app_131" << endl;
		for (unsigned int j=0; j<peer_set_.size(); j++) {	
			// unchoke
			if (peer_set_[j]->am_choking && peer_set_[j]->peer_interested) {
				cout << "app_132" << endl;
				unchoke_peer(j);
			}
		}		
		choking_timer_->resched(choking_interval);
		return;
	}
	
	opt_unchoke_counter--;

	vector<service_info *> unchoke_cand;
	vector<service_info *>::iterator sit;
	
	bool found_opt_unchoke = FALSE;


	for (unsigned int i=0; i<peer_set_.size(); i++) {
		
		if (peer_set_[i]->connected == 1 
		&& peer_set_[i]->peer_interested) {
			cout << "app_133" << endl;
			service_info *new_unchoke_cand = new service_info;
			new_unchoke_cand->id = peer_set_[i]->id;
			new_unchoke_cand->index = i;
			new_unchoke_cand->service_rate = 0;
			
			for (int k=0; k<ROLLING_AVERAGE; k++) {
			
				if (seed==1) {
					cout << "app_134" << endl;
					// SEED
					new_unchoke_cand->service_rate += peer_set_[i]->uploaded_bytes[k];	
					
				} else {
					cout << "app_135" << endl;
					// LEEECHER
					new_unchoke_cand->service_rate += peer_set_[i]->downloaded_bytes[k];
					
				}
			}
			
			unchoke_cand.push_back(new_unchoke_cand);
		}
	}

	if (signed(unchoke_cand.size()) > unchokes -1) {
		cout << "app_136" << endl;
		// sort unchoking candidates
		sort(unchoke_cand.begin(), unchoke_cand.end(), CompareServices());
	
		// Mem has to be freed before deleting the pointers in the vector
		for( sit = unchoke_cand.begin(); sit != unchoke_cand.end() - (unchokes - 1); sit++ ) {
			delete *sit;
		}
		unchoke_cand.erase( unchoke_cand.begin(), unchoke_cand.end() - (unchokes - 1));
	}

	
	bool choke_peerQ;
	for (unsigned int i=0; i<peer_set_.size(); i++) {
		choke_peerQ = TRUE;
		
		// loop through fastest uploaders
		for (sit = unchoke_cand.begin(); sit != unchoke_cand.end(); sit++ ) {
			
			if ((*sit)->id == peer_set_[i]->id) {
				cout << "app_137" << endl;
				choke_peerQ = FALSE;
				
				// unchoke peers on the list when not already unchoked
				if (peer_set_[i]->am_choking) {
					cout << "app_138" << endl;
					unchoke_peer(i);
				}
				
				// if peer is my optimistic unchoke, find a new one
				if ((*sit)->id == opt_unchoke_pid) {
					cout << "app_139" << endl;
					opt_unchoke_pid=-1;
				}
			}
		}
		
		
		// when peer is unchoked but not on the new list, send a choke
		if (peer_set_[i]->id == opt_unchoke_pid && peer_set_[i]->peer_interested) {
			cout << "app_140" << endl;
			found_opt_unchoke = TRUE;
		} else if (!peer_set_[i]->am_choking && choke_peerQ) {
			cout << "app_141" << endl;
			choke_peer(i);
		}
		
		// set byte counters to 0
		for (int j=ROLLING_AVERAGE-1; j>0; j--) {
			peer_set_[i]->downloaded_bytes[j] = peer_set_[i]->downloaded_bytes[j-1];
			peer_set_[i]->uploaded_bytes[j] = peer_set_[i]->uploaded_bytes[j-1];
		} 
		peer_set_[i]->downloaded_bytes[0]= 0;
		peer_set_[i]->uploaded_bytes[0]= 0;
	}
		
	// free mem
	for( sit = unchoke_cand.begin(); sit != unchoke_cand.end(); sit++ ) {
		delete *sit;
	}
	unchoke_cand.clear();
	
	
	// optimistic unchoked peer was not found, maybe it left the network. Find a new one.
	if (!found_opt_unchoke) {
		cout << "app_142" << endl;
		opt_unchoke_pid=-1;
	}
	
	// optimistic unchoke
	int old_opt_unchoke_pid = opt_unchoke_pid;;
	if (opt_unchoke_counter==0 || opt_unchoke_pid==-1) {
	
		// count the number of interested peers
		int interested_peers = 0;
		for (int i=0; i<signed(peer_set_.size()); i++) {
			
			// when peer was choked in this interval we do not unchoke him here
			if (!peer_set_[i]->am_choking || !peer_set_[i]->peer_interested || peer_set_[i]->last_choke == Scheduler::instance().clock()) {
				cout << "app_143" << endl;
				peer_set_[i]->opt_unchoke_prob=0;
			}
			
			interested_peers += peer_set_[i]->opt_unchoke_prob;
		}
		
		if (interested_peers!=0 && peer_set_.size()>0) {
			cout << "app_144" << endl;
			// get random number
			int rand_num = rand_.uniform(interested_peers)+1;
				
			// find the random chosen peer in peer set
			int index =0;
			while (index < signed(peer_set_.size())) {
				rand_num -=peer_set_[index]->opt_unchoke_prob;
				if (rand_num <= 0) {
					cout << "app_145" << endl;
					opt_unchoke_pid = peer_set_[index]->id;
					
					if (peer_set_[index]->am_choking) {
						
						cout << "app_146" << endl; 
						unchoke_peer(index);
					}
					
					index = peer_set_.size();
				} else {
					cout << "app_147" << endl;
					index++;
				}
			}
		}
		
		// reschedule optimistic unchoke
		opt_unchoke_counter = OPTIMISTIC_UNCHOKE_INTERVAL;
			
		for (int i=0; i<signed(peer_set_.size()); i++) {
		
			// set opt unchoke prob to 1 for all peers
			if (peer_set_[i]->connected==1) {
				cout << "app_148" << endl;
				peer_set_[i]->opt_unchoke_prob=1;
			}
			
			// choke old opt unchoke when it is not also the new one
			if ((peer_set_[i]->id == old_opt_unchoke_pid) && (peer_set_[i]->id != opt_unchoke_pid) && !peer_set_[i]->am_choking) {
				cout << "app_149" << endl;
				choke_peer(i);
			}
		}
	}
	
	// reschedule choking timer
	choking_timer_->resched(choking_interval);
};



//==============================================================
//     MAKE REQUEST
//==============================================================
void BitTorrentApp::make_request(int cID, int no_of_requests){	
	cout << "app_MAKE REQUEST150" << endl;
	int sender_index=-1, chunk_id;
	
	long missing_part=-1;
	int no_of_req=0;
	

	// find sender in peer set
	for (unsigned int i=0; i<peer_set_.size(); i++) {
		if (peer_set_[i]->con_id == cID) {
			cout << "app_151" << endl;
			sender_index=i;
			break;
		}
	}

	// select chunk to request
	chunk_id = chunk_selection(sender_index);
	
	// make several requests for pipelining
	while ((chunk_id!=-1) && (no_of_requests > 0)) {
		
		// check if it is the last maybe not full-sized chunk
		if (chunk_id == tracker_->N_C-1) {
			cout << "app_152" << endl;
			missing_part = tracker_->S_F - (tracker_->N_C-1)*tracker_->S_C - requested[chunk_id]; 
		} else {
			cout << "app_153" << endl;
			missing_part = tracker_->S_C - requested[chunk_id];
		}
		
		// check for error
		if (missing_part <= 0) {
			cout << "app_154" << endl;
			cout << "ERROR: Negative missing part" << endl;
			break;
		}
		
		while ((missing_part > 0) && (no_of_requests > 0))  {
			
			// put request in my own request queue
			request_list_entry *new_request = new request_list_entry;
			new_request->chunk_id = chunk_id;
			new_request->length = MIN(missing_part, REQUESTED_PIECE_LENGTH);
			new_request->rid = request_id;
			peer_set_[sender_index]->my_requests.push_back(new_request);
				
			// send request
			send(createRequestMessage(chunk_id, MIN(missing_part, REQUESTED_PIECE_LENGTH), request_id, peer_set_[sender_index]->last_remote_unchoke), cID);
			
			// update requested counter
			requested[chunk_id]+=MIN(missing_part, REQUESTED_PIECE_LENGTH);
			
			missing_part -= MIN(missing_part, REQUESTED_PIECE_LENGTH);
			
			no_of_requests--;
			request_id++;
			
			// for debugging/info only
			no_of_req++;
		}
		
		
		if (no_of_requests > 0) {
			cout << "app_155" << endl;
			// select chunk to request
			chunk_id = chunk_selection(sender_index);
			
			//calDownload_size(highlight_size, C);  // ***
		}
	}
}



//==============================================================
//     CHUNK SELECTION
//==============================================================
int BitTorrentApp::chunk_selection(int sender_index) {
	cout << "app_CHUNK SELECTION156" << endl;
	int chunk_id =-1;

	if (choking_algorithm == BITTORRENT_WITH_PURE_RAREST_FIRST){ 
		cout << "app_157" << endl; 
		goto rarestfirst_mark;
	}
	
	////////////////////
	/// STRICT PRIORITY:
	////////////////////
	// check for incomplete chunks, request the chunk which has at least missing bytes
	
	if (sender_index < 0 || sender_index > int(peer_set_.size()) ) {
		cout << "app_158" << endl;
		cout << "ERROR: Incorrect sender_index in chunk_selection" << endl;
	}
	
	for (int i=0; i<tracker_->N_C; i++) {
		
		


int requested_bytes=0;
		
		if ( 
			(chunk_set[i] == 0)
			&& (peer_set_[sender_index]->chunk_set[i] == 1)
			&& (requested[i] > requested_bytes)
			&& ( (requested[i] < tracker_->S_C) || ((i==tracker_->N_C -1) && (requested[i] < (tracker_->S_F - (tracker_->N_C - 1) * tracker_->S_C))))
		) {
			cout << "app_159" << endl;
			chunk_id = i;
			requested_bytes = requested[i];	
		}
	}
	if (chunk_id != -1) {
		return chunk_id;
	}
	
	///////////////////////	
	/// RANDOM FIRST PIECE:
	///////////////////////
	// when the first piece is not assembled, request a chunk randomly not the rarest
	
	if (first_chunk_selection) {
		cout << "app_160" << endl;
		vector <int> full_pieces;
		for (int i=0; i<tracker_->N_C; i++) {
		
			// uploaded has the chunk? (Since I have nothing I am interested in every chunk)
			if (
				
				peer_set_[sender_index]->chunk_set[i] == 1
				&& ( requested[i] < tracker_->S_C || ( (i == tracker_->N_C -1) && (requested[i] < ( tracker_->S_F - (tracker_->N_C - 1 ) * tracker_->S_C))))
			) {
				cout << "app_161" << endl;
				full_pieces.push_back(i);
			}
		}
		
		if (full_pieces.size() == 0) {
			cout << "app_162" << endl;
			return -1;
		} 

		chunk_id = full_pieces[rand_.uniform((int)full_pieces.size())];
		full_pieces.clear();
		
		return chunk_id;
	}

	
	
	/////////////////
	/// RAREST FIRST
	////////////////
	
rarestfirst_mark:

	vector<int> occurrence;
	occurrence.assign(tracker_->N_C, 0);
	
	// count occurrence of a piece 
	for (unsigned int i=0; i<peer_set_.size(); i++) {
		for (int j=0; j<tracker_->N_C; j++) {
			if (peer_set_[i]->chunk_set[j]==1) {
				occurrence[j]++;
			}
		}
	}
	
	
	// init counter for minimal occurrence and set it to a high value
	int min_occurrence = peer_set_.size() +1;
	
	vector<int> rarest_pieces;
	
	// iterate pieces 
	for (unsigned int i=0; i<occurrence.size(); i++) {
		
		// if uploader has the piece and I donot
		if ( 
			(requested[i] < tracker_->S_C 
			|| (int(i) == tracker_->N_C -1 && requested[i] < tracker_->S_F - (tracker_->N_C-1)*tracker_->S_C)) 
			&& peer_set_[sender_index]->chunk_set[i] == 1 
			&& chunk_set[i] == 0
		) {
			cout << "app_163" << endl;
			
			// if active piece is rarer as min_occurrence, save it and drop old values
			if (occurrence[i] < min_occurrence) {
				cout << "app_164" << endl;
				rarest_pieces.clear();
				rarest_pieces.push_back(i);
				min_occurrence = occurrence[i];
			}
				
			// if active piece is as rar as min_occurence, save it
			else if (occurrence[i] == min_occurrence){
				cout << "app_165" << endl;
				rarest_pieces.push_back(i);		
			}
		}
	}
	
	
	if (rarest_pieces.size() == 0) {
		cout << "app_166" << endl;
		return -1;
	} 
	
	// return one of the rarest pieces (randomly)
	chunk_id = rarest_pieces[rand_.uniform((int)rarest_pieces.size())];
	
	rarest_pieces.clear();
	
	return chunk_id;
	
}



//==============================================================
//     DOWNLOADING FROM OTHER PEER
//==============================================================
bool BitTorrentApp::downloading_from_other_peer(int chunk_index, int sender_index) {
	cout << "app_DOWNLOADING FROM OTHER PEER167" << endl;
	for (int i=0; i<signed(peer_set_.size()); i++) {
		if (!peer_set_[i]->my_requests.empty() && sender_index!=i) {
			cout << "app_168" << endl;
			for (unsigned int j=0; j<peer_set_[i]->my_requests.size(); j++) {
				if (peer_set_[i]->my_requests[j]->chunk_id == chunk_index) {
					cout << "app_169" << endl;
					return true;
				}
			}
		}
	} 	
	return false;
}



//==============================================================
//     CHUNK COMPLETE
//==============================================================
void BitTorrentApp::chunk_complete(int chunk_id, int cID)
{
	cout << "app_170" << endl;
	int i;

#ifdef BT_DEBUG
	cout << "[@" << Scheduler::instance().clock() << "] [" << id << "] chunk " << chunk_id <<  " completed" << endl;
#endif	
	
	chunk_set[chunk_id]=1;
	int N_DC=0;
	for (i=0; i<tracker_->N_C; i++) {
		if (chunk_set[i]==1) {
			cout << "app_171" << endl;
			N_DC++;
		} 
	}
	
	// inform other peers about new chunk
	for (i=0; i < signed(peer_set_.size()); i++) {

		if (peer_set_[i]->connected == 1) {	
			cout << "app_172" << endl;
			send(createHaveMessage(chunk_id), peer_set_[i]->con_id);
			
			// send NOT_INTERESTED msg to other peers where i waited exactly for this chunk 
			if (peer_set_[i]->am_interested) {
				cout << "app_173" << endl;
				// check interest
				if (!check_interest(i)) {
					cout << "app_174" << endl;
					peer_set_[i]->am_interested=false;
					send(createNotInterestedMessage(), peer_set_[i]->con_id);
				}
			}
		
			
			// cancel all downloads from peers which up this chunk
			if (!peer_set_[i]->my_requests.empty()) {
				cout << "app_175" << endl;
				vector<request_list_entry *>::iterator it;
				for( it = peer_set_[i]->my_requests.begin(); it != peer_set_[i]->my_requests.end(); it++ ) {	
					
					if ((*it)->chunk_id == chunk_id) {
						cout << "app_176" << endl;
						// send CANCEL
						send(createCancelMessage(chunk_id, (*it)->length, (*it)->rid), peer_set_[i]->con_id);	
						
						// delete from my request queue
						delete *it;
						peer_set_[i]->my_requests.erase(it);
						
						break;
					}
				}
			}
		}
	}
	
	// first chunk ?
	if (N_DC == 1) {
		cout << "app_177" << endl;
		first_chunk_selection = FALSE;

		// analysis only
		first_chunk_time = Scheduler::instance().clock();
	} 
	
	// file complete ?
	if (N_DC == tracker_->N_C) {
		cout << "app_178" << endl;
#ifdef BT_DEBUG
	cout << "[@" << Scheduler::instance().clock() << "] [" << id << "] download complete" << endl;
#endif
		
		// peer is seed now
		seed=1;
		download_finished = Scheduler::instance().clock();
	
		// set leaving timer to stop application
		compute_leaving_time();
	}
}



//==============================================================
//     CHOKE PEER
//==============================================================
void BitTorrentApp::choke_peer(int peer_set_index){
	cout << "app_CHOKE PEER179" << endl;
//cout << "[@" << Scheduler::instance().clock() << "] [" << id << "] choke " << peer_set_[peer_set_index]->id << endl;


	if (!peer_set_[peer_set_index]->am_choking) {
		cout << "app_180" << endl;
		act_cons--;
	}
	peer_set_[peer_set_index]->am_choking = true;
	for (unsigned int j=0; j<peer_set_[peer_set_index]->peer_requests.size(); j++) {
		delete peer_set_[peer_set_index]->peer_requests[j];
	}
	peer_set_[peer_set_index]->peer_requests.clear();
	peer_set_[peer_set_index]->uploaded_requested_bytes = 0;
	peer_set_[peer_set_index]->x_up = 0.0;
	peer_set_[peer_set_index]->last_choke = Scheduler::instance().clock();
	

	send(createChokeMessage(), peer_set_[peer_set_index]->con_id);	
};



//==============================================================
//     UNCHOKE PEER
//==============================================================
void BitTorrentApp::unchoke_peer(int peer_set_index){
	cout << "app_UNCHOKE PEER181" << endl;
//cout << "[@" << Scheduler::instance().clock() << "] [" << id << "] unchoke " << peer_set_[peer_set_index]->id << endl;

	if (peer_set_[peer_set_index]->am_choking) {
		cout << "app_182" << endl;
		act_cons++;
	}
	peer_set_[peer_set_index]->am_choking = false;
	peer_set_[peer_set_index]->greedy = true;
	peer_set_[peer_set_index]->last_unchoke = Scheduler::instance().clock();
	peer_set_[peer_set_index]->queue_request = false;
	send(createUnchokeMessage(), peer_set_[peer_set_index]->con_id);
};



//==============================================================
//     COMPUTE LEAVING TIME
//==============================================================
void BitTorrentApp::compute_leaving_time() {
	cout << "app_COMPUTE LEAVING TIME183" << endl;
	// when a peer finishs its download, it could leave the p2p network.
	// Three different leaving scenarios are implemented:
	// - immediate leave (LEAVE_PARAM = 0)
	// - not leaving (LEAVE_PARAM < 0)
	// - leaving network according a exponential distribution with mean specified by LEAVE_PARAM
	
	if (leave_option == 0) {
		cout << "app_184" << endl;
		leaving_timer_->resched(0.0);
	}
	else if (leave_option > 0) {
		cout << "app_185" << endl;
		RNG rand_;
		double r = rand_.exponential(leave_option);
		leaving_timer_->resched(r);
	}
	else {
		cout << "app_186" << endl;
	
#ifdef BT_TELL_TCL
		// For analysis only!
		//peer stays in the network, call done procedure here		
		// tell tcl script that i am done
		Tcl& tcl = Tcl::instance();
		tcl.evalf("done");
#endif
	}
	
}



//==============================================================
//     RESCHED TIMEOUT
//==============================================================
void BitTorrentApp::resched_timeout(int index) {	
	cout << "app_RESCHED TIMEOUT187" << endl;
	peer_set_[index]->connection_timeout_->resched(TIMEOUT_CHECK_INTERVAL);
	peer_set_[index]->last_msg_recv = Scheduler::instance().clock();
}



//==============================================================
//     TIMEOUT
//==============================================================
void BitTorrentApp::timeout() {
	cout << "app_TIMEOUT188" << endl;
	for (unsigned int i=0; i < peer_set_.size(); i++) {
		if ( peer_set_[i]->connected!= -1 && (Scheduler::instance().clock() - peer_set_[i]->last_msg_recv >= TIMEOUT_CHECK_INTERVAL - 1)) {
			cout << "app_189" << endl;
			close_connection(peer_set_[i]->con_id);
			
#ifdef BT_DEBUG
			cout << "[@" << Scheduler::instance().clock() << "] [" << id << "] Connection timeout with " << peer_set_[i]->id << endl;
#endif
			return;
			
		}
	}
}



//==============================================================
//     SEND KEEP ALIVE
//==============================================================
void BitTorrentApp::send_keep_alive() {
	cout << "app_SEND KEEP ALIVE190" << endl;
	for (unsigned int i=0; i < peer_set_.size(); i++) {
		if (peer_set_[i]->connected == 1
		&& Scheduler::instance().clock() - peer_set_[i]->last_msg_sent >= KEEPALIVE_INTERVAL) {
			cout << "app_191" << endl;
			send(createKeepAliveMessage(), peer_set_[i]->con_id);
		}
	}
}





////////////////////////////////////////
// CREATE MESSAGES
////////////////////////////////////////



//==============================================================
//     CREATE KEEP ALIVE MESSAGE
//==============================================================
BitTorrentData  BitTorrentApp::createKeepAliveMessage()
{
	cout << "app_CREATE KEEP ALIVE MESSAGE192" << endl;
    	BitTorrentData msg = BitTorrentData(KEEPALIVE, this->node_);
	return msg;
}



//==============================================================
//     CREATE HANDSHAKE MESSAGE
//==============================================================
BitTorrentData  BitTorrentApp::createHandshakeMessage()
{
	cout << "app_CTRATE HANDSHAKE MESSAGE193" << endl;
    	BitTorrentData msg = BitTorrentData(HANDSHAKE, this->node_);
	msg.peer_id_ = id;
	return msg;
}



//==============================================================
//     CREATE BITFIELD MESSAGE
//==============================================================
BitTorrentData BitTorrentApp::createBitfieldMessage()
{
	cout << "app_CREATE BITFILD MESSAGE194" << endl;
	BitTorrentData msg = BitTorrentData(BITFIELD, this->node_);
	
	msg.chunk_set_ = new int[tracker_->N_C];
	
	for (int i=0; i<tracker_->N_C; i++) {
    		msg.chunk_set_[i] = chunk_set[i]; 
    	}
	
	msg.length_ +=(int) ceil((double) tracker_->N_C/8) ;
	
	return msg;
}



//==============================================================
//     CREATE HAVE MESSAGE
//==============================================================
BitTorrentData BitTorrentApp::createHaveMessage(int chunk_id)
{
	cout << "app_CREATE HAVE MESSAGE195" << endl;
	BitTorrentData msg = BitTorrentData(HAVE, this->node_);
	msg.chunk_index_ = chunk_id;
	return msg;
}



//==============================================================
//     CREATE INTERESTED MESSAGE
//==============================================================
BitTorrentData BitTorrentApp::createInterestedMessage()
{	
	cout << "app_CREATE INTERSTED MESSAGE196" << endl;
	BitTorrentData msg = BitTorrentData(INTERESTED, this->node_);
	return msg;
}



//==============================================================
//     CREATE NOT INTERESTED MESSAGE
//==============================================================
BitTorrentData BitTorrentApp::createNotInterestedMessage()
{	
	cout << "app_CREATE NOT INTERESTED MESSAGE197" << endl;
	BitTorrentData msg = BitTorrentData(NOT_INTERESTED, this->node_);
	return msg;
}



//==============================================================
//     CREATE CHOKE MESSAGE
//==============================================================
BitTorrentData BitTorrentApp::createChokeMessage()
{	
	cout << "app_CREATE CHOKE MESSAGE198" << endl;
	BitTorrentData msg = BitTorrentData(CHOKE, this->node_);
	return msg;
}



//==============================================================
//     CREATE UNCHOKE MESSAGE
//==============================================================
BitTorrentData BitTorrentApp::createUnchokeMessage()
{
	cout << "app_CREATE UNCHOKE MESSAGE199" << endl;
	BitTorrentData msg = BitTorrentData(UNCHOKE, this->node_);
	msg.timestamp =  Scheduler::instance().clock();
	return msg;
}



//==============================================================
//     CREATE REQUEST MESSAGE
//==============================================================
BitTorrentData BitTorrentApp::createRequestMessage(int chunk_id, long length, long begin, double unchoke_time)
{
	cout << "app_CREATE REQUEST MESSAGE200" << endl;
	BitTorrentData msg = BitTorrentData(REQUEST, this->node_);
	msg.req_piece_length_= length;
	msg.req_piece_begin_= begin;
	msg.chunk_index_ = chunk_id;
	msg.timestamp = unchoke_time;
	return msg;
}



//==============================================================
//     CREATE PIECE MESSAGE
//==============================================================
BitTorrentData BitTorrentApp::createPieceMessage(int chunk_id, long length, long begin)
{	
	cout << "app_CREATE PIECE MESSAGE201" << endl;
	BitTorrentData msg = BitTorrentData(PIECE, this->node_);
	msg.req_piece_length_= length;
	msg.req_piece_begin_ = begin;
	msg.length_ += length;
	msg.chunk_index_ = chunk_id;
	
	// update data byte counter
	data_bytes_queued += length;
	
	//calDownload_size(highlight_size, rest_Mbps, length);
	//if(!seed)
	//	highlight_size -= rest_bandwidth;
	return msg;
}



//==============================================================
//    CREATE CANCEL MESSAGE
//==============================================================
BitTorrentData BitTorrentApp::createCancelMessage(int chunk_id, long length, long begin)
{
	cout << "app_CREATE CANCEL MESSAGE202" << endl;
	BitTorrentData msg = BitTorrentData(CANCEL, this->node_);
	msg.req_piece_length_= length;
	msg.req_piece_begin_= begin;
	msg.chunk_index_ = chunk_id;
	return msg;
}




////////////////////////////////////////
// ANSWER MESSAGES
////////////////////////////////////////


//==============================================================
//     HANDLE HANDSHAKE
//==============================================================
void BitTorrentApp::handle_handshake(BitTorrentData bittorrentData, int connectionID) 
{
	cout << "app_HANDLE HANDSHAKE203" << endl;
	int i, is_new;
	peer_list_entry *new_peer;
	
	is_new = 1;
	
	int peer_id =bittorrentData.peer_id_;
			
	// look if remote peer is in the peer set
	for (i=0; i<signed(peer_set_.size()); i++) {
		if (peer_id == peer_set_[i]->id) {
			cout << "app_204" << endl;
			is_new = 0;
			if (peer_set_[i]->connected == 0) {
				cout << "app_205" << endl;
				// received msg is reply to my handshake
				peer_set_[i]->connected = 1;
				peer_set_[i]->opt_unchoke_prob = 3; // new peer has three times higher optimistic unchoke probability
				
				// send bitfield
				// check if any chunks to upload
				if (super_seeding!=1) {
					cout << "app_206" << endl;
					send(createBitfieldMessage(), connectionID);
				} else {	
					cout << "app_207" << endl;
					super_seeding_have(i, connectionID);
				}
			}
			else if (peer_set_[i]->connected == -1)
			{
				cout << "app_208" << endl;
				// new handshake
				if (num_of_cons() < max_open_cons) {
					cout << "app_209" << endl;
					// only reply if peer can open this connection
					peer_set_[i]->connected = 1;
					peer_set_[i]->opt_unchoke_prob=3;
					peer_set_[i]->con_id = connectionID;
					
					send(createHandshakeMessage(),connectionID); 
					
					if (super_seeding!=1) {
						cout << "app_210" << endl;
						// send bitfield
						send(createBitfieldMessage(), connectionID);
					} else {
						cout << "app_211" << endl;
						super_seeding_have(i,connectionID);
					}
				}
			}
		} else if (connectionID == peer_set_[i]->con_id) {
			cout << "app_212" << endl;
			cout << "ERROR: Same con_id for different peers!" << endl;
		}
	}
	
	if (is_new == 1)
	{
		cout << "app_213" << endl;
		// other peer is not in the peer list
		if (num_of_cons() < max_open_cons) {
			cout << "app_214" << endl;
			new_peer = make_new_peer_list_entry(bittorrentData.source_node_);
			new_peer->id = peer_id;
			new_peer->connected=1;
			new_peer->opt_unchoke_prob=3;
			peer_set_.push_back(new_peer);
			peer_set_[i]->con_id = connectionID;
			send(createHandshakeMessage(),connectionID); 
			
			if (super_seeding!=1) {
				cout << "app_215" << endl;
				// send bitfield
				send(createBitfieldMessage(), connectionID);
			} else {
				cout << "app_216" << endl;
				super_seeding_have(i,connectionID);
			}
			
		}
		else {
			cout << "app_217" << endl;
			cout << "[" << id << "] No reply to handshake, too many tcp connections: " << num_of_cons() << "/" << max_open_cons << endl;
		}
	}
	
}



//==============================================================
//     NUM OF CONS
//==============================================================
long BitTorrentApp::num_of_cons() 
{
	cout << "app_NUM OF CONS218" << endl;
	return conList_.size();
}




//==============================================================
//     NUM OF CONS
//==============================================================
long BitTorrentApp::num_of_interested_peers() 
{
	cout << "app_NUM OF CONS219" << endl;
	long num = 0;
	vector<peer_list_entry *>::iterator it;	
	for( it = peer_set_.begin(); it != peer_set_.end(); it++ ) {
		if ((*it)->peer_interested == 1) {
			cout << "app_220" << endl;
			num++;
		}
	}	
	
	return num;
}



//==============================================================
//     HANDLE BITFIELD
//==============================================================
void BitTorrentApp::handle_bitfield(BitTorrentData bittorrentData, int connectionID) 
{
	cout << "app_HANDLE BITFIELD221" << endl;
	bool interest = false;

	int peer_set_index=-1;
	for (unsigned int i=0; i<peer_set_.size(); i++) {
		if (connectionID == peer_set_[i]->con_id) {
			cout << "app_222" << endl;
			peer_set_index=i;
			for (int j=0; j<tracker_->N_C; j++) {
				peer_set_[i]->chunk_set[j]= bittorrentData.chunk_set_[j];
				
				// check interest
				if (peer_set_[i]->chunk_set[j] == 1 && chunk_set[j] == 0) {
					cout << "app_223" << endl;
					interest = true; 
				}
			}
    		}
	}
	
	delete[] bittorrentData.chunk_set_;
	
	if (interest && !peer_set_[peer_set_index]->am_interested) 
	{
		cout << "app_224" << endl;
		send(createInterestedMessage(), connectionID);
		peer_set_[peer_set_index]->am_interested = true;
	}
}



//==============================================================
//     HANDLE REQUEST
//==============================================================
void BitTorrentApp::handle_request(BitTorrentData bittorrentData, int connectionID) {

	cout << "app_HANDLE REQUEST225" << endl;
	for (unsigned int i=0; i<peer_set_.size(); i++) {
		
		if (connectionID == peer_set_[i]->con_id && !peer_set_[i]->am_choking && peer_set_[i]->last_unchoke == bittorrentData.timestamp) {
			cout << "app_226" << endl;
			// How is data sent by BitTorrent?
			//  only one REQUEST is put in the TCP buffer and we wait until data is sent

			if (peer_set_[i]->queue_request) {
				cout << "app_227" << endl;
				request_list_entry *new_request = new request_list_entry;
					
				new_request->chunk_id = bittorrentData.chunk_index_;
					
				new_request->length = bittorrentData.req_piece_length_;
					
				new_request->rid = bittorrentData.req_piece_begin_;
					
				peer_set_[i]->peer_requests.push_back(new_request);
					
			} else {
				cout << "app_228" << endl;
				send(createPieceMessage(bittorrentData.chunk_index_, bittorrentData.req_piece_length_, bittorrentData.req_piece_begin_),peer_set_[i]->con_id);
					
				peer_set_[i]->uploaded_bytes[0] += bittorrentData.req_piece_length_;
					
				peer_set_[i]->queue_request = true;
				
				if (super_seeding == 1 && peer_set_[i]->uploaded_bytes[0] == tracker_->S_C) {
					cout << "app_229" << endl;
					peer_set_[i]->super_seeding_chunk_id = bittorrentData.chunk_index_;
					
					peer_set_[i]->uploaded_bytes[0] = 0;
					
					choke_peer(i);
		
					if (act_cons <= unchokes) {
						cout << "app_230" << endl;
						check_choking();
					}
					
				}
			}			
		}
	}
}
	
	
	
//==============================================================
//     HANDLE PIECE
//==============================================================
void BitTorrentApp::handle_piece(BitTorrentData bittorrentData, int connectionID) {

	// **** 
	int tmp = delayTime_Highlight; // store ex-delayTime_Highlight
	// ****

	cout << "app_HANDLE PIECE231" << endl;
	
	for (unsigned int i=0; i<peer_set_.size(); i++) {
		
		if (connectionID == peer_set_[i]->con_id) {
			cout << "app_232" << endl;
			if (!peer_set_[i]->my_requests.empty() 
			&& bittorrentData.chunk_index_ == peer_set_[i]->my_requests[0]->chunk_id
			&& bittorrentData.req_piece_begin_ == peer_set_[i]->my_requests[0]->rid) {
				cout << "app_233" << endl;
				// update byte counters
				peer_set_[i]->downloaded_bytes[0] += bittorrentData.req_piece_length_;
				
				peer_set_[i]->downloaded_requested_bytes += bittorrentData.req_piece_length_;
						
				// update download counter
				download[bittorrentData.chunk_index_]+= bittorrentData.req_piece_length_;
				
				// update data bytes received counter (only analysis)
				data_bytes_recv += bittorrentData.req_piece_length_;
				
				// ***
				
				//delayTime_Highlight = ceil(Scheduler::instance().clock());
				delayTime_Highlight = Scheduler::instance().clock();
					log_delay();

				
				// piece complete ?
				if (peer_set_[i]->downloaded_requested_bytes >= peer_set_[i]->my_requests[0]->length) {					
					cout << "app_234" << endl;
					// delete request
					delete *peer_set_[i]->my_requests.begin();
					peer_set_[i]->my_requests.erase(peer_set_[i]->my_requests.begin());
					
					// set byte counter to 0
					peer_set_[i]->downloaded_requested_bytes=0;
					
					// request one new piece to fill pipe
					make_request(connectionID, 1);
				}
	
				// chunk complete (keep not full-sized last chunk in mind)?
				if (download[bittorrentData.chunk_index_] >= tracker_->S_C 
				|| (bittorrentData.chunk_index_== tracker_->N_C-1 && download[bittorrentData.chunk_index_] >= tracker_->S_F - (tracker_->N_C-1)*tracker_->S_C) ) {		
					cout << "app_235" << endl;
					chunk_complete(bittorrentData.chunk_index_, connectionID);
				}
			}
			else {
				cout << "app_236" << endl;
 				fprintf(stderr, "Received piece without request\n");
 				cout << "[@" << Scheduler::instance().clock() << "] [" << id << "] received piece without request from " << peer_set_[i]->id << " Chunk id: " <<  bittorrentData.chunk_index_ << "(" << bittorrentData.req_piece_begin_ << ")"<< endl;
			}
		}
	}
}



//==============================================================
//     SET RAND CHUNK SET
//==============================================================
void BitTorrentApp::set_rand_chunk_set() {
	cout << "app_SET RAND CHUNK SET237" << endl;
// sets the chunk set randomly, i.e. number of chunks and chunks are chosen uniform randomly. Firstly, we determine a random number of chunks, then we determine which chunks. Other possibility is to choose one random var between 0 and 2^N_C -1 and use binary representation ,but these numbers get large!

	RNG rand_;
		
	vector<int> rand_set;
	
	for (int i=0; i<tracker_->N_C; i++) {
		
		rand_set.push_back(i);
		
		chunk_set[i] = 0;
	
	}
	
	int rand_num = rand_.uniform(tracker_->N_C);
	int rand_pos;
	for (int i=0; i < rand_num; i++) {
	
		rand_pos = rand_.uniform(int(rand_set.size()));
		
		chunk_set[ rand_set[rand_pos] ] = 1;
		
		rand_set.erase(rand_set.begin()+rand_pos);
	}
	
	rand_set.clear();
		
	if (rand_num > 0) {
		cout << "app_238" << endl;
		first_chunk_selection = FALSE;
	}

#ifdef BT_DEBUG		
	cout << rand_num << endl;
			
	for (int i=0; i<tracker_->N_C; i++) {
		cout << chunk_set[i] << " ";
	}
	cout << endl;
#endif
}



//==============================================================
//     SET RAND CHUNK SET
//==============================================================
void BitTorrentApp::set_rand_chunk_set_N(int N) {

	cout << "app_SET RAND CHUNK SET239" << endl;
// sets the chunk set randomly, i.e. number of chunks and chunks are chosen uniform randomly. Firstly, we determine a random number of chunks, then we determine which chunks. Other possibility is to choose one random var between 0 and 2^N_C -1 and use binary representation ,but these numbers get large!

	RNG rand_;
		
	vector<int> rand_set;
	
	for (int i=0; i<tracker_->N_C; i++) {
		
		rand_set.push_back(i);
		
		chunk_set[i] = 0;
	
	}
	
	if (N > tracker_->N_C) {
		cout << "app_240" << endl;
		cout << "ERROR: set_rand_chunk_set_N: N must be smaller than N_C" << endl;
	}
	
	int rand_num = N;
	int rand_pos;
	for (int i=0; i < rand_num; i++) {
	
		rand_pos = rand_.uniform(int(rand_set.size()));
		
		chunk_set[ rand_set[rand_pos] ] = 1;
	
		rand_set.erase(rand_set.begin()+rand_pos);
		
	}
	
	rand_set.clear();
		
	if (rand_num > 0) {
		cout << "app_241" << endl;
		first_chunk_selection = FALSE;
	}

#ifdef BT_DEBUG		
	cout << rand_num << endl;
			
	for (int i=0; i<tracker_->N_C; i++) {
		cout << chunk_set[i] << " ";
	}
	cout << endl;
#endif
}



//==============================================================
//     HAVE RAREST CHUNK
//==============================================================
void BitTorrentApp::have_rarest_chunk() {
	cout << "app_HAVE RAREST CHUNK242" << endl;
	for (int i=0; i<tracker_->N_C; i++) {
		chunk_set[i] = 0;
	}
	chunk_set[ tracker_->return_rarest_chunk()] = 1;
	
	first_chunk_selection = FALSE;

}



//==============================================================
//     SUPER-SEEDING HAVE
//==============================================================
void BitTorrentApp::super_seeding_have(long peer_index, int cID) {
	cout << "app_SUPER-SEEDING HAVE243" << endl;
	bool all_sent = TRUE;
	
	for (int i=0; i<tracker_->N_C; i++) {
		if (super_seeding_chunk_set[i] == super_seeding_min_count) {
			cout << "app_244" << endl;
			super_seeding_pending_chunks++;
			super_seeding_chunk_set[i]++;
						
			send(createHaveMessage(i), cID);
			
			return;
		}
		
		
		if (super_seeding_chunk_set[i] > -2) {
			cout << "app_245" << endl;
			all_sent = FALSE;
		}
	}
	
	

	// HAVE msgs sent for every chunk once
	if (all_sent == FALSE) {
		cout << "app_246" << endl;
	 	if (super_seeding_pending_chunks <= unchokes) {
			cout << "app_247" << endl;
			super_seeding_min_count++;
				
			super_seeding_have(peer_index, cID);
		}
		return;
	}
	
	

	// HAVE msgs for all chunks are received, switch to normal mode
	super_seeding = 0;

	// inform peers about my true bitfield
	vector<peer_list_entry*>::iterator it;
	for (it = peer_set_.begin(); it != peer_set_.end(); it++) {
		send(createBitfieldMessage(), (*it)->con_id);
	}
}


//==============================================================
//     SUPER-SEEDING CHOKING
//==============================================================
void BitTorrentApp::super_seeding_choking() {

	for (unsigned int i=0; i<peer_set_.size(); i++) {
		
		// if unchokes found leave
		if (act_cons >= unchokes)
		{
			cout << "app_248" << endl;
			return;
		}
		// unchoke a peer which has not received a chunk or uploaded it to others
		if (peer_set_[i]->connected == 1 && peer_set_[i]->peer_interested && peer_set_[i]->am_choking && peer_set_[i]->super_seeding_chunk_id == -1) {		
			cout << "app_249" << endl;
			unchoke_peer(i);	
		} else if (peer_set_[i]->connected == 1 && !peer_set_[i]->peer_interested && peer_set_[i]->super_seeding_chunk_id == -1) {
			cout << "app_250" << endl;
			super_seeding_have(i, peer_set_[i]->con_id);
		}
	}
	
	// when all peers received a chunk and no peer has forwarded it, keep on sending
	
	for (unsigned int i=0; i<peer_set_.size(); i++) {
		
		// if unchokes found leave
		if (act_cons >= unchokes)
		{
			cout << "app_250" << endl;
			return;
		}
		// unchoke a peer which is interested and choked
		if (peer_set_[i]->connected == 1 && peer_set_[i]->peer_interested && peer_set_[i]->am_choking) {		
			cout << "app_251" << endl;
			unchoke_peer(i);	
		}	
	}
}




////////////////////////////////////////
// TIMERS
////////////////////////////////////////


//==============================================================
//     CHOKING TIMER
//==============================================================
void ChokingTimer::expire(Event *) {
	cout << "app_252" << endl;
	app_->check_choking();
}



//==============================================================
//     TRACKER REQUEST tIMER
//==============================================================
void TrackerRequestTimer::expire(Event *) {
	cout << "app_253" << endl;
	app_->tracker_request();
}



//==============================================================
//     LEAVING TIMER
//==============================================================
void LeavingTimer::expire(Event *) {
	cout << "app_254" << endl;
	app_->stop();
}



//==============================================================
//     CONNECTION TIMEOUT
//==============================================================
void ConnectionTimeout::expire(Event *e) {
	
	cout << "app_255" << endl; 
	app_->timeout();
}



//==============================================================
//     KEEP ALIVE TIMER
//==============================================================
void KeepAliveTimer::expire(Event *e) {
	cout << "app_256" << endl;
	app_->send_keep_alive();
}



//==============================================================
//     COMMAND
//==============================================================
int BitTorrentApp::command(int argc, const char*const* argv)
{
//static int peerNu1;

	cout << "app_COMMAND-257" << endl;
	for(int i = 0; i < argc; i++)
			cout << "app COMMAND argv: " << i << " " << argv[i] << endl;
	// Define Tcl-Command to start the Protocol
	if (strcmp(argv[1], "start") == 0)
	{
		//peerNu1++;
		cout << "app_start258 : " << peerNu << endl;
		if(argc == 2)
		{
			cout << "app_259" << endl;
			start();
			return TCL_OK;
		}
		else
		{
			cout << "app_260" << endl;
			fprintf(stderr,"Too many arguments. Usage: <BitTorrentApp> start\n");
			return TCL_ERROR;
		}
	}
	else if (strcmp(argv[1], "stop") == 0)
	{
		cout << "app_stop261" << endl;
		if(argc == 2)
		{
			cout << "app_262" << endl;
			stop();
			return TCL_OK;
		}
		else
		{
			cout << "app_263" << endl;
			fprintf(stderr,"Too many arguments. Usage: <BitTorrentApp> stop\n");
			return TCL_ERROR;
		}
	}
	else if (strcmp(argv[1], "tracefile") == 0)
	{
		cout << "app_tracefile264" << endl;
		if (argc == 3)
		{
			cout << "app_265" << endl;
			strncpy(p2ptrace_file, argv[2], 256);
			return TCL_OK;
		}
		else
		{
			cout << "app_266" << endl;
			fprintf(stderr,"Too many arguments. Usage: <BitTorrentApp> tracefile\n");
			return TCL_ERROR;
		}
	}
	else if (strcmp(argv[1], "rand_chunk_set") == 0)
	{
		cout << "app_rand_chunk_set267" << endl;
		if (argc == 2) {
			cout << "app_268" << endl;
			set_rand_chunk_set();
			return TCL_OK;
		} else {
			cout << "app_269" << endl;
			return TCL_ERROR;
		}
	}
	else if (strcmp(argv[1], "rand_chunk_set_N") == 0)
	{
		cout << "app_rand_chunk_set_N270" << endl;
		if (argc == 3) {
			cout << "app_271" << endl;
			set_rand_chunk_set_N(atoi(argv[2]));
			return TCL_OK;
		} else {
			cout << "app_272" << endl;
			return TCL_ERROR;
		}
	}
	else if (strcmp(argv[1], "have_rarest_chunk") == 0)
	{
		cout << "app_have_rarest_chunk273" << endl;
		if (argc == 2) {
			cout << "app_274" << endl;
			have_rarest_chunk();
			return TCL_OK;
		} else {
			cout << "app_275" << endl;
			return TCL_ERROR;
		}
	}
	
	return TclObject::command(argc,argv);
}






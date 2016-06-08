
#include "bittorrent_app_flowlevel.h"
#include <iostream>


#define MAX(x, y) ((x)>(y) ? (x) : (y))
#define MIN(x, y) ((x)<(y) ? (x) : (y))

static class BitTorrentAppFlowlevelClass : public TclClass
{
public:
	BitTorrentAppFlowlevelClass() : TclClass("BitTorrentApp/Flowlevel") {}
	TclObject* create(int argc, const char*const* argv)
	{
		if (argc != 7)
			return NULL;
		else
			return (new BitTorrentAppFlowlevel(
				atoi(argv[4]), 
				atol(argv[5]), 
				(BitTorrentTrackerFlowlevel *) TclObject::lookup(argv[6]))
			);
	}
} class_bittorrent_app_flowlevel;



//==============================================================
//     CONSTRUCTOR
//==============================================================
BitTorrentAppFlowlevel::BitTorrentAppFlowlevel(int seed_init, long capacity, BitTorrentTrackerFlowlevel *global)
: BitTorrentApp(seed_init, capacity, global)
{	
	tracker_ = global;
	chunk_set = new int[tracker_->N_C];
	download = new long[tracker_->N_C];
	requested = new long[tracker_->N_C];
	
	
	for (int i=0; i<tracker_->N_C; i++) {
		if (seed) {
			chunk_set[i] = 1;
		} else {
			chunk_set[i] = 0;
			download[i] = 0;
			requested[i] = 0;
		}
	}
};



//==============================================================
//     DESTRUCTOR
//==============================================================
BitTorrentAppFlowlevel::~BitTorrentAppFlowlevel() {
};



//==============================================================
//     START
//==============================================================
void BitTorrentAppFlowlevel::start()
{
	start_time = Scheduler::instance().clock();
	last_upload_time = start_time;
	upload_quota = 0.0;
	
	running = true;

	id=tracker_->reg_peer(this);
	
	
#ifdef BT_DEBUG
	cout << "[@" << Scheduler::instance().clock() << "] [" << id << "] enters (BitTorrentAppFlowlevel)" << endl;
#endif

	tracker_request_timer_->resched(0.0);
	
};



//==============================================================
//    STOP
//==============================================================
void BitTorrentAppFlowlevel::stop() 
{


	if (running) {
		
		running = false;

		stop_time = Scheduler::instance().clock();
		
		log_statistics();
		
		// delete me from tracker
		tracker_->del_peer(id);
		
		// cancel all pending (1) timeouts
		if  (tracker_request_timer_->status() == 1) {
			tracker_request_timer_->cancel();
		}
		if (choking_timer_->status() == 1) {
			choking_timer_->cancel();
		}

		// delete old peer_set
		for (unsigned int i=0; i<peer_set_.size(); i++) {

			if (peer_set_[i]->connected ==  1) {				
				tracker_->close_con(id, peer_set_[i]->con_id);

			} 
			
			delete [] peer_set_[i]->chunk_set;
			delete [] peer_set_[i]->uploaded_bytes;
			delete [] peer_set_[i]->downloaded_bytes;
			
			// cancel all pending (1) timeouts
			if  (peer_set_[i]->connection_timeout_->status() == 1) {
				peer_set_[i]->connection_timeout_->cancel();
			}
			if (peer_set_[i]->keep_alive_timer_->status() == 1) {
				peer_set_[i]->keep_alive_timer_->cancel();
			}
			
			delete peer_set_[i]->connection_timeout_;
			delete peer_set_[i]->keep_alive_timer_;
			peer_set_[i]->peer_requests.clear();
			peer_set_[i]->my_requests.clear();
			
			
		}
		
		peer_set_.clear();
		
		UploadQueue.clear();
	}
	
	done = 1;

#ifdef BT_TELL_TCL
	// tell tcl script that i am done
	Tcl& tcl = Tcl::instance();
	tcl.evalf("done");
#endif
}



//==============================================================
//     GET IDS FROM TRACKER
//==============================================================
bool BitTorrentAppFlowlevel::get_ids_from_tracker() {
	
	vector<long> new_set_;
	peer_list_entry *new_peer;
	unsigned int length;
	
	// update counter
	tracker_requests++;
		
	// get ids from tracker
	new_set_ = tracker_->get_peer_set(num_from_tracker);

	if (new_set_.empty() == false) {
		length = new_set_.size();

		// check if new peer id is already in peer set, otherwise add
		for (unsigned int i=0; i<length; i++) {
		
			if (not_in_peer_set(new_set_[i])) {
				new_peer = make_new_peer_list_entry(new_set_[i]);
				peer_set_.push_back(new_peer);
			}
		}
	}
	
	new_set_.clear();
		
	return true;
}



//==============================================================
//     NOT IN PEER SET
//==============================================================
bool BitTorrentAppFlowlevel::not_in_peer_set(int peer_id){
	int i, length;
	
	// check if peer_id is not own id
	if (id == peer_id) {
		return false;
	}

	// check if peer set is empty
	length=peer_set_.size();
	if (length!=0) {
		// check if peer_id is already in the peer set
		for (i=0; i<length; i++) {
			if (peer_set_[i]->id == peer_id) {
				return false;
			}
		}
	}
	return true;
}



//==============================================================
//     MAKE NEW PEER LIST ENTRY
//==============================================================
peer_list_entry* BitTorrentAppFlowlevel::make_new_peer_list_entry(long pid) {
	peer_list_entry *new_peer;

	new_peer = BitTorrentApp::make_new_peer_list_entry(NULL);
	new_peer->id = pid;
	new_peer->con_id = pid;
	
	return new_peer;
}



//==============================================================
//     CHECK CONNECTIONS
//==============================================================
void BitTorrentAppFlowlevel::check_connections() {
	
	for (unsigned int i=0; i<peer_set_.size(); i++) {
		if (peer_set_[i]->connected == -1) {
					
			peer_set_[i]->connected = 0;
			
			send(createHandshakeMessage(), peer_set_[i]->con_id); 
		}
	}
}



//==============================================================
//     SEND
//==============================================================
void BitTorrentAppFlowlevel::send(BitTorrentData tmp, int cID)
{
	
	// update KEEP ALIVE timer
	peer_list_entry * sender = NULL;
	for (unsigned int i=0; i < peer_set_.size(); i++) {
		if (cID == peer_set_[i]->con_id) {
						
			peer_set_[i]->last_msg_sent = Scheduler::instance().clock();
			
			peer_set_[i]->keep_alive_timer_->resched(KEEPALIVE_INTERVAL);
			
			sender = peer_set_[i];
			
			break;
		}
	}
	
	
	// All messages except PIECE are given to receiver directly
	if (tmp.message_id_ != PIECE) {
		tracker_->sendmsg(tmp, id, cID);	
		return;
	}
	
	double delay = 0.0;

	// make new entry for upload queue
	UploadQueueEntry uqe(tmp, delay, id, cID, sender);
	
	UploadQueue.push_back(uqe);
	
	// update total byte counter
	total_bytes_queued += tmp.length_;
};



//==============================================================
//     CHECK CHOKING
//==============================================================
void BitTorrentAppFlowlevel::check_choking() {

	// do upload of PIECE msgs
	if (!UploadQueue.empty()) {
		
		if (super_seeding == 1) {
			// algorithm is run several times at a specific time point
			upload_quota += C * (Scheduler::instance().clock() - last_upload_time);
		} else {
			// preserve the unused bandwidth from last round, which was smaller than a PIECE plus new bandwidth
			upload_quota = MIN(upload_quota, REQUESTED_PIECE_LENGTH) + C * (Scheduler::instance().clock() - last_upload_time);
		}
				
		
		while (upload_quota >= UploadQueue.begin()->data.length_ && !UploadQueue.empty() ) {
			
			// when peer is choked discard data
			if (!(UploadQueue.begin()->sender==NULL) && !(UploadQueue.begin()->sender->am_choking)) {			
			
				UploadQueue.begin()->sender->super_seeding_chunk_id = UploadQueue.begin()->data.chunk_index_;

				// give first element in the queue to the tracker
				tracker_->sendmsg( UploadQueue.begin()->data, UploadQueue.begin()->sender_cid, UploadQueue.begin()->receiver_cid);
	
				upload_quota -= UploadQueue.begin()->data.length_;
				
				if (super_seeding == 1 && UploadQueue.begin()->sender->uploaded_bytes[0] > tracker_->S_C) {					
					
					UploadQueue.begin()->sender->uploaded_bytes[0] = UploadQueue.begin()->sender->uploaded_bytes[0] - tracker_->S_C;
					
					
					for (unsigned int i=0; i<peer_set_.size(); i++) {
					
						if (UploadQueue.begin()->receiver_cid == peer_set_[i]->con_id) {
							choke_peer(i);
						}
						
						break;
					}
				}
			}
			
			// remove first element in the queue
			UploadQueue.erase(UploadQueue.begin());
		}
	}
	
	last_upload_time = Scheduler::instance().clock();
	
	BitTorrentApp::check_choking();
	
	if (super_seeding == 1) {
		if (upload_quota > REQUESTED_PIECE_LENGTH + PIECE_HEADER_LENGTH) {

			check_choking();
			
			return;
			
		} else {
			choking_timer_->resched(choking_interval);
		}
	}
}



//==============================================================
//     HANDLE REQUEST
//==============================================================
void BitTorrentAppFlowlevel::handle_request(BitTorrentData bittorrentData, int connectionID) {

	for (unsigned int i=0; i<peer_set_.size(); i++) {
		
		if (connectionID == peer_set_[i]->con_id && !peer_set_[i]->am_choking && peer_set_[i]->last_unchoke == bittorrentData.timestamp) {
			
			if (bittorrentData.req_piece_length_ > REQUESTED_PIECE_LENGTH) {
				choke_peer(i);
				cout << "ERROR: Request too large" << endl;
			} else {
			
				send(createPieceMessage(bittorrentData.chunk_index_, bittorrentData.req_piece_length_, bittorrentData.req_piece_begin_),peer_set_[i]->con_id);
					
				peer_set_[i]->uploaded_bytes[0] += bittorrentData.req_piece_length_;	
			}
			
			return;
		}
	}
}




//==============================================================
//     CLOSE CON
//==============================================================
void BitTorrentAppFlowlevel::close_con(long pid)
{
	
	// find in peer set and delete
	vector<peer_list_entry *>::iterator it;
	
	for( it = peer_set_.begin(); it != peer_set_.end(); it++ ) {	
		
		if (pid == (*it)->id) {
		
		
			if (!(*it)->my_requests.empty())	{
				// count bytes requested from that peer
				for (unsigned int j=0; j < (*it)->my_requests.size(); j++) {
					if (j==0) {
						requested[(*it)->my_requests[j]->chunk_id] = requested[(*it)->my_requests[j]->chunk_id] - (*it)->my_requests[j]->length + (*it)->downloaded_requested_bytes;
							
					}
					else {
						requested[ (*it)->my_requests[j]->chunk_id] -= (*it)->my_requests[j]->length;
					}
				}
			}
								
			delete [] (*it)->chunk_set;
			delete [] (*it)->uploaded_bytes;
			delete [] (*it)->downloaded_bytes;
			
			// cancel all pending (1) timeouts
			if  ((*it)->connection_timeout_->status() == 1) {
				(*it)->connection_timeout_->cancel();
			}
			if ((*it)->keep_alive_timer_->status() == 1) {
				(*it)->keep_alive_timer_->cancel();
			}
			
			delete (*it)->connection_timeout_;
			delete (*it)->keep_alive_timer_;
			(*it)->peer_requests.clear();
			(*it)->my_requests.clear();
		
			peer_set_.erase(it);
			
			break;
		}
	}
	

	// find in upload queue and delete
	vector<UploadQueueEntry>::iterator it2;
	it2 = UploadQueue.begin();
	
	while ( it2 != UploadQueue.end() ) {
		
		if (pid == it2->receiver_cid) {
			UploadQueue.erase(it2);			
		} else {
			it2++;
		}
	}	
}



//==============================================================
//     TIMEOUT
//==============================================================
void BitTorrentAppFlowlevel::timeout() {
	// No timeout functionality implemented at flow-level
}



//==============================================================
//     NUM OF CONS
//==============================================================
long BitTorrentAppFlowlevel::num_of_cons() 
{
	long num = 0;
	vector<peer_list_entry *>::iterator it;	
	for( it = peer_set_.begin(); it != peer_set_.end(); it++ ) {
		if ((*it)->connected == 1) {
			num++;
		}
	}	
	
	return num;
}


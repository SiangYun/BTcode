
#include "bittorrent_tracker.h"
#include <iostream>
#include <ext/algorithm>

static class BitTorrentTrackerClass : public TclClass
{
public:
	BitTorrentTrackerClass() : TclClass("BitTorrentTracker") {}
	TclObject* create(int argc, const char*const* argv)
	{
		if (argc != 6)
			return NULL;
		else
			return (new BitTorrentTracker(atol(argv[4]), atol(argv[5])));
	}
} class_bittorrent_tracker;



//==============================================================
//     CONSTRUCTOR
//==============================================================
BitTorrentTracker::BitTorrentTracker(long file_size, long chunk_size) {

	// init file vars
	S_F = file_size;
	S_C = chunk_size;
	N_C = (int)ceil((double)S_F/S_C);
	N_P = 0;
	

#ifdef BT_DEBUG
	cout << "S_F= " << S_F << ", S_C= " << S_C << ", N_C= " << N_C << endl;
#endif
}



//==============================================================
//     REG PEER
//==============================================================
void BitTorrentTracker::reg_peer(Node* peer) 
{
	// update peer counter
	N_P++;
	peer_ids_.push_back(peer);
}



//==============================================================
//     DEL PEER
//==============================================================
void BitTorrentTracker::del_peer(Node* peer) 
{
	vector<Node *>::iterator it;

	// delete peer from list
	for( it = peer_ids_.begin(); it != peer_ids_.end(); it++ ) {
		if ((*it)->address() == peer->address()) {
			peer_ids_.erase(it);
			N_P--;
			break;
		}
	}
}



//==============================================================
//     GET PEER SET
//==============================================================
vector<Node*> BitTorrentTracker::get_peer_set(int req_num_of_peers) {
	
	vector<Node *>::iterator it;
	vector<Node *> return_set(req_num_of_peers);

	
	// get length of tracker list
	int length=peer_ids_.size();

	// if list is empty return null pointer
	if (length==0) {
		return return_set;
	}
	
	// if peer list is smaller than requested peer set, return the complete list
	if (length<req_num_of_peers) {
		req_num_of_peers=length;
		return_set.resize(req_num_of_peers);
	}

	it = random_sample_n(peer_ids_.begin(), peer_ids_.end(), return_set.begin(), req_num_of_peers);
	
	random_shuffle(return_set.begin(), return_set.end());
	
	return return_set;
	
}	



//==============================================================
//     RETURN RAREST CHUNK ID
//==============================================================
long BitTorrentTracker::return_rarest_chunk() {
	// implemented for flow level only
	return 0;
}



//==============================================================
//     COMMAND
//==============================================================
int BitTorrentTracker::command(int argc, const char*const* argv) 
{
	
	if(argc == 3) {
		if(strcmp(argv[1], "regPeer") == 0) {
			Node* peer = (Node*)TclObject::lookup(argv[2]);
			if(peer == 0) {
				return TCL_ERROR;
			} else {
				reg_peer(peer);
				return TCL_OK;
			}
		}	
		else if (strcmp(argv[1], "tracefile") == 0)
		{
		
			if (argc == 3)
			{
				strncpy(p2ptrace_file, argv[2], 256);
				strncpy(p2ptrace_file2, argv[2], 256);
				strncpy(p2ptrace_file3, argv[2], 256);
				strncpy(p2ptrace_file4, argv[2], 256);
				strncpy(p2ptrace_file5, argv[2], 256);

				strcat(p2ptrace_file2, "chunk");
				strcat(p2ptrace_file3, "cons");
				strcat(p2ptrace_file4, "complchunk");
				strcat(p2ptrace_file5, "pop");

				return TCL_OK;
			}
			else
			{
				fprintf(stderr,"Too many arguments. Usage: <BitTorrentApp> start\n");
				return TCL_ERROR;
			}
		}
	}
	return TclObject::command(argc,argv);
}


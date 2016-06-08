#ifndef bittorrent_tracker
#define bittorrent_tracker

#include <vector>
#include <fstream>
#include "node.h"
#include "rng.h"
#include "bittorrent_params.h"
#include "bittorrent_app.h"


class BitTorrentApp;

class BitTorrentTracker : public TclObject 
{

public:

	// Constructor
	BitTorrentTracker(long file_size, long chunk_size);

	// get a list of peers
	vector<Node*> get_peer_set(int num);
	
	// Tcl command interpreter
	virtual int command(int argc, const char*const* argv);

	// register a peer
	void reg_peer(Node* peer);
	
	// delete peer from list
	void del_peer(Node* peer);
	
	long	S_F;	// file size [B]
	long	S_C;	// chunk size [B]
	int 	N_C;	// number of chunks
	
	virtual long return_rarest_chunk(); 

protected:

// VARS	
	vector<Node*> peer_ids_;
	
	RNG rand_;
	
	
// analysis:
	
	// total number of peers registered at tracker
	long N_P;
	
	// p2p trace file
	char p2ptrace_file[256];
	char p2ptrace_file2[256];
	char p2ptrace_file3[256];
	char p2ptrace_file4[256];
	char p2ptrace_file5[256];


};

#endif

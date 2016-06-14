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
	BitTorrentTracker(long file_size, long chunk_size, long highlight_size);

	// get a list of peers �o��ϥΪ̲M��
	vector<Node*> get_peer_set(int num);
	
	// Tcl command interpreter TCL �R�O�sĶ�{��
	virtual int command(int argc, const char*const* argv);

	// register a peer �Ȧs�ϥΪ�
	void reg_peer(Node* peer);
	
	// delete peer from list �q�M��R���ϥΪ�
	void del_peer(Node* peer);
	
	long	S_F;	// file size [B]    �ɮפj�p
	long	S_C;	// chunk size [B]   �϶��j�p 
	int 	N_C;	// number of chunks �϶��ƶq
	
	long    S_H;    // highlights size
	int     N_H;    // number of highlights 
	
	virtual long return_rarest_chunk();   //�^�ǵ}���϶�

protected:

// VARS	
	vector<Node*> peer_ids_;  // �ϥΪ�ID�}�C
	
	RNG rand_;                //���;�
	
	
// analysis:
	
	// total number of peers registered at tracker �Ȧs�btracker���`�ϥΪ̼�
	long N_P;
	
	// p2p trace file trace�ɮ�
	char p2ptrace_file[256];
	char p2ptrace_file2[256];
	char p2ptrace_file3[256];
	char p2ptrace_file4[256];
	char p2ptrace_file5[256];


};

#endif

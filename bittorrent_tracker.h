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

	// get a list of peers 得到使用者清單
	vector<Node*> get_peer_set(int num);
	
	// Tcl command interpreter TCL 命令編譯程式
	virtual int command(int argc, const char*const* argv);

	// register a peer 暫存使用者
	void reg_peer(Node* peer);
	
	// delete peer from list 從清單刪除使用者
	void del_peer(Node* peer);
	
	long	S_F;	// file size [B]    檔案大小
	long	S_C;	// chunk size [B]   區塊大小 
	int 	N_C;	// number of chunks 區塊數量
	
	long    S_H;    // highlights size
	int     N_H;    // number of highlights 
	
	virtual long return_rarest_chunk();   //回傳稀有區塊

protected:

// VARS	
	vector<Node*> peer_ids_;  // 使用者ID陣列
	
	RNG rand_;                //產生器
	
	
// analysis:
	
	// total number of peers registered at tracker 暫存在tracker的總使用者數
	long N_P;
	
	// p2p trace file trace檔案
	char p2ptrace_file[256];
	char p2ptrace_file2[256];
	char p2ptrace_file3[256];
	char p2ptrace_file4[256];
	char p2ptrace_file5[256];


};

#endif

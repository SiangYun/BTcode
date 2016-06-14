
#include "bittorrent_tracker.h"
#include <iostream>
#include <ext/algorithm>

static class BitTorrentTrackerClass : public TclClass
{
public:
	BitTorrentTrackerClass() : TclClass("BitTorrentTracker") {}
	TclObject* create(int argc, const char*const* argv)
	{
		if (argc != 7) // 6 -> 7
			return NULL;
		else
			return (new BitTorrentTracker(atol(argv[4]), atol(argv[5]), atol(argv[6]))); // 多atol(argv[6]) 
	}
} class_bittorrent_tracker;



//==============================================================
//     CONSTRUCTOR
//==============================================================
BitTorrentTracker::BitTorrentTracker(long file_size, long chunk_size, long highlight_size) {

	// init file vars 初始化檔案的值
	S_F = file_size;
	S_C = chunk_size;
	N_C = (int)ceil((double)S_F/S_C);
	N_P = 0;

	S_H = highlight_size;

	

#ifdef BT_DEBUG
	cout << "S_F= " << S_F << ", S_C= " << S_C << ", N_C= " << N_C << endl;
#endif
}



//==============================================================
//     REG PEER
//==============================================================
void BitTorrentTracker::reg_peer(Node* peer)  // 暫存器的peer
{
	// update peer counter 更新使用者計數器
	N_P++;
	peer_ids_.push_back(peer);  // 將peer放入peer_ids_
}



//==============================================================
//     DEL PEER
//==============================================================
void BitTorrentTracker::del_peer(Node* peer)  // 刪除peer
{
	vector<Node *>::iterator it;

	// delete peer from list 從清單中刪除peer
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
vector<Node*> BitTorrentTracker::get_peer_set(int req_num_of_peers) {  // 得到peer集合
	
	vector<Node *>::iterator it;
	vector<Node *> return_set(req_num_of_peers);                       // 回傳集合(提出請求的peers數量)

	
	// get length of tracker list 得到tracker的peer清單長度
	int length=peer_ids_.size();

	// if list is empty return null pointer 如果清單沒有資料回傳空指標
	if (length==0) {
		return return_set;
	}
	
	// if peer list is smaller than requested peer set, return the complete list  如果peer清單小於請求peer集合，回傳完整的清單
	if (length<req_num_of_peers) {
		req_num_of_peers=length;              // 請求的peer數量 = peer ID長度 
		return_set.resize(req_num_of_peers);  // 重新調整return_set大小
	}

	it = random_sample_n(peer_ids_.begin(), peer_ids_.end(), return_set.begin(), req_num_of_peers);  // random_sample_n 隨機抽樣(順序) (peer ID第一個元素, peer ID 最後一個元素, 回傳集合的第一個元素ㄝ, 請求的peer數量)
	
	random_shuffle(return_set.begin(), return_set.end()); // random_shuffle 隨機重排 
	
	return return_set;
	
}	



//==============================================================
//     RETURN RAREST CHUNK ID
//==============================================================
long BitTorrentTracker::return_rarest_chunk() {          // 回傳最稀有的區塊
	// implemented for flow level only 僅用於flow level 實作
	return 0;
}



//==============================================================
//     COMMAND
//==============================================================
int BitTorrentTracker::command(int argc, const char*const* argv)   // 命令
{
	
	if(argc == 3) {
		if(strcmp(argv[1], "regPeer") == 0) {                // argv[1] = regPeer
			Node* peer = (Node*)TclObject::lookup(argv[2]);
			if(peer == 0) {
				return TCL_ERROR;
			} else {
				reg_peer(peer);
				return TCL_OK;
			}
		}	
		else if (strcmp(argv[1], "tracefile") == 0)          // argv[1] = tracefile
		{
		
			if (argc == 3)
			{
				strncpy(p2ptrace_file, argv[2], 256);        // 將 argv[2] 複製 256個位元到 p2ptrace_file
				strncpy(p2ptrace_file2, argv[2], 256);
				strncpy(p2ptrace_file3, argv[2], 256);
				strncpy(p2ptrace_file4, argv[2], 256);
				strncpy(p2ptrace_file5, argv[2], 256);

				strcat(p2ptrace_file2, "chunk");             // 將 chunk 接在 p2ptrace_file2 後面 => p2ptrace_file2 chunk
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


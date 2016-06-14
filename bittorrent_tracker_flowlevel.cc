
#include "bittorrent_tracker_flowlevel.h"
#include <iostream>
#include <ext/algorithm>

static class BitTorrentTrackerFlowlevelClass : public TclClass
{
public:
	BitTorrentTrackerFlowlevelClass() : TclClass("BitTorrentTracker/Flowlevel") {}
	TclObject* create(int argc, const char*const* argv)
	{
		if (argc != 6)
			return NULL;
		else
			return (new BitTorrentTrackerFlowlevel(
				atol(argv[4]), 
				atol(argv[5])	
				)
			);
	}
} class_bittorrent_tracker_flowlevel;



//==============================================================
//     CONSTRUCTOR
//==============================================================
BitTorrentTrackerFlowlevel::BitTorrentTrackerFlowlevel(long file_size, long chunk_size, long highlight_size) : BitTorrentTracker(file_size, chunk_size, highlight_size), peer_counter(0)  // 繼承tracker的檔案大小跟區塊大小, 計算使用者
{
}



//==============================================================
//     REG PEER
//==============================================================
long BitTorrentTrackerFlowlevel::reg_peer(BitTorrentAppFlowlevel *p) {	// tracker裡暫存器的peer
 	
	tracker_list_entry *new_peer = new tracker_list_entry;              // (long id, BitTorrentAppFlowlevel *ptr)
 
 	new_peer->id = peer_counter;                                        // peer的編號
 	new_peer->ptr = p;                                                  // BitTorrentAppFlowlevel (新的區塊集合且若為種子時，則區塊集合元素=1)
 	
	peer_ids_.push_back(new_peer);

 	peer_counter++;
	
//cout << new_peer->id << " reg" << endl;
 	
	return new_peer->id;
};



//==============================================================
//     DEL PEER
//==============================================================
void BitTorrentTrackerFlowlevel::del_peer(long id) 
{	
	vector<tracker_list_entry *>::iterator it;
	
	
	for (it = peer_ids_.begin(); it != peer_ids_.end(); it++) {
		if ( (*it)->id == id) {
			peer_ids_.erase(it);
			return;
		}
	}	 	
};



//==============================================================
//     RETURN INDEX
//==============================================================
long BitTorrentTrackerFlowlevel::return_index(long pid) {                  // 回傳索引

	for (unsigned int i=0; i < peer_ids_.size(); i++) {
		if (peer_ids_[i]->id == pid) {
			return i;
		}
	}
	
	return -1;
};



//==============================================================
//     RETURN RAREST CHUNK ID
//==============================================================
long BitTorrentTrackerFlowlevel::return_rarest_chunk() {                  // 回傳最稀有區塊

//cout << "here" << endl;
	
	vector<int> occurrence;
	occurrence.assign(N_C, 0);                                            // 以具有指定值之指定數目的項目取代容器中的所有項目, occurrence的大小為區塊數量,其值皆為0
	
	// count occurrence of a piece                                        // 計算片段的出現
	for (unsigned int i=0; i < peer_ids_.size(); i++) {
		for (int j=0; j<N_C; j++) {
			if (peer_ids_[i]->ptr->chunk_set[j]==1) {
				occurrence[j]++;
			}
		}
	}
	
	
	// init counter for minimal occurrence and set it to a high value 初始計算用於將數量最少的片段擁有最高的值
	int min_occurrence = peer_ids_.size() +1;
	
	vector<int> rarest_pieces;
	
	// iterate pieces  重複片段
	for (unsigned int i=0; i<occurrence.size(); i++) {
	
			
		// if active piece is rarer as min_occurrence, save it and drop old values 如果存活的片段比數量最少的片段稀有，儲存occurrence[i]到rarest_pieces並將舊的min_occurrence值丟棄
		if (occurrence[i] < min_occurrence) {
			rarest_pieces.clear();
			rarest_pieces.push_back(i);
			min_occurrence = occurrence[i];
		}
				
		// if active piece is as rar as min_occurence, save it 如果存活的片段與數量最少的片段一樣稀有，儲存occurrence[i]到rarest_pieces
		else if (occurrence[i] == min_occurrence){
			rarest_pieces.push_back(i);		
		}
	}
	
	
	if (rarest_pieces.size() == 0) {
		return -1;
	} 
	
	// return one of the rarest pieces (randomly) 回傳 1/稀有片段 (隨機的)
	return rarest_pieces[rand_.uniform((int)rarest_pieces.size())];
};




//==============================================================
//     GET PEER SET
//==============================================================
vector<long> BitTorrentTrackerFlowlevel::get_peer_set(int req_num_of_peers) {

	vector<tracker_list_entry *>::iterator it;
	vector<tracker_list_entry *> return_set_tmp(req_num_of_peers);  // 回傳集合暫存(請求的peer數量)
	vector<long> return_set;                                        // 回傳集合陣列
	 
	
	// get length of tracker list 得到tracker清單的長度
	int length=peer_ids_.size();

	// if list is empty return null pointer 如果清單為空，回傳空指標
	if (length==0) {
		return return_set;
	}
	
	// if peer list is smaller than requested peer set, return the complete list 如果 peer清單 < 請求的peer集合，回傳完整的清單
	if (length<req_num_of_peers) {
		req_num_of_peers=length;
		return_set_tmp.resize(req_num_of_peers); // 重新調整陣列大小
	}
	
	
	it = random_sample_n(peer_ids_.begin(), peer_ids_.end(), return_set_tmp.begin(), req_num_of_peers); // 隨機抽樣(順序) (peer ID第一個元素, peer ID 最後一個元素, 回傳集合暫存的第一個元素, 請求的peer數量)
	
	random_shuffle(return_set_tmp.begin(), return_set_tmp.end());     // random_shuffle 隨機重排
	for (int i=0; i<int(return_set_tmp.size()); i++) {                // long -> int
		return_set.push_back(return_set_tmp[i]->id);
	}

// 	vector<long> return_set(req_num_of_peers);
// 	int length, rand_num;	
// 	int i, j;
// 
// // cout << " requests peer set " << peer_ids_.size()<< endl;
// // for (int i=0; i < peer_ids_.size(); i++) {
// // 	cout << peer_ids_[i]->id << endl;
// // }
// // cout << endl;
// 
// 
// 	// get length of tracker list
// 	length=peer_ids_.size();
// 
// 	// if list is empty return null pointer
// 	if (length==0) {
// 		return return_set;
// 	}
// 	
// 	// if peer list is smaller than requested peer set, return the complete list
// 	if (length<req_num_of_peers) {
// 		// peer list at tracker is shorter as requested list. Return all peers in the peer list
// 		req_num_of_peers=length;
// 		return_set.resize(req_num_of_peers);
// 	}
// 	
// 	// compute a random sequence with different entries as positions 
// 	vector <int> left_pos;
// 	for (i=0; i<length; i++) {
// 		left_pos.push_back(i);
// 	}
// 	length = left_pos.size();
// 	
// 	for (i=0; i<req_num_of_peers; i++) {
// 		//rand_num=int((length-i) * (rand() / (RAND_MAX+1.0)));
// 		rand_num=int ((length-i) * rand_.uniform(0,1));
// 		return_set[i] = peer_ids_[left_pos[rand_num]]->id;
// 			
// 		for (j=rand_num; j<length; j++) {
// 			left_pos[j]=left_pos[j+1];
// 		}
// 	}
// 	
// 	// free mem
// 	left_pos.clear();
	
	return return_set;
};



//==============================================================
//     SENDMSG
//==============================================================
void BitTorrentTrackerFlowlevel::sendmsg(BitTorrentData data, long sender, long receiver)  // data, 送端, 收端
{
	long index = return_index(receiver);   // -> *76   收端的索引

	if (index == -1 ) {
		//cout << "ERROR: tracker sendmsg to wrong pid " << sender << " - " << receiver << endl;
		
		// if peer is unknown to tracker tell peer to close the connection  如果peer不知道tracker告訴peer，關閉連線
		close_con(receiver, sender);  // -> *248

		
	} else {
		peer_ids_.at(index)->ptr->handle_recv_msg(data, sender);  // -> app_.cc *884 
	}
}



//==============================================================
//     CLOSE
//==============================================================
void BitTorrentTrackerFlowlevel::close_con(long sender_id, long receiver)
{

//cout << "close con " << sender_id << " " << receiver << endl;
	long index = return_index(receiver);  // -> *76

	if (index == -1 ) {
		cout << "ERROR: tracker close_con with wrong pid " << receiver << endl;
	} else {	
		peer_ids_.at(index)->ptr->close_con(sender_id);
	}
}


//==============================================================
//     TIMEOUT
//==============================================================
//void BitTorrentTrackerFlowlevel::timeout()
//{
//}




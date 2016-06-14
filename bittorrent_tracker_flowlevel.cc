
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
BitTorrentTrackerFlowlevel::BitTorrentTrackerFlowlevel(long file_size, long chunk_size, long highlight_size) : BitTorrentTracker(file_size, chunk_size, highlight_size), peer_counter(0)  // �~��tracker���ɮפj�p��϶��j�p, �p��ϥΪ�
{
}



//==============================================================
//     REG PEER
//==============================================================
long BitTorrentTrackerFlowlevel::reg_peer(BitTorrentAppFlowlevel *p) {	// tracker�̼Ȧs����peer
 	
	tracker_list_entry *new_peer = new tracker_list_entry;              // (long id, BitTorrentAppFlowlevel *ptr)
 
 	new_peer->id = peer_counter;                                        // peer���s��
 	new_peer->ptr = p;                                                  // BitTorrentAppFlowlevel (�s���϶����X�B�Y���ؤl�ɡA�h�϶����X����=1)
 	
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
long BitTorrentTrackerFlowlevel::return_index(long pid) {                  // �^�ǯ���

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
long BitTorrentTrackerFlowlevel::return_rarest_chunk() {                  // �^�ǳ̵}���϶�

//cout << "here" << endl;
	
	vector<int> occurrence;
	occurrence.assign(N_C, 0);                                            // �H�㦳���w�Ȥ����w�ƥت����ب��N�e�������Ҧ�����, occurrence���j�p���϶��ƶq,��ȬҬ�0
	
	// count occurrence of a piece                                        // �p����q���X�{
	for (unsigned int i=0; i < peer_ids_.size(); i++) {
		for (int j=0; j<N_C; j++) {
			if (peer_ids_[i]->ptr->chunk_set[j]==1) {
				occurrence[j]++;
			}
		}
	}
	
	
	// init counter for minimal occurrence and set it to a high value ��l�p��Ω�N�ƶq�̤֪����q�֦��̰�����
	int min_occurrence = peer_ids_.size() +1;
	
	vector<int> rarest_pieces;
	
	// iterate pieces  ���Ƥ��q
	for (unsigned int i=0; i<occurrence.size(); i++) {
	
			
		// if active piece is rarer as min_occurrence, save it and drop old values �p�G�s�������q��ƶq�̤֪����q�}���A�x�soccurrence[i]��rarest_pieces�ñN�ª�min_occurrence�ȥ��
		if (occurrence[i] < min_occurrence) {
			rarest_pieces.clear();
			rarest_pieces.push_back(i);
			min_occurrence = occurrence[i];
		}
				
		// if active piece is as rar as min_occurence, save it �p�G�s�������q�P�ƶq�̤֪����q�@�˵}���A�x�soccurrence[i]��rarest_pieces
		else if (occurrence[i] == min_occurrence){
			rarest_pieces.push_back(i);		
		}
	}
	
	
	if (rarest_pieces.size() == 0) {
		return -1;
	} 
	
	// return one of the rarest pieces (randomly) �^�� 1/�}�����q (�H����)
	return rarest_pieces[rand_.uniform((int)rarest_pieces.size())];
};




//==============================================================
//     GET PEER SET
//==============================================================
vector<long> BitTorrentTrackerFlowlevel::get_peer_set(int req_num_of_peers) {

	vector<tracker_list_entry *>::iterator it;
	vector<tracker_list_entry *> return_set_tmp(req_num_of_peers);  // �^�Ƕ��X�Ȧs(�ШD��peer�ƶq)
	vector<long> return_set;                                        // �^�Ƕ��X�}�C
	 
	
	// get length of tracker list �o��tracker�M�檺����
	int length=peer_ids_.size();

	// if list is empty return null pointer �p�G�M�欰�šA�^�Ǫū���
	if (length==0) {
		return return_set;
	}
	
	// if peer list is smaller than requested peer set, return the complete list �p�G peer�M�� < �ШD��peer���X�A�^�ǧ��㪺�M��
	if (length<req_num_of_peers) {
		req_num_of_peers=length;
		return_set_tmp.resize(req_num_of_peers); // ���s�վ�}�C�j�p
	}
	
	
	it = random_sample_n(peer_ids_.begin(), peer_ids_.end(), return_set_tmp.begin(), req_num_of_peers); // �H�����(����) (peer ID�Ĥ@�Ӥ���, peer ID �̫�@�Ӥ���, �^�Ƕ��X�Ȧs���Ĥ@�Ӥ���, �ШD��peer�ƶq)
	
	random_shuffle(return_set_tmp.begin(), return_set_tmp.end());     // random_shuffle �H������
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
void BitTorrentTrackerFlowlevel::sendmsg(BitTorrentData data, long sender, long receiver)  // data, �e��, ����
{
	long index = return_index(receiver);   // -> *76   ���ݪ�����

	if (index == -1 ) {
		//cout << "ERROR: tracker sendmsg to wrong pid " << sender << " - " << receiver << endl;
		
		// if peer is unknown to tracker tell peer to close the connection  �p�Gpeer�����Dtracker�i�Dpeer�A�����s�u
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




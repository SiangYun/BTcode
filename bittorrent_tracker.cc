
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
			return (new BitTorrentTracker(atol(argv[4]), atol(argv[5]), atol(argv[6]))); // �hatol(argv[6]) 
	}
} class_bittorrent_tracker;



//==============================================================
//     CONSTRUCTOR
//==============================================================
BitTorrentTracker::BitTorrentTracker(long file_size, long chunk_size, long highlight_size) {

	// init file vars ��l���ɮת���
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
void BitTorrentTracker::reg_peer(Node* peer)  // �Ȧs����peer
{
	// update peer counter ��s�ϥΪ̭p�ƾ�
	N_P++;
	peer_ids_.push_back(peer);  // �Npeer��Jpeer_ids_
}



//==============================================================
//     DEL PEER
//==============================================================
void BitTorrentTracker::del_peer(Node* peer)  // �R��peer
{
	vector<Node *>::iterator it;

	// delete peer from list �q�M�椤�R��peer
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
vector<Node*> BitTorrentTracker::get_peer_set(int req_num_of_peers) {  // �o��peer���X
	
	vector<Node *>::iterator it;
	vector<Node *> return_set(req_num_of_peers);                       // �^�Ƕ��X(���X�ШD��peers�ƶq)

	
	// get length of tracker list �o��tracker��peer�M�����
	int length=peer_ids_.size();

	// if list is empty return null pointer �p�G�M��S����Ʀ^�Ǫū���
	if (length==0) {
		return return_set;
	}
	
	// if peer list is smaller than requested peer set, return the complete list  �p�Gpeer�M��p��ШDpeer���X�A�^�ǧ��㪺�M��
	if (length<req_num_of_peers) {
		req_num_of_peers=length;              // �ШD��peer�ƶq = peer ID���� 
		return_set.resize(req_num_of_peers);  // ���s�վ�return_set�j�p
	}

	it = random_sample_n(peer_ids_.begin(), peer_ids_.end(), return_set.begin(), req_num_of_peers);  // random_sample_n �H�����(����) (peer ID�Ĥ@�Ӥ���, peer ID �̫�@�Ӥ���, �^�Ƕ��X���Ĥ@�Ӥ�����, �ШD��peer�ƶq)
	
	random_shuffle(return_set.begin(), return_set.end()); // random_shuffle �H������ 
	
	return return_set;
	
}	



//==============================================================
//     RETURN RAREST CHUNK ID
//==============================================================
long BitTorrentTracker::return_rarest_chunk() {          // �^�ǳ̵}�����϶�
	// implemented for flow level only �ȥΩ�flow level ��@
	return 0;
}



//==============================================================
//     COMMAND
//==============================================================
int BitTorrentTracker::command(int argc, const char*const* argv)   // �R�O
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
				strncpy(p2ptrace_file, argv[2], 256);        // �N argv[2] �ƻs 256�Ӧ줸�� p2ptrace_file
				strncpy(p2ptrace_file2, argv[2], 256);
				strncpy(p2ptrace_file3, argv[2], 256);
				strncpy(p2ptrace_file4, argv[2], 256);
				strncpy(p2ptrace_file5, argv[2], 256);

				strcat(p2ptrace_file2, "chunk");             // �N chunk ���b p2ptrace_file2 �᭱ => p2ptrace_file2 chunk
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


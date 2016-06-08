/* BitTorrent Data Packet Class
   
   This class represents a Bittorent packet. All header
   fields are implemented and can be accessed by the 
   application.
   
   The message_id_ field is used to identify the packet.
   Values are defined as in real BitTorrent -> see defines!
*/ 

#ifndef bittorrent_data_h
#define bittorrent_data_h

#include "node.h"
#include "bittorrent_params.h"

class BitTorrentData {

public:

// General Header Fields
	int source_port_;
	int source_ip_address_;
	Node* source_node_;
	int dest_port_;
	int dest_ip_address_;
	Node* dest_node_;
	int message_id_;
	
	double timestamp;

// BitTorrent Header Fields
	int length_; // length_ is the total length of the BitTorrent Data packet!!!

// HANDSHAKE
	int peer_id_;

// HAVE Payload
	int chunk_index_;
	
// BITFIELD Payload
	int *chunk_set_;
	
// REQUEST + CANCEL
	// chunk_index_
	long req_piece_begin_;	// not double, because onyl complete bytes are sent
	long req_piece_length_;

// PIECE
	// piece-section
	
// Constructor
	BitTorrentData(int type, Node* my_node);
};


#endif

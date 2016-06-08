/* BitTorrent Data Packet Class
 *
 * This class represents a bittorrent packet. All header
 * fields are implemented and can be accessed by the
 * application.
 */

#include "bittorrent_data.h"

BitTorrentData::BitTorrentData(int type, Node* my_node) {
	
	// constructor with given type and own IP address
	
	source_port_ = -1;
	if (my_node != NULL) {
		source_ip_address_ = my_node->address();
	} else {
		source_ip_address_ = -1;
	}
	source_node_ = my_node;
	dest_port_ = -1;
	dest_ip_address_ = -1;
	message_id_ = type;
	timestamp = -1.0;

	// default initializations (mark fields as invalid!)
	chunk_index_ = -1;
	chunk_set_= NULL;
	peer_id_ = -1;
	req_piece_begin_ = -1;
	req_piece_length_ = -1;
	
	switch (type) {
		
		case HANDSHAKE:
			
			// pstrlen + pstr + reserved + info_hash + peer_id
			// 1B + 19B + 8B + 20B +20B
			length_ = 68;	
		
		break;
		
		case CHOKE:
			
			// length + id
			// 4B + 1B
			length_ = CHOKE_HEADER_LENGTH;
			
		break;

		case UNCHOKE:
			
			length_ = 5;
			
		break;

		case INTERESTED:
			
			length_ = 5;
			
		break;

		case NOT_INTERESTED:
			
			length_ = 5;
			
		break;

		case HAVE:
			
			// length + id + piece index
			// 4B + 1B + 4B
			length_ = 9;
			
		break;

		case BITFIELD:
			
			// length + id + 1 Bit for each chunk (+ spare bits at the end (complete to the end of a Byte or 4 Bytes???))
			
			// length_ has to be set to right value
			length_ = 5;

		break;
		
		case REQUEST:
			
			// length + id + index + begin + length
			// 4B + 1B + 4B + 4B + 4B
			length_ = 17;
			
		break;
		
		case PIECE:
		
			// length + id + index + begin + piece
			// 4B + 1B + 4B + 4B + ?
			// length_ has to be set in app to right value
			length_ = PIECE_HEADER_LENGTH;
		
		break;
		
		case CANCEL:
			
			// length + id + begin + length
			length_ = 17;
		
		break;
		
		case KEEPALIVE:
		
			length_ = 4;
			
		break;
	
		
		default:
				
			length_ = -1;  // invalid packet type !
			fprintf(stderr, "BitTorrent App sent invalid packet type! \n");
				
		break;
	}

}
	

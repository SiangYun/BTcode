
// show info
//#define BT_DEBUG

// inform TCL script when peer finishes download
#define BT_TELL_TCL

// Choking algorithm used in the simulation
#define BITTORRENT 0
#define BITTORRENT_WITH_PURE_RAREST_FIRST 1


// rolling average (multiple of CHOKING_INTERVAL)
// In BT: 20s
#define ROLLING_AVERAGE			2

// time interval between two optimistic unchokes (multiple of CHOKING_INTERVAL)
// In BT: 30s
#define OPTIMISTIC_UNCHOKE_INTERVAL	3

// time interval between remote peer has to upload at least one piece to be not snubbed
// In BT: 60s
#define ANTI_SNUBBING			6

// number of missing pieces to switch from normal mode to end-game
#define END_GAME			1

// time to wait before requesting more peers (default = 300)
#define REREQUEST_INTERVAL 300

// time to wait between checking if any connections have timed out (defaults to 300.0)
#define TIMEOUT_CHECK_INTERVAL	300

// number of seconds to pause between sending keepalives (defaults to 120.0)
#define KEEPALIVE_INTERVAL	120

// Time between the tcp close command and the freeing of memory for the connection
#define WAIT_BEFORE_DELETE 100.0

// Length of the requested piece to download (Bytes) 
// In BT:  typical=2^15 (32KB), max = 2^17 (128KB)
// Bram Cohens: Incentives Build Robustness in BitTorrent = 16KB
#define REQUESTED_PIECE_LENGTH	16384



// BitTorrent Message Types
#define CHOKE 		0
#define UNCHOKE 	1
#define INTERESTED 	2
#define NOT_INTERESTED 	3
#define HAVE 		4
#define BITFIELD 	5
#define REQUEST 	6
#define PIECE 		7
#define CANCEL 		8
#define HANDSHAKE 	19
#define KEEPALIVE	20

#define PIECE_HEADER_LENGTH		13
#define CHOKE_HEADER_LENGTH	5

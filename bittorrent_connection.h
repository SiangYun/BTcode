
#ifndef bittorrent_connection_h
#define bittorrent_connection_h

#include "tcp-full.h"
#include "bittorrent_app.h"
#include "bittorrent_params.h"


class ConnectionCloser;

// DataBuf contains a data part of type BitTorrentData and List-handling features
// An object of type DataBuf is generated when a new Message is sent
// It is kept until the Message is completely received
class BitTorrentDataBuf
{
public:
	BitTorrentDataBuf(BitTorrentData *data) : data_(data), next_(NULL) {}
	~BitTorrentDataBuf()
	{
		if (data_ != NULL)
			delete data_;
	}
	BitTorrentData* data() { return data_; }

protected:
	friend class BitTorrentDataBufList;
	BitTorrentData *data_;
	BitTorrentDataBuf *next_;
};

// A FIFO queue for storing all messages sent on one connection
// until they are fully received by the destination
// which then deletes the received DataBuf
class BitTorrentDataBufList
{
public:
	BitTorrentDataBufList() : head_(NULL), tail_(NULL) {}
	~BitTorrentDataBufList();

	void insert(BitTorrentDataBuf *databuf);	// call if new message is sent
	BitTorrentDataBuf* detach(); 		// only pop head (FIFO)

protected:
	BitTorrentDataBuf *head_;
	BitTorrentDataBuf *tail_;
};



// manages all data of the connection including the agents
class BitTorrentConnection
{
public:
	
	BitTorrentConnection(Node *src_node_, BitTorrentApp *dst, BitTorrentApp *src, Agent *dst_tcp, int conID, int dst_conID = -1, BitTorrentConnection *dst_con = NULL);
	
	~BitTorrentConnection();
	
	void RemoteClose();

	void CallCloser();

	inline int conID() { return conID_; }
	inline int dst_conID() { return dst_conID_; }
	inline void set_dst_conID(int cid) { dst_conID_=cid; }
	inline int dst_address() { return dst_address_; }

	Agent *tcp_, *dst_tcp_;
	BitTorrentConnection *dst_con;
	
	// buffer stores BitTorrrentData such that receiver can access the data sent
	BitTorrentDataBufList buf_;  	// holding the buffered data at sender
	BitTorrentDataBuf *curdata_; 	// holding current data at receiver
	int curbytes_;		// holding currently received bytes of curdata
	
private:

	Node *src_node;
	int dst_address_;
	// connection ID is unique identifier for a connection. When tcp recvs a FIN conID=-2
	int conID_, dst_conID_;

	ConnectionCloser *closer;
};


class ConnectionCloser:public TimerHandler
{
protected:
	BitTorrentConnection *con_;

public:
	ConnectionCloser(BitTorrentConnection* con) : TimerHandler(), con_(con) {}
	inline virtual void expire(Event *);

};


#endif

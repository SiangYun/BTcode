
#include "bittorrent_connection.h"
#include <iostream>

BitTorrentDataBufList::~BitTorrentDataBufList()
{
	while (head_ != NULL)
	{
		tail_ = head_;
		head_ = head_->next_;
		delete tail_;
	}
}


void BitTorrentDataBufList::insert(BitTorrentDataBuf *databuf)
{
	if (tail_ == NULL)
		head_ = tail_ = databuf;
	else
	{
		tail_->next_ = databuf;
		tail_ = databuf;
	}
}

BitTorrentDataBuf* BitTorrentDataBufList::detach()
{
	if (head_ == NULL)
		return NULL;
	BitTorrentDataBuf *p = head_;
	head_ = head_->next_;
	if (head_ == NULL)
		tail_ = NULL;
	return p;
}



/*
 * generates new TCP-Agent tcp_send_ and tcp_sink_ including connectionID and
 * binds them to application node.
 * stores tcl-objectname obtained by name() method in send_name and sink_name
 * then connects these with advertised dst_tcp_send and dst_tcp_sink
 * finally sets application dst_ and calls dst_->connect(...)
 * also implicitly creates empty DataBufList
 */
 
  
BitTorrentConnection::BitTorrentConnection(Node *src_node_, BitTorrentApp *dst, BitTorrentApp *src, Agent *dst_tcp, int conID, int dst_conID, BitTorrentConnection *dst_con_) : curdata_(NULL), curbytes_(0), closer(NULL)
{
	conID_ = conID;
	dst_conID_ = dst_conID;
	dst_con = dst_con_;
	dst_address_ = dst->node_->address();
	src_node = src_node_;
	Tcl& tcl = Tcl::instance();
	
	tcl.eval("new Agent/TCP/FullTcp/Newreno");
	tcp_ = (Agent *) TclObject::lookup(tcl.result());
	tcp_->attachApp(src, conID);
	
	tcl.evalf("%s attach %s",src->node_->name(), tcp_->name());
	
	dst_tcp_ = dst_tcp;
}



BitTorrentConnection::~BitTorrentConnection()
{	
	Tcl& tcl = Tcl::instance();

	if (tcp_ != NULL) {
			
		tcl.evalf("[Simulator instance] detach-agent %s %s", src_node->name(), tcp_->name());
		tcl.evalf("delete %s", tcp_->name());
	}
	
	if (dst_con !=NULL) {
		dst_con->RemoteClose();
	}
}


void BitTorrentConnection::RemoteClose() {
	dst_con = NULL;
}


void BitTorrentConnection::CallCloser() {

	tcp_->attachApp(NULL, -1);
	
	closer = new ConnectionCloser(this);
 	closer->resched(WAIT_BEFORE_DELETE);
}



void ConnectionCloser::expire(Event *e) {

	delete con_;	
	delete this;
	
}


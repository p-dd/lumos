#ifndef TALK_TO_SERVER_H
#define TALK_TO_SERVER_H

#include "../../lumos/DTO.h"
#include "../../lumos/talk_to_db.h"
#include "../../tester/tester.h"
#include "../../lumos/declaration.h"

class talk_to_svr : public boost::enable_shared_from_this<talk_to_svr>, boost::noncopyable
{
	typedef talk_to_svr self_type;
	talk_to_svr () : sock_ (service), started_ (true), timer_ (service), timer_cls (service) {}
	void start (boost::asio::ip::tcp::endpoint ep);

public:
	typedef boost::system::error_code error_code;
	typedef boost::shared_ptr<talk_to_svr> ptr;

	static ptr new_ (boost::asio::ip::tcp::endpoint ep)
	{
		ptr new_talk (new talk_to_svr ());
		new_talk->start (ep);
		return new_talk;
	}

	void stop ();

	bool started () { return started_; }
private:
	void on_connect (const error_code & err);

	void on_read (const error_code & err, size_t bytes);

	void on_login ();
	
	void on_db_settings (DTO & ans);

	//void on_ping (const std::string & msg);
	
	void on_problem (DTO & answer);
		
	void on_task (DTO & ans);

	//void do_ping ();

	//void postpone_ping ();

	void on_write (const error_code & err, size_t bytes, boost::shared_ptr<std::vector<unsigned char>> buf);

	void do_read ();

	void do_cls ();

	void do_write (DTO & msg);

	size_t read_complete (const boost::system::error_code & err, size_t bytes);


private:
	DTO server_query;
	size_t read_size = 0;
	bool read_start = false;

	boost::asio::ip::tcp::socket sock_;
	enum { READ_BUF_LEN = 1024 };
	char read_buffer_[READ_BUF_LEN];
	char write_buffer_[READ_BUF_LEN];
	bool started_;
	std::string username_;
	boost::asio::deadline_timer timer_;
	boost::asio::deadline_timer timer_cls;
	bool ready_client;

};



#endif
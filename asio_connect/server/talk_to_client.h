#ifdef WIN32
#define _WIN32_WINNT 0x0501
#include <stdio.h>
#endif

#ifndef TALK_TO_CLIENT_H
#define TALK_TO_CLIENT_H

//#include <boost/mem_fn.hpp>
//#include <boost/asio.hpp>

#include "../../lumos/DTO.h"
#include "schedule.h"
#include "../../lumos/talk_to_db.h"
#include "../../lumos/declaration.h"
//using namespace boost::asio;
//using namespace boost::posix_time;


class talk_to_client : public boost::enable_shared_from_this<talk_to_client>, boost::noncopyable
{
private:
	boost::asio::ip::tcp::socket sock_;
	enum { READ_BUF_LEN = 1024 };
	char read_buffer_[READ_BUF_LEN];
	char write_buffer_[READ_BUF_LEN];
	DTO client_query;
	size_t read_size = 0;
	bool read_start = false;

	bool started_;
	boost::asio::deadline_timer timer_;
	boost::posix_time::ptime last_ping;
	bool ready_;
	bool ping;
	typedef talk_to_client self_type;
	talk_to_client ();

	
public:
	typedef boost::system::error_code error_code;
	typedef boost::shared_ptr<talk_to_client> ptr;

	void start ();

	static ptr new_ ()
	{
		ptr new_ (new talk_to_client);
		return new_;
	}
	void stop ();

	bool started () const { return started_; }
	bool ready () const { return ready_; }
	void set_ready () { ready_ = true; }
	void set_busy () { ready_ = false; timer_.cancel(); }
	boost::asio::ip::tcp::socket & sock () { return sock_; }
	void do_task (tester::task_ptr & task_);
	void do_upd_problem(tester::id id_prob);
private:
	void on_cls (DTO & query);

	void on_ready (DTO & ans);

	void on_read (const error_code & err, size_t bytes);

	/*void do_ping (const error_code & err);

	void on_check_ping (const error_code & err);

	void postpone_ping ();

	void post_check_ping ();*/

	void on_write (const error_code & err, size_t bytes, boost::shared_ptr<std::vector<unsigned char>> buf);

	void do_read ();

	void do_write (DTO & msg);

	size_t read_complete (const boost::system::error_code & err, size_t bytes);

};

void tcp_server_start ();

#endif
#ifndef TALK_TO_PHP_H
#define TALK_TO_PHP_H

//#include <boost/mem_fn.hpp>
//#include <boost/asio.hpp>

#include "../../lumos/DTO.h"
#include "schedule.h"
#include "../../lumos/all_objects.h"
//#include "../../lumos/talk_to_db.h"
#include "../../lumos/declaration.h"
//using namespace boost::asio;
//using namespace boost::posix_time;

class talk_to_php : public boost::enable_shared_from_this<talk_to_php>, boost::noncopyable
{
private:
	boost::asio::ip::tcp::socket sock_;
	enum { READ_BUF_LEN = 1024 };
	char read_buffer_[READ_BUF_LEN];
	bool started_;
	typedef talk_to_php self_type;
	talk_to_php ();

public:
	typedef boost::system::error_code error_code;
	typedef boost::shared_ptr<talk_to_php> ptr;

	void start ();

	static ptr new_ ()
	{
		ptr new_ (new talk_to_php);
		return new_;
	}
	void stop ();
	bool started () const { return started_; }
	boost::asio::ip::tcp::socket & sock () { return sock_; }
private:	
	void on_read (const error_code & err, size_t bytes);	

	void do_read ();

	size_t read_complete (const boost::system::error_code & err, size_t bytes);

};

void php_server_start ();

#endif
#ifndef TALK_TO_DB_H
#define TALK_TO_DB_H

#include <string>
#include "declaration.h"
//#include "../tester/tester.h"
#include <mysql.h>
//#include "../DTO.h"
#include "all_objects.h"

//class cls {};

class talk_to_db : public boost::enable_shared_from_this<talk_to_db>, boost::noncopyable
{
private:	
	boost::asio::io_service::strand db_strand;
	boost::asio::deadline_timer timer_ping, timer_send;
	bool timer_ready;
	MYSQL * con;
	boost::mutex mtx_con;
	bool started_;
	bool started () const { return started_; }
	typedef talk_to_db self_type;	
public:	
	typedef boost::shared_ptr<talk_to_db> ptr;
	static std::string host, user, password;
	static unsigned long port;
	static bool flag_cls;

	talk_to_db() : db_strand(service), timer_ping(service), timer_send(service) {};
	void start (std::string schema = "lumosdb_2");
	void ping();

	static void settings (const std::string host_, const std::string user_, const std::string password_, const unsigned long port_, const bool flag_cls_ = false);

	std::string binary_to_string (std::string s);
	void on_cls (tester::CLS_ptr cls);
	void on_send();
	//friend talk_to_db & operator >> (talk_to_db & stream, tester::CLS & data);

	tester::problem_ptr get_problem (tester::id index);
	tester::solution_ptr get_solution (tester::id index);
	tester::compiler_ptr get_compiler (tester::id index);
	void ins_cls (tester::CLS_ptr & cls);
	void upd_solution (tester::id index, tester::binary_ptr bin_);
};

extern talk_to_db::ptr db;

#endif
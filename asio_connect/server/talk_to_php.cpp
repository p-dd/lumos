
#include "talk_to_php.h"

typedef boost::shared_ptr<talk_to_php> php_ptr;
typedef std::vector<php_ptr> phps;
phps all_phps;

talk_to_php::talk_to_php () : sock_ (service) {}

void talk_to_php::start ()
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	started_ = true;
	all_phps.push_back (shared_from_this ());	
	do_read ();
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}

void talk_to_php::stop ()
{	
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	if (!started_) return;
	started_ = false;
	sock_.close ();
	std::cout << "stop: sock closed" << std::endl;
	ptr self = shared_from_this ();
	phps::iterator it = std::find (all_phps.begin (), all_phps.end (), self);
	all_phps.erase (it);
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}

void talk_to_php::on_read (const error_code & err, size_t bytes)
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	if (err) stop ();
	if (!started ()) return;
	tester::id id_sol = *((unsigned long *)read_buffer_);
	tester::id id_prob = *(((unsigned long *)read_buffer_) + 1);
	std::cout << "solution: " << id_sol << "problem: " << id_prob << std::endl;
	tester::status_ptr status (new tester::status (tester::task_ptr (new tester::task ({ id_sol, id_prob, tester::all_prob[id_prob]->id_begin_test (), tester::all_prob[id_prob]->id_end_test () }))));
	std::cout << "all_statuses[id_sol] = status" << std::endl;
	tester::all_statuses[id_sol] = status;
	std::cout << "on_schedule" << std::endl;
	schedule_strand.post (boost::bind (&on_schedule, status, nullptr));
	do_read ();
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}

void talk_to_php::do_read ()
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	async_read (sock_, boost::asio::buffer (read_buffer_), MEM_FN2 (read_complete, _1, _2), MEM_FN2 (on_read, _1, _2));		
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}

size_t talk_to_php::read_complete (const boost::system::error_code & err, size_t bytes)
{
	if (err) return 0;
	if (bytes == 0) return 2 * sizeof (unsigned long);
	return 0;
}

boost::asio::ip::tcp::acceptor php_acceptor (service, boost::asio::ip::tcp::endpoint (boost::asio::ip::tcp::v4 (), 8010));

void php_handle_accept (talk_to_php::ptr & client, const boost::system::error_code & err)
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	client->start ();
	talk_to_php::ptr new_client = talk_to_php::new_ ();
	php_acceptor.async_accept (new_client->sock (), boost::bind (php_handle_accept, new_client, _1));
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}

void php_server_start ()
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	talk_to_php::ptr client = talk_to_php::new_ ();
	php_acceptor.async_accept (client->sock (), boost::bind (php_handle_accept, client, _1));	
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}
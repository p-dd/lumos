/*#ifdef WIN32
#define _WIN32_WINNT 0x0501
#include <stdio.h>
#endif*/



/*#include "tmp.cpp"
#include <boost/bind.hpp>
#include <boost/mem_fn.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>*/
#include "talk_to_client.h"
/*using namespace boost::asio;
using namespace boost::posix_time;*/



//clients all_clients;

talk_to_client::talk_to_client () : sock_ (service), started_ (false), timer_ (service), ping(false) {}

void talk_to_client::do_task (tester::task_ptr & task_)
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	if (!started ()) return;
	//if (!ping) service.post (MEM_FN1(do_task, task_));
	DTO msg;
	msg << TASK << *task_;
	do_write (msg);
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}

void talk_to_client::do_upd_problem(tester::id id_prob)
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	if (!started()) return;
	DTO msg;
	msg << PROBLEM << id_prob;
	do_write(msg);
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}

void talk_to_client::start ()
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	started_ = true;
	all_clients.push_back (shared_from_this ());
	std::cout << "2use count: " << all_clients[0].use_count () << std::endl;
	//last_ping = boost::posix_time::microsec_clock::local_time ();
	// first, we wait for client to login
	do_read ();
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}

void talk_to_client::stop ()
{
	std::cout << "stop: {" << std::endl;
	if (!started_) return;

	started_ = false;
	sock_.close ();
	std::cout << "stop: sock closed" << std::endl;
	ptr self = shared_from_this ();
	clients::iterator it = std::find (all_clients.begin (), all_clients.end (), self);
	all_clients.erase (it);
	std::cout << "stop: }" << std::endl;
}


void talk_to_client::on_cls (DTO & query)
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	tester::CLS_ptr cls(new tester::CLS (query));		
	std::cout << "on_cls get_verdict: " << cls->get_id_sol() << " " << cls->get_id_test() << " " << cls->get_verdict() << std::endl;
	db->ins_cls (cls);
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}
/*
void talk_to_client::on_ready (DTO & ans)
{	
	tester::id id_sol;
	ans >> id_sol;
	if (id_sol) schedule_strand.post (boost::bind (&on_schedule, tester::all_statuses[id_sol], shared_from_this ()));
	else schedule_strand.post (boost::bind (&on_schedule, nullptr, shared_from_this ()));
}*/

void talk_to_client::on_ready (DTO & ans)
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	tester::id id_sol;
	ans >> id_sol;
	//tester::status_ptr status_ = tester::all_statuses[id_sol];
	if (id_sol) {
		if (tester::all_statuses[id_sol]->is_empty ()) {
			tester::CLS_ptr cls (new tester::CLS (id_sol, 1 << 30));
			cls->set_verdict (tester::CLS::AC);
			db->ins_cls (cls);
			schedule_strand.post (boost::bind (&on_schedule, nullptr, shared_from_this ()));
		} else {
			schedule_strand.post (boost::bind (&on_schedule, tester::all_statuses[id_sol], shared_from_this ()));			
		}
	} else {
		schedule_strand.post (boost::bind (&on_schedule, nullptr, shared_from_this ()));
	}
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}

void talk_to_client::on_read (const error_code & err, size_t bytes)
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	if (err) {
		printf("I'm error: %d %s bytes: %d\n", err, err.message().c_str(), bytes); fflush(stdout);
		stop ();
	}
	if (read_size) {
		async_read(sock_, boost::asio::buffer(read_buffer_), MEM_FN2(read_complete, _1, _2), MEM_FN2(on_read, _1, _2));
		std::cout << "finish 0: " << __FUNCSIG__ << std::endl;
		return;
	}
	if (!started ()) return;
	action act;
	client_query >> act;
	std::cout << "action: " << act << std::endl;
	DTO ans;
	std::cout << "use count: " << all_clients[0].use_count () << std::endl;
	switch (act) {
		case HELLO: ans << HELLO; break;
		case CLS: ans << CLS; on_cls (client_query); break;
		//case PING: last_ping = boost::posix_time::microsec_clock::local_time (); ping = true; postpone_ping ();  break;
		case DB_SETTINGS: ans << DB_SETTINGS << talk_to_db::host << talk_to_db::user << talk_to_db::password << talk_to_db::port; break;
		case READY: /*ping = true; postpone_ping ();*/ on_ready (client_query);  break;
		case PROBLEM: break; // problem load done
			
		case BYE: stop (); break;
	}
	if (ans.size()) do_write (ans);	
	std::cout << "finish 1: " << __FUNCSIG__ << std::endl;
}
/*
void talk_to_client::do_ping (const error_code & err)
{
	ping = false;
	if (err) return;
	std::cout << "do_ping" << std::endl;	
	DTO query; query << PING;
	do_write (query);
	post_check_ping ();
}

void talk_to_client::postpone_ping ()
{
	timer_.expires_from_now (boost::posix_time::millisec (3000));
	timer_.async_wait (MEM_FN1 (do_ping, _1));
}

void talk_to_client::on_check_ping (const error_code & err)
{
	if (!started()) return;
	if (err) return;
	boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time ();
	if ((now - last_ping).total_milliseconds () > 1000) {
		std::cout << "stopping " << sock_.remote_endpoint ().address () << " - no ping in time" << std::endl;
		//stop ();
	}	
}

void talk_to_client::post_check_ping ()
{
	timer_.expires_from_now (boost::posix_time::millisec (1000));
	timer_.async_wait (MEM_FN1 (on_check_ping, _1));
}*/


void talk_to_client::on_write (const error_code & err, size_t bytes, boost::shared_ptr<std::vector<unsigned char>> buf)
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	std::cout << "buffer write, use count: " << buf.use_count() << std::endl;
	do_read ();
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}

void talk_to_client::do_read ()
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	std::cout << "do_read" << std::endl;
	read_start = true;
	client_query.clear();
	async_read(sock_, boost::asio::buffer(read_buffer_), MEM_FN2(read_complete, _1, _2), MEM_FN2(on_read, _1, _2));
	std::cout << "3use count: " << all_clients[0].use_count () << std::endl;
	//post_check_ping ();
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}

void talk_to_client::do_write (DTO & msg)
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	std::cout << "do_write : ";
	for (size_t i = 0; i < msg.data ()->size (); i++) printf ("%d ", (int)((*msg.data ())[i]));
	printf ("\n");
	if (!started ()) return;
	sock_.async_write_some (boost::asio::buffer (*(msg.data ()), msg.data()->size ()), MEM_FN3 (on_write, _1, _2, msg.data ()));
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}

size_t talk_to_client::read_complete (const boost::system::error_code & err, size_t bytes)
{
	printf("read_complete; bytes : %d\n", bytes);
	if (err) {
		printf("read_complete; err: %d\n", err);
		return 0;
	}
	if (bytes == 0) {
		return read_start ? sizeof(size_t) : read_size;
	}
	if (read_start && bytes == sizeof (size_t)) {
		read_start = false;
		size_t sz = (*((size_t *)read_buffer_));
		read_size = sz + sizeof (size_t);
		printf("read_complete; size : %d\n", read_size);
		return sz;
	}
	printf("read_complete; read: ");
	for (size_t i = 0; i < bytes; i++) {
		printf ("%d ", (int)read_buffer_[i]);
	}
	printf ("\n");
	read_size -= bytes;
	client_query.data ()->insert (client_query.data ()->end (), read_buffer_, read_buffer_ + bytes);
	std::cout << "remain bytes: " << read_size << std::endl;
	return 0;
}



boost::asio::ip::tcp::acceptor acceptor (service, boost::asio::ip::tcp::endpoint (boost::asio::ip::tcp::v4 (), 8001));

void handle_accept (talk_to_client::ptr & client, const boost::system::error_code & err)
{
	std::cout << "1use count: " << client.use_count () << std::endl;
	client->start ();
	std::cout << "4use count: " << all_clients[0].use_count () << std::endl;
	talk_to_client::ptr new_client = talk_to_client::new_ ();
	acceptor.async_accept (new_client->sock (), boost::bind (handle_accept, new_client, _1));
}

void tcp_server_start ()
{
	talk_to_client::ptr client = talk_to_client::new_ ();	
	acceptor.async_accept (client->sock (), boost::bind (handle_accept, client, _1));
	service.run ();
}
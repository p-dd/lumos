#include "talk_to_server.h"

void  talk_to_svr::start (boost::asio::ip::tcp::endpoint ep)
{
	sock_.async_connect (ep, MEM_FN1 (on_connect, _1));
	//postpone_ping ();
}


void talk_to_svr::stop ()
{
	if (!started_) return;
	std::cout << "stopping " << username_ << std::endl;
	started_ = false;
	sock_.close ();
}


void talk_to_svr::on_connect (const error_code & err)
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	if (err) stop ();
	DTO query; query << HELLO;
	do_write (query);
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}

void talk_to_svr::on_read (const error_code & err, size_t bytes)
{
	std::cout << "start: " << __FUNCSIG__ << std::endl; 
	if (err) stop();
	if (!started ()) return;
	// process the msg
	action act;
	server_query >> act;
	std::cout << "action: " << act << std::endl;
	switch (act) {
		case HELLO: on_login (); break;
		case CLS: do_cls (); break;
		//case PING: do_ping (); break;
		case DB_SETTINGS: on_db_settings(server_query); break;
		case PROBLEM: on_problem(server_query); break; // problem load done
		case TASK: on_task(server_query); break;
		case BYE: stop (); break;
	}
	//if (ans.size ()) do_write (ans);
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}

void talk_to_svr::do_cls ()
{	
	std::cout << "start: " << __FUNCSIG__ << std::endl; 
	if (ready_client) return;
	for (auto it = tester::all_cls.begin (); it != tester::all_cls.end (); it++) {
		tester::CLS_ptr cls = *it;
		if (cls->is_sent ()) continue;
		std::cout << "do_cls get_verdict: " << cls->get_id_sol() << " " << cls->get_id_test() << " " << cls->get_verdict() << std::endl;
		DTO ans;
		ans << CLS << *cls;
		cls->do_sent ();
		do_write (ans);
		std::cout << "finish 0: " << __FUNCSIG__ << std::endl;
		return;
	}
	if (tester::ready_tester) {
		tester::id id_sol = tester::all_tester->get_id_sol();//tester::all_cls.front() ->get_id_sol();
		tester::all_cls.clear ();
		ready_client = true;
		DTO ans;
		ans << READY << id_sol;
		do_write (ans);
		std::cout << "finish 1: " << __FUNCSIG__ << std::endl;
		return;
	}
	timer_cls.expires_from_now (boost::posix_time::millisec (100));
	timer_cls.async_wait (MEM_FN (do_cls));
	std::cout << "finish 2: " << __FUNCSIG__ << std::endl;
}

void talk_to_svr::on_task (DTO & ans)
{
	std::cout << "start: " << __FUNCSIG__ << std::endl; 
	tester::task task_;
	ans >> task_;
	tester::all_tester->reset (task_);
	ready_client = false;
	tester::ready_tester = false;
	service.post (boost::bind (&tester::tester::start, tester::all_tester));
	timer_cls.expires_from_now (boost::posix_time::millisec (1000));
	timer_cls.async_wait (MEM_FN (do_cls));
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}

void talk_to_svr::on_problem (DTO & ans)
{
	std::cout << "start: " << __FUNCSIG__ << std::endl; 
	tester::id prob_id;
	ans >> prob_id;
	tester::all_prob[prob_id];
	do_read ();
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}


void talk_to_svr::on_login ()
{
	std::cout << "start: " << __FUNCSIG__ << std::endl; 
	DTO query; query << DB_SETTINGS;
	do_write (query);
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}

void talk_to_svr::on_db_settings (DTO & ans)
{
	std::cout << "start: " << __FUNCSIG__ << std::endl; 
	ans >> talk_to_db::host >> talk_to_db::user >> talk_to_db::password >> talk_to_db::port;
	db->start ();
	DTO query; query << READY << tester::id(0);
	do_write (query);
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}
/*
void talk_to_svr::do_ping ()
{	
	std::cout << "do_ping" << std::endl;
	DTO query; query << PING;
	do_write (query);	
}
void talk_to_svr::postpone_ping ()
{
	timer_.expires_from_now (boost::posix_time::millisec (3000));
	timer_.async_wait (MEM_FN (do_ping));
}
*/
void talk_to_svr::on_write (const error_code & err, size_t bytes, boost::shared_ptr<std::vector<unsigned char>> buf)
{
	std::cout << "start: " << __FUNCSIG__ << std::endl; 
	std::cout << "on_write, use count: " << buf.use_count() << std::endl;
	do_read ();
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}

/*void talk_to_svr::do_read ()
{
	std::cout << "start: " << __FUNCSIG__ << std::endl; 
	std::cout << "do_read" << std::endl;
	boost::asio::async_read (sock_, boost::asio::buffer (read_buffer_), MEM_FN2 (read_complete, _1, _2), MEM_FN2 (on_read, _1, _2));
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}*/

void talk_to_svr::do_read()
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	std::cout << "do_read" << std::endl;
	read_start = true;
	server_query.clear();
	async_read(sock_, boost::asio::buffer(read_buffer_), MEM_FN2(read_complete, _1, _2), MEM_FN2(on_read, _1, _2));
	//post_check_ping ();
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}

void talk_to_svr::do_write (DTO & msg)
{
	std::cout << "start: " << __FUNCSIG__ << std::endl; 
	std::cout << "do_write : ";
	for (size_t i = 0; i < msg.data ()->size (); i++) printf ("%d ", (int)((*msg.data ())[i]));
	printf("\n");
	if (!started ()) return;
	//sock_.async_write_some (boost::asio::buffer (*(msg.data ()), msg.data()->size ()), MEM_FN3 (on_write, _1, _2, msg.data (), flag_read));
	boost::asio::async_write (sock_, boost::asio::buffer (*(msg.data ()), msg.data ()->size ()), MEM_FN3 (on_write, _1, _2, msg.data ()));
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}

/*size_t talk_to_svr::read_complete (const boost::system::error_code & err, size_t bytes)
{
	if (err) return 0;
	if (bytes == 0) {
		answer.clear ();
		return sizeof (size_t);
	}
	if (bytes == sizeof (size_t)) return (*((size_t *)read_buffer_));
	answer.data ()->insert (answer.data ()->end (), read_buffer_, read_buffer_ + bytes);
	return 0;
}*/

size_t talk_to_svr::read_complete(const boost::system::error_code & err, size_t bytes)
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
		printf("%d ", (int)read_buffer_[i]);
	}
	printf("\n");
	read_size -= bytes;
	server_query.data()->insert(server_query.data()->end(), read_buffer_, read_buffer_ + bytes);
	std::cout << "remain bytes: " << read_size << std::endl;
	return 0;
}
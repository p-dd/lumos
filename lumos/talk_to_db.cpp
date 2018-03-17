#include "talk_to_db.h"

talk_to_db::ptr db (new talk_to_db);
std::string talk_to_db::host, talk_to_db::user, talk_to_db::password;
unsigned long talk_to_db::port;
bool talk_to_db::flag_cls;

void talk_to_db::settings (const std::string host_, const std::string user_, const std::string password_, const unsigned long port_, const bool flag_cls_)
{
	if (host_.size ()) host = host_;
	if (user_.size ()) user = user_;
	if (password_.size ()) password = password_;
	if (port_) port = port_;
	flag_cls = flag_cls_;
}


void talk_to_db::start (std::string schema)
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	con = mysql_init (NULL);
	if (con == NULL) {
		printf("%s\n", mysql_error(con)); fflush(stdout);
		//std::system ("pause");
		exit (1);
	}
	if (mysql_real_connect (con, host.c_str (), user.c_str (), password.c_str (), schema.c_str (), port, NULL, CLIENT_MULTI_STATEMENTS) == NULL) {
		printf("%s\n", mysql_error(con)); fflush(stdout);
		mysql_close (con);
		//std::system ("pause");
		exit (1);
	}
	started_ = true;
	ping();
	if (flag_cls) on_send ();
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}


void talk_to_db::ping()
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	mtx_con.lock();
	int err = mysql_ping(con);
	if (err) {
		std::string current_time = boost::posix_time::to_iso_string(boost::posix_time::second_clock::local_time());
		printf("%s | %s\n", current_time.c_str(), mysql_error(con)); fflush(stdout);
		mysql_close(con);
		//std::system("pause");
		exit(1);
	}
	mtx_con.unlock();
	timer_ping.expires_from_now(boost::posix_time::minutes(60));// millisec(1000)
	timer_ping.async_wait(db_strand.wrap(MEM_FN(ping)));
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}


void talk_to_db::on_cls (tester::CLS_ptr cls)
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	tester::all_cls.push_back (cls);	
	tester::all_statuses[cls->get_id_sol ()]->update (cls);
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}


/*		on_send old
		std::stringstream sql;
		sql << "INSERT INTO cls(Solutions_idSolution,Tests_idTest,Verdict)VALUES";		
		auto it = tester::all_cls.begin (); 
		sql << "(" << (*it)->get_id_sol () << "," << (*it)->get_id_test () << "," << (size_t)((*it)->get_verdict ()) << ")";		
		for (it++; it != tester::all_cls.end (); it++) 
			sql << ",(" << (*it)->get_id_sol () << "," << (*it)->get_id_test () << "," << (size_t)((*it)->get_verdict ()) << ")";					
		*/

std::string talk_to_db::binary_to_string (std::string s)
{
	char * res = new char[s.size () * 2 + 1];
	size_t len = mysql_real_escape_string (con, res, &s[0], (unsigned long)s.size());
	std::string ans (res, res + len);
	delete []res;
	return ans;
}

void talk_to_db::on_send ()
{
//	std::cout << "start: " << __FUNCSIG__ << std::endl;
	if (!started ()) return;	
	if (tester::all_cls.size()) {
		std::stringstream sql;
		int num = 0;
		for (auto it = tester::all_cls.begin (); it != tester::all_cls.end (); it++) {
			if ((*it)->is_ext ()) {
				sql << "call insext(" << (*it)->get_id_sol () << "," << (*it)->get_id_test () << "," << (size_t)((*it)->get_verdict ()) << ",\"" <<
					binary_to_string((*it)->get_ext ()->get_message ()) << "\",null," << (*it)->get_ext ()->get_run_time () << "," << (*it)->get_ext ()->get_peak_memory () << "," <<
					(*it)->get_ext ()->get_exit_code () << ",null," <<
					((*it)->is_par () ? std::to_string ((*it)->get_par ()->get_num_proc ()) : "null") << "," << ((*it)->is_par () ? std::to_string ((*it)->get_par ()->get_num_thrd ()) : "null") << ");";
			}
			else sql << "call inscls(" << (*it)->get_id_sol () << "," << (*it)->get_id_test () << "," << (size_t)((*it)->get_verdict ()) << ");";
			num++;
		}
		std::vector <tester::id> upd_sols;
		for (auto jt = tester::all_statuses.begin (); jt != tester::all_statuses.end ();) {
			tester::CLS_ptr cls = ((*jt).second)->get_cls ();
			if (!cls->is_sent ()) {
				cls->do_sent ();
				sql << "call updstat(" << jt->first << ");";
				upd_sols.push_back(jt->first);
				num++;
			}
			if (cls->get_id_test () == (1 << 30)) {
				auto kt = jt; jt++;
				tester::all_statuses.erase (kt);
			} else jt++;
		}
		
		std::cout << "I'm send to db str: " << sql.str() << std::endl;
		mtx_con.lock ();
		if (mysql_real_query (con, sql.str ().c_str (), (unsigned long)sql.str().size())) {
			printf ("%s\n", mysql_error (con));
			//system ("pause");
		}
		while (mysql_next_result (con) == 0);
		mtx_con.unlock ();
		for (auto sol_id : upd_sols) {
			tester::lti_update_grade(tester::all_sol[sol_id]);
		}
		tester::all_cls.clear ();
	}
	timer_send.expires_from_now (boost::posix_time::millisec (1000));
	timer_send.async_wait (db_strand.wrap (MEM_FN (on_send)));
//	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}


void talk_to_db::ins_cls (tester::CLS_ptr & cls)
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	auto ptr = shared_from_this ();
	db_strand.post (MEM_FN1 (on_cls, cls));
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}

unsigned long long mysql_bitset_to_long (unsigned char * s, int len)
{
	unsigned long long res = 0;
	for (int i = 0; i < len; i++) res = (res << 8) | s[i];
	return res;
}

tester::problem_ptr talk_to_db::get_problem (tester::id index)
{	
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	mtx_con.lock ();
	std::stringstream sql;
	sql << "call get_tests_problem(" << index << ");";
	if (mysql_query (con, sql.str ().c_str ())) {
		printf("%s\n", mysql_error(con)); fflush(stdout);
		mysql_close (con);
		//std::system ("pause");
		exit (1);
	}
	MYSQL_RES * result;
	result = mysql_store_result (con);
	if (result == NULL) {
		printf("%s\n", mysql_error(con)); fflush(stdout);
		mysql_close (con);
		//std::system ("pause");
		exit (1);
	}
	printf("mysql_store_result done\n"); fflush(stdout);
	MYSQL_ROW row;
	row = mysql_fetch_row (result);
	unsigned long * lengths;
	lengths = mysql_fetch_lengths (result);
	printf("mysql_fetch_row and mysql_fetch_lengths = %u done\n", *lengths); fflush(stdout);
	tester::text input_, output_, checker_code_, before_code_;
	tester::binary_ptr checker_bin_;
	tester::test::fmt input_fmt_, output_fmt_;
	std::wstring line_run_;
	unsigned long long num_proc_(1);
	unsigned long long num_thrd_(1);
	int rule = 0;
	if (row[0]) rule = *row[0];
	if (row[1]) input_  = tester::text (row[1], row[1] + lengths[1]);
	if (row[2]) output_ = tester::text (row[2], row[2] + lengths[2]);
	size_t time_limit_ (atoi (row[3])), memory_limit_ (atoi (row[4]));
	input_fmt_ = tester::test::fmt (*row[5]);
	output_fmt_ = tester::test::fmt (*row[6]);
	if (row[7]) line_run_ = std::wstring (row[7], row[7] + lengths[7]);
	if (row[8]) num_proc_ = mysql_bitset_to_long ((unsigned char *)row[8], lengths[8]);
	if (row[9]) num_thrd_ = mysql_bitset_to_long ((unsigned char *)row[9], lengths[9]);
	if (row[10]) before_code_ = tester::text (row[10], row[10] + lengths[10]);
	
	mysql_free_result (result);
	mysql_next_result (con);
	result = mysql_store_result (con);
	tester::tests_ptr tests(new tester::tests);
	tests->push_back (tester::test (nullptr, nullptr, L""));
	int id_test = 0;
	while (row = mysql_fetch_row (result)) {
		unsigned long * lengths;
		lengths = mysql_fetch_lengths (result);
		printf("I get from db by problem %d test number: %d with length of input: %u and length answer: %u\n", index, id_test++, lengths[0], lengths[1]); fflush(stdout);
		tests->push_back (tester::test (tester::binary_ptr (row[0] ? new tester::binary (row[0], row[0] + lengths[0]) : nullptr),
										tester::binary_ptr (row[1] ? new tester::binary (row[1], row[1] + lengths[1]) : nullptr),
										row[2] ? std::wstring (row[2], row[2] + lengths[2]) : L""));
	}
	mysql_free_result (result);
	mysql_next_result (con);
	result = mysql_store_result (con);
	row = mysql_fetch_row (result);
	tester::id id_ch_ (atoi (row[0]));
	tester::checker::type type_ch = tester::checker::type (*row[1]);
	tester::id sol_id (atoi (row[2]));
	//checker_input_fmt_ = tester::test::fmt (*row[2]);
	//checker_output_fmt_ = tester::test::fmt (*row[3]);
	mysql_free_result (result);
	mysql_next_result (con);
	printf("mysql_next_result\n"); fflush(stdout);
	mtx_con.unlock ();
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
	return tester::problem_ptr (
		new tester::problem (index, input_, output_, input_fmt_, output_fmt_, time_limit_, memory_limit_, 
							before_code_, line_run_, num_proc_, num_thrd_,
							tests, tester::checker_ptr (new tester::checker (id_ch_, type_ch, tester::all_sol[sol_id])))
		);
}

tester::solution_ptr talk_to_db::get_solution (tester::id index)
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	mtx_con.lock ();
	std::stringstream sql;
	sql << "call get_tests_solution(" << index << ");";
	if (mysql_query (con, sql.str ().c_str ())) {
		printf("%s\n", mysql_error(con)); fflush(stdout);
		mysql_close (con);
		//std::system ("pause");
		exit (1);	
	}
	MYSQL_RES * result;
	result = mysql_store_result (con);
	if (result == NULL) {
		printf("%s\n", mysql_error(con)); fflush(stdout);
		mysql_close (con);
		//std::system ("pause");
		exit (1);
	}	
	MYSQL_ROW row;
	row = mysql_fetch_row (result);
	unsigned long * lengths;
	lengths = mysql_fetch_lengths (result);
	tester::id id_problem_(atoi(row[0]));
	tester::id id_cm_ (atoi(row[1]));
	tester::text code_ (row[2], row[2] + lengths[2]);
	tester::binary_ptr bin_ = nullptr;
	if (row[3]) bin_.reset(new tester::binary(row[3], row[3] + lengths[3]));
	bool extcls_ (*row[4] != '0');
	bool parcls_ (*row[5] != '0');
	bool ischecker_ (*row[6] != '0');
	tester::id id_user_(atoi(row[7]));
	tester::id id_course_(atoi(row[8]));

	if (!ischecker_) {
		code_ = tester::all_prob[id_problem_]->get_before_code() + code_;
	}
	mysql_free_result (result);
	mysql_next_result (con);
	mtx_con.unlock ();
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
	return tester::solution_ptr(new tester::solution(index, id_user_, id_problem_, id_course_, code_, bin_, tester::all_cm[id_cm_]/*, tester::all_prob[id_prob_]*/, extcls_, parcls_));
}

tester::compiler_ptr talk_to_db::get_compiler (tester::id index)
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	mtx_con.lock ();
	std::stringstream sql;
	sql << "call get_tests_compiler(" << index << ");";
	if (mysql_query (con, sql.str ().c_str ())) {
		printf("%s\n", mysql_error(con)); fflush(stdout);
		mysql_close (con);
		//std::system ("pause");
		exit (1);
	}
	MYSQL_RES * result;
	result = mysql_store_result (con);
	if (result == NULL) {
		printf("%s\n", mysql_error(con)); fflush(stdout);
		mysql_close (con);
		//std::system ("pause");
		exit (1);
	}
	MYSQL_ROW row;
	row = mysql_fetch_row (result);
	unsigned long * lengths;
	lengths = mysql_fetch_lengths (result);
	tester::text line_compile_ (row[0], row[0] + lengths[0]);
	tester::text enviroment_ (row[1], row[1] + lengths[1]);
	mysql_free_result (result);
	mysql_next_result (con);
	mtx_con.unlock ();
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
	return tester::compiler_ptr (new tester::compiler (index, line_compile_, enviroment_));
}

void talk_to_db::upd_solution (tester::id index, tester::binary_ptr bin_)
{
	std::cout << "start: " << __FUNCSIG__ << std::endl;
	std::stringstream sql;
	MYSQL_STMT *stmt;
	MYSQL_BIND param[2];
	unsigned long len = (unsigned long)bin_->size();
	sql << "call upd_tests_solution(?,?)";
	memset (param, 0, sizeof (param));
	param[0].buffer_type = MYSQL_TYPE_LONG;
	param[0].buffer = (void *)&(index);
	param[1].buffer_type = MYSQL_TYPE_BLOB;
	param[1].length = &len;
	//param[1].buffer = (void *)&((*bin_)[0]);
	param[1].buffer = (void *)bin_->data();
	mtx_con.lock ();
	stmt = mysql_stmt_init (con);
	if (mysql_stmt_prepare(stmt, sql.str().c_str(), (unsigned long)sql.str().size()) != 0) {
		printf ("%s\n", mysql_error (con));
		printf ("Could not prepare statement");
		//system ("pause");
	}
	if (mysql_stmt_bind_param (stmt, param) != 0) {
		printf ("%s\n", mysql_error (con));
		printf ("Could not bind parameters");
		//system ("pause");
	}
	if (mysql_stmt_execute (stmt) != 0) {
		printf ("%s\n", mysql_error (con));
		printf ("Could not execute statement");
		//system ("pause");
	}

	mysql_next_result (con);
	mtx_con.unlock ();
	std::cout << "finish: " << __FUNCSIG__ << std::endl;
}

// Get queue

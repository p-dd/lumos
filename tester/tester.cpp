//#define _CRT_SECURE_NO_WARNINGS
//#include <winsock2.h>
/*#include <windows.h>
#include <queue>
#include <vector>*/

/*#include <boost/thread.hpp>
#include <boost/ref.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>*/
#include "tester.h"

namespace tester {
	tester_ptr all_tester (new tester);
	bool ready_tester;
	fs::path test_dir("D:\\lumos\\");	
	
	// class tester
	/*
	tester::tester (task & task_) : prob (all_prob[task_.id_problem]), sol (all_sol[task_.id_solution]), begin_test (task_.id_begin_test), end_test (task_.id_end_test)
	{
		run_settings.hToken = hToken;
		run_settings.user_name = user_name;
		boost::uuids::uuid dir_uuid = boost::uuids::random_generator ()();		
		WorkDir = test_dir / (boost::lexical_cast<std::string>(dir_uuid));
		run_settings.affinity = 1;
		ready_tester = true;
	}*/
	class result;

	void tester::reset (task & task_)
	{
		prob = all_prob[task_.id_problem];
		sol = all_sol[task_.id_solution];
		begin_test = task_.id_begin_test;
		end_test = task_.id_end_test;
		boost::uuids::uuid dir_uuid = boost::uuids::random_generator ()();		
		work_dir = test_dir / (boost::lexical_cast<std::string>(dir_uuid));
		affinity = (1 << (GetMaximumProcessorCount (ALL_PROCESSOR_GROUPS))) - 1;
		ready_tester = true;
	}

//	LONGLONG time_ = 10000000; // x10e-7 in seconds - 100-nanoseconds
//	unsigned long affinity = 0x1, memory_ = 339000000;



	void tester::test_it (CLS_ptr cls, int num_proc, int num_thrd)
	{
		job job_sol (prob->get_input_fmt (), prob->get_output_fmt (), test::fmt::NO, affinity, prob->get_time_limit (), prob->get_memory_limit (), true, true, true,
					sol->get_path_bin(), sol->get_path_dir(), sol->get_compiler()->get_enviroment());
		if (prob->get_input_fmt () == test::fmt::FILE) prob->get_test (id_test).save_input (test_input);
		if (prob->get_input_fmt () == test::fmt::STREAM) prob->get_test (id_test).save_input (job_sol.get_pipe_stdin());
		job_sol.set_num_proc (num_proc);
		job_sol.set_num_thrd (num_thrd);
		job_sol.set_args(prob->get_test (id_test).get_args ());
		job_sol.set_line_run (prob->get_line_run ());
		
		job_sol.set_cls (cls);
		if (!job_sol.run()) {
			cls->set_verdict(CLS::verdict::DE);
			printf("!job_sol.run()\n"); fflush(stdout);
			return;
		}
		std::cout << "get_verdict 1: " << cls->get_verdict() << std::endl;
		if (cls->is_done ()) return ;
		if (prob->get_output_fmt () == test::fmt::FILE) if (!fs::exists (test_output)) { cls->set_verdict (CLS::verdict::PE); return; }
		if (prob->get_checker ()->get_type () == checker::type::A) {
			job job_checker (test::fmt::NO, test::fmt::NO, test::fmt::NO, affinity, 60 * 10000000, 
							1024 * 1024 * 256, true, false, false,
							prob->get_checker ()->get_solution ()->get_path_bin (), prob->get_checker ()->get_solution ()->get_path_dir (), prob->get_checker ()->get_solution ()->get_compiler ()->get_enviroment ());
			CLS_ptr cls_ch (new CLS(prob->get_checker ()->get_solution ()->get_id(), 0));
			job_checker.set_args(prob->get_test(id_test).get_args());
			job_checker.set_cls (cls_ch);
			if (prob->get_input_fmt () != test::fmt::NO) {
				prob->get_test (id_test).save_input (checking_dir / prob->path_input ());
				
			}
			prob->get_test (id_test).save_answer (checking_dir / prob->path_answer ());
			/*if (prob->get_input_fmt () == test::fmt::STREAM || prob->get_output_fmt() == test::fmt::STREAM) {
				prob->get_test (id_test).save_input (job_checker.get_pipe_stdin ());
				prob->get_test (id_test).save_answer (job_checker.get_pipe_stdin ());
			}*/
			if (prob->get_output_fmt () == test::fmt::FILE) fs::rename (test_output, checking_dir / prob->path_output ());
			
			//if (prob->get_output_fmt () == test::fmt::STREAM) redirect (*job_sol.get_pipe_stdout(), *job_checker.get_pipe_stdin ());
			if (prob->get_output_fmt () == test::fmt::STREAM) {
				binary_ptr tmp (new binary);
				*job_sol.get_pipe_stdout () >> tmp;
				save_file (checking_dir / prob->path_output (), tmp->data(), tmp->size()); // &(*tmp)[0]
			}

			job_checker.run ();
			std::cout << "get_verdict 2: " << cls->get_verdict() << std::endl;
			// Read checker result
			fs::path path_result = checking_dir / "result.txt";
			if (cls_ch->is_done()) {
				cls->set_verdict(CLS::verdict::DE);
				printf("cls_ch->is_done ()\n"); fflush(stdout);
				return;
			}
			if (!fs::exists (path_result)) {
				cls->set_verdict (CLS::verdict::DE);
				printf("!fs::exists (path_result)\n"); fflush(stdout);
				return;
			}
			binary::size_type len = (binary::size_type)fs::file_size (path_result);
			if (len == 0) {
				cls->set_verdict(CLS::verdict::DE);
				printf("len == 0\n"); fflush(stdout);
				return;
			}
			/*std::cout << "get_verdict copy: " << std::endl;
			fs::copy(path_result, ("D:/lumos/time_measure/result/result_" + 
				std::to_string(sol->get_id()) + "_" + 
				std::to_string(id_test) + "_" +
				std::to_string(num_proc) + "_" + std::to_string(num_thrd) +	".txt"));
			std::cout << "open result file: " << std::endl;*/
			fs::ifstream file_bin (path_result, std::ios_base::binary);

			std::cout << "reading result file: " << std::endl;
			enum ext_cls : unsigned char { NO = 1, VERDICT, MESSAGE, TIME, MEMORY, RES_EOF };
			bool eof = false;
			while (!eof && !file_bin.eof()) {
				ext_cls cls_type;
				file_bin.read((char*)&cls_type, sizeof (cls_type));
				switch (cls_type) {
					case ext_cls::VERDICT:{
						CLS::verdict v;
						file_bin.read ((char*)&v, sizeof (v));
						std::cout << "VERDICT = " << v << std::endl;
						cls->set_verdict (v);
						break;
					}
					case ext_cls::MESSAGE:{
						int l;
						text s;
						file_bin.read ((char*)&l, sizeof (l));
						s.resize (l);
						file_bin.read ((char*)&s[0], l);
						if (cls->is_ext ()) cls->get_ext ()->set_message (s);
						break;
					}
					case ext_cls::TIME:{
						long long t = 0;
						file_bin.read ((char*)&t, sizeof (t));
						printf("time from checker: %lld\n", t);
						if (cls->is_ext ()) cls->get_ext ()->set_run_time (t);
						break;
					}
					case ext_cls::RES_EOF:{
						eof = true;
						printf("res_eof\n");
						break;
					}
				}
			}	
			
			file_bin.close ();
			std::cout << "get_verdict 3: " << cls->get_verdict() << std::endl;
			//if (prob->get_output_fmt () == test::fmt::STREAM) *job_checker.get_pipe_stdout () >> result;
			/*cls->set_verdict (CLS::verdict(result->back ()));
			if (cls->is_ext ()) {
				result->pop_back ();
				cls->get_ext ()->set_run_time ();
				cls->get_ext ()->set_message (text (result->begin (), result->end ()));
			}*/
		}
	}

	void tester::test_group()
	{
		CLS_ptr cls;
		unsigned long long num_thrd = prob->get_num_thrd(), num_proc = prob->get_num_proc();
		
		if (id_test < end_test) {
			for (int proc = 0; (num_proc >> proc) > 0; proc++) {
				if (((num_proc >> proc) & 1) == 0) continue;
				for (int thrd = 0; (num_thrd >> thrd) > 0; thrd++) {
					if (((num_thrd >> thrd) & 1) == 0) continue;
					cls.reset(new CLS(sol->get_id(), id_test, sol->is_extcls(), sol->is_parcls()));
					if (sol->is_parcls()) {
						cls->get_par()->set_num_proc(proc + 1);
						cls->get_par()->set_num_thrd(thrd + 1);
					}
					cls->set_state(CLS::NOW);
					all_cls.push_back(cls);
					std::cout << "test_it begin: " << this->get_id_sol() << " " << id_test << " " << thrd << " " << cls->get_verdict() << std::endl;
					test_it(cls, proc + 1, thrd + 1);
					std::cout << "test_it end: " << this->get_id_sol() << " " << id_test << " " << thrd << " " << cls->get_verdict() << std::endl;
				}
			}
			id_test++;
			service.post(MEM_FN(test_group));
		} else {
			finish();
		}
	}

	void tester::finish()
	{
		boost::system::error_code err;
		fs::remove_all(work_dir, err);
		ready_tester = true;
		if (fs::exists(work_dir)) {
			fs::remove_all(work_dir, err);
		}
	}

	void tester::start ()
	{
		std::cout << "start: " << __FUNCSIG__ << std::endl;
		// Check solution
		ready_tester = false;
		CLS_ptr cls (new CLS (sol->get_id (), 0, sol->is_extcls (), sol->is_parcls ()));
		cls->set_state (CLS::NOW);
		testing_dir = work_dir / "tests";		
		test_input = testing_dir / prob->path_input();
		test_output = testing_dir / prob->path_output();
		checking_dir = work_dir / "check";
		//fs::path TestAnswer = work_dir/ "check";

		fs::create_directory (work_dir);
		fs::create_directory (testing_dir);
		fs::create_directory (checking_dir);
		solution::compiled is_compiled = sol->is_compile ();
		if (!sol->save_and_compile (testing_dir)) {
			if (is_compiled == solution::compiled::NO) {
				cls->set_verdict (CLS::CE);
				all_cls.push_back (cls);
			}
			if (cls->is_ext()) cls->get_ext ()->set_message (sol->get_compile_message());
			finish();
		} else {
			if (is_compiled == solution::compiled::NO) {
				cls->set_verdict (CLS::AC);
				all_cls.push_back (cls);
			}
//			cls->set_state (CLS::END);
			prob->get_checker ()->get_solution ()->save_and_compile (checking_dir);
			id_test = begin_test;
			test_group();
		}
		std::cout << "finish: " << __FUNCSIG__ << std::endl;
	}

};
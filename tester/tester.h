#ifndef TESTER_H
#define TESTER_H

//#include <winsock2.h>
//#include <queue>

#include <vector>
//#include "../declaration.h"
/*#include <boost/thread.hpp>
#include <boost/ref.hpp>*/

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp> 
#include "../lumos/all_objects.h"
#include "job.h"

namespace tester {
	class tester : public boost::enable_shared_from_this<tester>, boost::noncopyable
	{
	private:
		typedef tester self_type;
		fs::path work_dir;
		
		boost::shared_ptr<problem>  prob;
		boost::shared_ptr<solution> sol;
		//std::vector <CLS> sol_cls;
		id begin_test, end_test;
		fs::path testing_dir;
		fs::path test_input;
		fs::path test_output;
		fs::path checking_dir;
		unsigned long affinity;
		id id_test;			
		void test_it (CLS_ptr cls, int num_proc, int num_thrd);
		void test_group();
		void finish();
	public:
		//tester (task & task_);
		void reset (task & task_);
		void start ();		
		id get_id_sol () { return sol->get_id (); }
	};
	typedef boost::shared_ptr<tester> tester_ptr;

	extern fs::path TestDir;	
	extern tester_ptr all_tester;
	extern bool ready_tester;
};

#endif
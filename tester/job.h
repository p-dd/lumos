#ifndef JOB_H
#define JOB_H

#include "WinAPI_helpers.h"
#include "proc_info.h"

#include "objects.h"
#include "../lumos/declaration.h"
#include "pipe.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp> 

namespace tester {

	class job
	{
	private:
		test::fmt input_fmt, output_fmt, error_fmt;
		unsigned long affinity;
		LONGLONG time_limit;
		SIZE_T memory_limit;
		bool re_check;
		bool ui_check;
		bool idle_check;
		binary_ptr input, output, error;
		fs::path path_input, path_output, path_error;
		std::wstring args;
		std::wstring line_run;
		int num_proc, num_thrd;
		fs::path path_bin;
		fs::path path_dir;
		CLS_ptr cls;

		PROCESS_INFORMATION pi;
		HANDLE hJob;
		HANDLE hProcess;
		HANDLE hThread;
		static std::wstring user_name;
		static HANDLE hToken;

		ipipe_ptr pipe_stderr, pipe_stdout;
		opipe_ptr pipe_stdin;

		char * enviroment;

		bool create_restricted_job ();
		bool create_sol_process ();
	public:
		job (test::fmt input_, test::fmt output_, test::fmt error_fmt_, unsigned long affinity_, LONGLONG time_limit_, SIZE_T memory_limit_,
			bool re_check_, bool ui_check_, bool idle_check_, fs::path path_bin_, fs::path path_dir_, char * enviroment_);

		bool run ();
		void set_cls (CLS_ptr & cls_) { cls = cls_; };
		
		ipipe_ptr get_pipe_stderr () { return pipe_stderr; }
		ipipe_ptr get_pipe_stdout () { return pipe_stdout; }
		opipe_ptr get_pipe_stdin () { return pipe_stdin; }

		void set_line_run (std::wstring line_run_) { line_run = line_run_; };
		void set_args (std::wstring args_) { args = args_; };
		void set_num_thrd (int num_thrd_) { num_thrd = num_thrd_; };
		void set_num_proc (int num_proc_) { num_proc = num_proc_; };
		static void set_restricted_user ()
		{
			user_name = get_user_name ();
			std::wcout << user_name << std::endl;
			hToken = get_restricted_user_token ();
		}
	};

	//extern HANDLE hToken;
	//extern std::wstring user_name;
};

#endif
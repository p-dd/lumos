#include "WinAPI_helpers.h"
#include "objects.h"
#include <boost/algorithm/string/replace.hpp>
#include "../lumos/talk_to_db.h"

namespace tester {
	void save_file (fs::path & path, const char * data, std::streamsize size)
	{
		fs::ofstream file_bin (path, std::ios_base::binary);
		file_bin.write (data, size);
		file_bin.close ();
	}

	// class compiler
	compiler::compiler (id id_cm_, text line_compile_, text enviroment_) : id_cm (id_cm_), line_compile (line_compile_) {
		std::stringstream ss(enviroment_);
		int len = (int)enviroment_.size () + 2;
		enviroment = new char[len];
		std::fill (enviroment, enviroment + len, 0);
		char * env = enviroment;
		while (ss.getline(env, len)) {
			while (*env) env++;
			env++;
		}
	}

	text compiler::get_line_compile () const
	{
		return line_compile;
	}

	char * compiler::get_enviroment () const
	{
		return enviroment;
	};


	// class solution
	
	solution::solution(id id_sol_, id id_user_, id id_problem_, id id_course_, text & code_, binary_ptr bin_, compiler_ptr sol_compiler_, bool extcls_, bool parcls_, bool ischecker_)
		: id_user(id_user_), id_problem(id_problem_), id_course(id_course_), id_sol(id_sol_), code(code_), sol_compiler(sol_compiler_), extcls(extcls_), parcls(parcls_), ischecker(ischecker_),
		compiled_flag (bin_ == nullptr ? compiled::NO : (bin_->size() == 0 ? compiled::CE : compiled::OK))
	{
		if (!ischecker) {
			code = tester::all_prob[id_problem]->get_before_code() + code;
		}
		(bin_ == nullptr) ? bin.reset (new binary) : bin = bin_;
	}

	/*void solution::run_exe (std::wstring program, std::string workdir)
	{
		SHELLEXECUTEINFO ShExecInfo = { 0 };
		ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
		ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		ShExecInfo.hwnd = NULL;
		ShExecInfo.lpVerb = NULL;
		ShExecInfo.lpFile = program.c_str();
		ShExecInfo.lpParameters = param.c_str ();
		ShExecInfo.lpDirectory = workdir.c_str ();
		ShExecInfo.nShow = SW_HIDE;
		ShExecInfo.hInstApp = NULL;
		ShellExecuteEx (&ShExecInfo);
		WaitForSingleObject (ShExecInfo.hProcess, INFINITE);
		
	}*/
	
	bool solution::save_and_compile (fs::path path_dir_)
	{
		path_dir = path_dir_;
		path_code = path_dir / "sol.cpp";
		path_bin = path_dir / "sol.exe";
		if (bin->size () != 0) {
			save_file (path_bin, bin->data(), bin->size ()); // &(*bin)[0]
			return true;
		}
		if (compiled_flag == compiled::OK) return true;
		if (compiled_flag == compiled::CE) return false;
		save_file (path_code, &code[0], code.size ());
		// Compiling
		//run_exe ("g++", "-o \"" + path_bin.string () + "\" \"" + path_code.string () + "\"", "");
		std::string s (sol_compiler->get_line_compile ());
		std::wstring line_compile (s.begin (), s.end ());
		boost::replace_all (line_compile, L"%sol%", L"\"" + path_code.wstring () + L"\"");
		boost::replace_all (line_compile, L"%bin%", L"\"" + path_bin.wstring () + L"\"");
		boost::replace_all (line_compile, L"%dir%", L"\"" + path_dir.wstring () + L"\"");
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory (&si, sizeof (si));
		si.cb = sizeof (STARTUPINFO);
		ipipe pipe_stdout, pipe_stderr;
		si.hStdOutput = pipe_stdout.get_pipe ();
		si.hStdError = pipe_stderr.get_pipe ();
		si.dwFlags |= STARTF_USESTDHANDLES;
		// CREATE_DEFAULT_ERROR_MODE
		if (!CreateProcess (nullptr, lpwstr (line_compile).str (), nullptr, nullptr, true, CREATE_NO_WINDOW | CREATE_BREAKAWAY_FROM_JOB, nullptr, nullptr, &si, &pi)) {	compiled_flag = compiled::DE; return false; } 
		pipe_stdout.close_handle ();
		pipe_stderr.close_handle ();
		DWORD WaitResult = WaitForSingleObject (pi.hProcess, 10000);
		if (WaitResult != WAIT_OBJECT_0) { 
			std::cout << "Compilation time more 10 sec" << std::endl;
			compiled_flag = compiled::DE; 
			return false; 
		}
		CloseHandle (pi.hThread);
		CloseHandle (pi.hProcess);
		pipe_stderr >> compile_message;
		text tmp;
		pipe_stdout >> tmp;
		std::cout << "Output of compilation:\n" << tmp << std::endl;
		std::cout << "Error of compilation:\n" << compile_message << std::endl;
		compile_message += tmp;
		if (!fs::exists (path_bin)) { compiled_flag = compiled::CE; db->upd_solution (id_sol, bin); return false; }
		bin->resize ((unsigned int)fs::file_size (path_bin));
		fs::ifstream file_bin (path_bin, std::ios_base::binary);
		file_bin.read (bin->data(), bin->size ()); // &(*bin)[0]
		file_bin.close ();
		db->upd_solution (id_sol, bin);
		compiled_flag = compiled::OK;
		return true;
	}

	/*void solution::run (std::string workdir, unsigned long affinity)
	{
#define MAX_IOPENDING 100

		//run_exe (path_bin.string(), param, workdir);
	}*/
	

	// class test	

	/*
	bool test::check (fs::path & path)
	{
		binary_ptr output (new binary ((unsigned int)fs::file_size (path)));
		fs::ifstream file_bin (path, std::ios_base::binary);
		file_bin.read (&(*output)[0], output->size ());
		file_bin.close ();
		
		std::pair<binary::iterator, binary::iterator> dif (std::mismatch (answer->begin (), answer->end (), output->begin ()));
		if (dif.first == answer->end () && dif.second == output->end ()) return true;

		return false;
	}*/

	// class status
	status::status (task_ptr task_) : tasks (task_)
	{
		cls.reset (new CLS (tasks->id_solution, 0));
		block = 1; // std::max((size_t)(task_->id_end_test - task_->id_begin_test) / 2, (size_t)1);
	}
	
	void status::update (CLS_ptr cls_)
	{
		bool f1 = cls->get_verdict() == CLS::AC || cls->get_verdict() == CLS::NO;
		bool f2 = cls_->get_verdict () == CLS::AC || cls_->get_verdict () == CLS::NO;
		bool f3 = cls->get_id_test () <= cls_->get_id_test ();
		if (!(f1 || f2 || f3)) { cls = cls_; return; }
		if ((f1 && f2 && f3)) { cls = cls_; return; } // AC NO
		if (f1 && !f2) { cls = cls_; return; }
		cls_->do_sent ();
		return;
	}
	
	bool status::is_empty () { return tasks->id_begin_test == tasks->id_end_test; }

	task_ptr status::next_task ()
	{
		task_ptr task_ (new task (*tasks));
		task_->id_end_test = std::min (task_->id_begin_test + block, task_->id_end_test);
		tasks->id_begin_test = task_->id_end_test;
		return task_;
	}

};
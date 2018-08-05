#ifndef OBJECTS_H
#define OBJECTS_H

#include "../lumos/declaration.h"

#include <boost/thread.hpp>
#include <boost/ref.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>         
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include "../lumos/DTO.h"
#include "simple_objects.h"
#include "pipe.h"


class talk_to_db;
namespace tester {
	void save_file (fs::path & path, const char * data, std::streamsize size);	

	struct task
	{
		id id_solution, id_problem;
		id id_begin_test, id_end_test;
	};

	typedef boost::shared_ptr<task> task_ptr;


	class compiler 
	{
	private:
		id id_cm;
		text line_compile;
		char * enviroment;
	public:
		compiler (id id_cm_, text line_compile_, text enviroment_);
		text get_line_compile () const;
		char * get_enviroment () const;
	};

	typedef boost::shared_ptr<compiler> compiler_ptr;

	class CLS_par
	{
	private:
		size_t num_proc;
		size_t num_thrd;
	public:
		CLS_par () : num_proc(0), num_thrd(0) {};
		CLS_par (size_t num_proc_, size_t num_thrd_) : num_proc (num_proc_), num_thrd (num_thrd_) {};
		CLS_par (DTO & data) { data >> (*this); };

		void set_num_proc (size_t num_proc_) { num_proc = num_proc_; }
		void set_num_thrd (size_t num_thrd_) { num_thrd = num_thrd_; }

		size_t get_num_thrd () { return num_thrd; }
		size_t get_num_proc () { return num_proc; }

		friend DTO & operator >> (DTO & stream, CLS_par & cls_par) { stream >> cls_par.num_proc >> cls_par.num_thrd; return stream; }
		friend DTO & operator << (DTO & stream, CLS_par  & cls_par) { stream << cls_par.num_proc << cls_par.num_thrd; return stream; }
	};
	typedef boost::shared_ptr<CLS_par> CLS_par_ptr;

	class CLS_ext
	{
	private:
		LONGLONG run_time;
		SIZE_T peak_memory;
		DWORD exit_code;
		text message;
	public:
		CLS_ext () : run_time (0), peak_memory (0), exit_code (0) {};
		CLS_ext (DTO & data) { data >> (*this); };
		void set_run_time (LONGLONG run_time_) { run_time = run_time_; }
		void set_peak_memory (SIZE_T peak_memory_) { peak_memory = peak_memory_; }
		void set_exit_code (DWORD exit_code_) { exit_code = exit_code_; }
		void set_message (text message_) { message = message_; }

		LONGLONG get_run_time () { return run_time; }
		SIZE_T get_peak_memory () { return peak_memory; }
		DWORD get_exit_code () { return exit_code; }
		text get_message () { return message; }

		friend DTO & operator >> (DTO & stream, CLS_ext & cls_ext) { stream >> cls_ext.run_time >> cls_ext.peak_memory >> cls_ext.exit_code >> cls_ext.message; return stream; }
		friend DTO & operator << (DTO & stream, CLS_ext & cls_ext) { stream << cls_ext.run_time << cls_ext.peak_memory << cls_ext.exit_code << cls_ext.message; return stream; }
	};
	typedef boost::shared_ptr<CLS_ext> CLS_ext_ptr;

	class CLS
	{
	public:
		enum verdict : unsigned char { NO = 1, AC, WA, CE, ML, TL, RE, IL, PE, DE };
		enum state : unsigned char { NOT, NOW, END };
	private:
		id id_sol;
		id id_test;
		verdict last_verdict;
		state last_state;
		CLS_ext_ptr ext;
		CLS_par_ptr par;
		bool flag_ext;
		bool flag_par;
		bool sent;
	public:
		id get_id_sol () { return id_sol; }
		id get_id_test () { return id_test; }
		verdict get_verdict () { return last_verdict; }
		CLS_ext_ptr get_ext () { return ext; }
		CLS_par_ptr get_par () { return par; }

		CLS (DTO & data) { data >> (*this); sent = false; }
		CLS (id id_sol_, id id_test_, bool flag_ext_ = false, bool flag_par_ = false) : 
			last_verdict (NO), last_state (NOT), id_sol (id_sol_), id_test (id_test_), sent (false), 
			flag_ext (flag_ext_ || flag_par_), flag_par (flag_par_), par (flag_par_ ? new CLS_par () : nullptr), ext (flag_ext_ || flag_par_ ? new CLS_ext () : nullptr)
		{}

		void set_state (state state_) { last_state = state_; sent = false; }
		void set_verdict (verdict verdict_) { last_verdict = verdict_; last_state = END; sent = false; }
		//void set_ext (CLS_ext_ptr ext_) { ext = ext_; }

		//void do_done () { sent = true; }
		bool is_par () { return flag_par; }
		bool is_ext () { return flag_ext; }
		bool is_done () { return (last_state == END); }
		bool is_sent () { return sent; }
		void do_sent () { sent = true; }

		friend DTO & operator >> (DTO & stream, CLS & cls) { 
			stream >> cls.id_sol >> cls.id_test >> cls.last_verdict >> cls.flag_ext >> cls.flag_par;
			if (cls.flag_ext) cls.ext = CLS_ext_ptr (new CLS_ext (stream)); 
			if (cls.flag_par) cls.par = CLS_par_ptr (new CLS_par (stream));
			return stream;
		}
		friend DTO & operator << (DTO & stream, CLS & cls) { 
			stream << cls.id_sol << cls.id_test << cls.last_verdict << cls.flag_ext << cls.flag_par; 
			if (cls.flag_ext) stream << *cls.ext;
			if (cls.flag_par) stream << *cls.par;
			return stream;
		}
	};

	typedef boost::shared_ptr<CLS> CLS_ptr;
	
	class test
	{
	private:
		binary_ptr input;
		binary_ptr answer;
		std::wstring args;
	public:
		enum fmt { NO = '1', STREAM, FILE};
		test (binary_ptr input_, binary_ptr answer_, std::wstring args_) : input (input_), answer (answer_), args(args_) {}
		void save_input (fs::path & path) { if (input != nullptr) save_file (path, input->data(), input->size ()); } // &(*input)[0]
		void save_answer (fs::path & path) { if (answer != nullptr) save_file (path, answer->data(), answer->size ()); } // &(*answer)[0]
		void save_input (opipe_ptr pipe_) { if (input != nullptr) *pipe_ << input; }
		void save_answer (opipe_ptr pipe_) { if (answer != nullptr) *pipe_ << answer; }
		std::wstring & get_args () { return args; }		
	};

	typedef std::vector<test> tests;
	typedef boost::shared_ptr<tests> tests_ptr;

	class solution
	{
	public:
		enum compiled { NO, CE, OK, DE };
	private:
		id id_sol;
		binary_ptr bin;
		text code;
		text compile_message;
		compiled compiled_flag;
		compiler_ptr sol_compiler;
		fs::path path_dir;
		fs::path path_code;
		fs::path path_bin;
		bool extcls;
		bool parcls;
		bool ischecker;

		id id_user, id_problem, id_course;
	public:
		solution(id id_sol_, id id_user_, id id_problem_, id id_course_, text & code_, binary_ptr bin_, compiler_ptr sol_compiler_, bool extcls_, bool parcls_, bool ischecker_);

		text get_compile_message () { return compile_message; }
		compiler_ptr get_compiler () { return sol_compiler; }
		fs::path & get_path_bin () { return path_bin; }
		fs::path & get_path_dir () { return path_dir; }
		id get_id () { return id_sol; }
		id get_id_user() { return id_user; }
		id get_id_course() { return id_course; }
		id get_id_problem() { return id_problem; }
		bool is_extcls () { return extcls; }
		bool is_parcls () { return parcls; }
		compiled is_compile () { return compiled_flag; }

		bool save_and_compile (fs::path path_dir_);
		~solution () {};
	};

	typedef boost::shared_ptr<solution> solution_ptr;

	class checker
	{
	public:
		enum type { A = 'A' };
	private:
		id id_ch;
		type type_ch;
		solution_ptr check_sol;
	public:
		checker (id id_ch_, type type_ch_, solution_ptr check_sol_) : id_ch (id_ch_), type_ch (type_ch_), check_sol (check_sol_) {};
		type get_type () { return type_ch; }
		solution_ptr get_solution () { return check_sol; }
	};

	typedef boost::shared_ptr<checker> checker_ptr;

	class problem
	{
	private:
		const LONGLONG mul_time = 10000;
		fs::path input;
		fs::path output;
		fs::path answer;
		//bool _stdin, _stdout, _args;
		test::fmt input_fmt, output_fmt;
		tests_ptr tests;
		LONGLONG time_limit; // 1e-7 sec
		size_t memory_limit;
		id id_prob;
		checker_ptr prob_checker;
		std::wstring line_run;
		text before_code;
		unsigned long long num_proc, num_thrd;
		text latest_update;
	public:
		// time_limit_ in milli seconds (1e-3 sec)
		problem(id id_prob_, fs::path input_, fs::path output_, test::fmt input_fmt_, test::fmt output_fmt_, size_t time_limit_, size_t memory_limit_,
			text & before_code_, std::wstring line_run_, unsigned long long num_proc_, unsigned long long num_thrd_, tests_ptr tests_, checker_ptr prob_checker_, text latest_update)
			: id_prob(id_prob_), time_limit(time_limit_ * mul_time), memory_limit(memory_limit_),
			before_code(before_code_), line_run(line_run_), num_proc(num_proc_), num_thrd(num_thrd_),
			input(input_), output(output_), answer("answer.txt"), tests(tests_),
			input_fmt(input_fmt_), output_fmt(output_fmt_), prob_checker(prob_checker_),
			latest_update(latest_update)
		{};

		fs::path & path_input() { return input; }
		fs::path & path_output() { return output; }
		fs::path & path_answer() { return answer; }

		LONGLONG get_time_limit() { return time_limit; }
		size_t get_memory_limit() { return memory_limit; }
		std::wstring & get_line_run() { return line_run; }
		unsigned long long  get_num_proc() { return num_proc; }
		unsigned long long  get_num_thrd() { return num_thrd; }
		test & get_test(id i) { return (*tests)[i]; }
		id id_begin_test() { return 1; }
		id id_end_test() { return tests->size(); }

		test::fmt get_input_fmt() { return input_fmt; }
		test::fmt get_output_fmt() { return output_fmt; }
		checker_ptr get_checker() { return prob_checker; }
		text get_before_code() { return before_code; }

		text get_timestamp() { return latest_update; }

		friend talk_to_db & operator >> (talk_to_db & stream, problem & data);
	};

	typedef boost::shared_ptr<problem> problem_ptr;	

	class status
	{
	private:	
		CLS_ptr cls;
		task_ptr tasks;
		size_t block;
	public:
		status (task_ptr task_);
		void update (CLS_ptr cls_);
		bool is_empty ();
		task_ptr next_task ();
		CLS_ptr get_cls () { return cls; }
	};
	typedef boost::shared_ptr<status> status_ptr;
};

#endif
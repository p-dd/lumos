#ifndef ALL_OBJECTS_H
#define ALL_OBJECTS_H

#include "../tester/objects.h"
#include <queue>

namespace tester {
	class compilers
	{
	private:
		std::map <id, compiler_ptr> all;
	public:
		compiler_ptr operator[] (id index);
	};
	class problems
	{
	private:
		std::map <id, problem_ptr> all;
	public:
		problem_ptr operator[] (id index);

	};

	class solutions
	{
	private:
		std::map <id, solution_ptr> all;
	public:
		solution_ptr operator[] (id index);

	};
	typedef std::map <id, status_ptr> statuses;
	/*class statuses
	{
	public:
		std::map <id, status> all;
		
	};*/
	extern compilers all_cm;
	extern solutions all_sol;
	extern problems all_prob;
	extern statuses all_statuses;
	extern std::vector <CLS_ptr> all_cls;
	void lti_update_grade(solution_ptr sol);

};

#endif
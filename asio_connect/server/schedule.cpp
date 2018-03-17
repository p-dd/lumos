#include "talk_to_client.h"
#include "talk_to_php.h"
#include "schedule.h"


//tasks all_tasks;
tasks assigned_tasks;
clients all_clients;

boost::asio::strand schedule_strand(service);

void on_schedule (tester::status_ptr status_, client_ptr client_)
{
	if (client_ != nullptr) client_->set_ready ();
	if (status_ != nullptr) if (!status_->is_empty()) assigned_tasks.push_back (status_->next_task ());	
	for (auto it = all_clients.begin (); it != all_clients.end () && assigned_tasks.size () > 0;) {
		if (!(*it)->ready ()) { it++; continue; }
		(*it)->set_busy ();
		(*it)->do_task (assigned_tasks.front ());
		it++;
		assigned_tasks.pop_front ();
	}
}


/*
//tester::CLS_ptr cls (new tester::CLS (task_->id_solution, 0));
//std::list <tester::task_ptr> l;
//if ()
//int block = std::max ((task_->id_end_test - task_->id_begin_test) / 5, (tester::id)1);
for (tester::id i = task_->id_begin_test; i < task_->id_end_test; i += block) {
tester::task_ptr task_i (new tester::task);
task_i->id_begin_test = i;
task_i->id_end_test = std::min (i + block, task_->id_end_test);
task_i->id_problem = task_->id_problem;
task_i->id_solution = task_->id_solution;
all_tasks.push_back (task_i);
}

//tester::task_ptr task_a();
//task_a->id_end_test = std::min (task_a->id_begin_test + block, task_->id_end_test);
//tester::all_statuses.set_status (cls, task_a);


//all_tasks.push_back (l.front ()); l.pop_front ();
*/
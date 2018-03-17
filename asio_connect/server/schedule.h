#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <boost/shared_ptr.hpp>
#include <vector>
#include <queue>
#include <list>
#include <algorithm> // min
#include "../../lumos/DTO.h"
#include "../../lumos/declaration.h"
//#include "../../tester/tester.h"
#include "../../tester/objects.h"

class talk_to_client;
typedef boost::shared_ptr<talk_to_client> client_ptr;
typedef std::vector<client_ptr> clients;

extern clients all_clients;

typedef std::list<tester::task_ptr> tasks;

//extern tasks all_tasks;

extern boost::asio::strand schedule_strand;

void on_schedule (tester::status_ptr status_, client_ptr client_);

#endif
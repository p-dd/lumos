//#define _SCL_SECURE_NO_WARNINGS
#include "../../tester/WinAPI_helpers.h"
#include "talk_to_server.h"
//#include "../../lumos/declaration.h"

//#include <lmcons.h>

int main (int argc, char* argv[])
{
	// connect several all_clients
	boost::uuids::uuid dir_uuid = boost::uuids::random_generator()();
	freopen(("d:\\lumos\\lumos_client_" + boost::lexical_cast<std::string>(dir_uuid) + ".log").c_str(), "w", stdout);
	tester::job::set_restricted_user ();
	boost::asio::ip::tcp::endpoint ep (boost::asio::ip::address::from_string ("10.0.52.10"), 8001);
	talk_to_svr::new_ (ep);
	service.run ();
}
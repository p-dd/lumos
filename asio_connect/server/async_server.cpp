
#include "talk_to_client.h"
#include "talk_to_php.h"
#include "../../lumos/talk_to_db.h"


int main (int argc, char* argv[])
{	
	// boost::thread
	//udp_server_start ();	
	freopen("d:\\lumos\\lumos_server.log", "w", stdout);
	db->settings ("10.0.52.10", "lumos_client", "Flopsie!", 3306, true);
	db->start ();
	php_server_start ();
	tcp_server_start ();
}
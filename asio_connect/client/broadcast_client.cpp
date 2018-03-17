#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <string>

namespace ip = boost::asio::ip;
boost::asio::io_service io_service;

char s[] = "int";

void find_server ()
{
	ip::udp::socket socket (io_service,
		ip::udp::endpoint (ip::udp::v4 (), 0));
	socket.set_option (boost::asio::socket_base::broadcast (true));

	// Broadcast will go to port 8888.
	ip::udp::endpoint broadcast_endpoint (ip::address_v4::broadcast (), 8888);

	// Broadcast data.
	std::string buffer(s);
	socket.send_to (boost::asio::buffer (buffer), broadcast_endpoint);
	buffer = "vas";
	socket.send_to (boost::asio::buffer (buffer), broadcast_endpoint);
	std::cout << "Sent" << std::endl;
	boost::array<char, 1024> buf;
	ip::udp::endpoint server;
	do {
		socket.receive_from (boost::asio::buffer (buf), server);
		std::cout << (server.address ()).to_string () << std::endl;
		std::cout << server.port () << std::endl;
		std::cout << (server.protocol ()).type () << std::endl;
		std::cout << (server.protocol ()).family () << std::endl;
		std::cout << (server.protocol ()).protocol () << std::endl;

		for (int i = 0; i < 4; i++) std::cout << buf[i];
		std::cout << std::endl;
	}
	while (buf[1] != 'x');
	socket.close ();
	system ("pause");
}

int main()
{

  // Server binds to any address and any port.
	find_server ();
  
}
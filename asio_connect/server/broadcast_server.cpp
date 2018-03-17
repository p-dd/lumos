#include <iostream>

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>

typedef boost::system::error_code error_code;

boost::array<char, 4> buf1;
boost::array<char, 4> buf2;

namespace ip = boost::asio::ip;
boost::asio::io_service io_service;
ip::udp::socket my_socket (io_service, ip::udp::endpoint (ip::udp::v4 (), 8888));
ip::udp::endpoint sender_endpoint;

namespace vasya {
	void onread (const error_code & err, size_t bytes, boost::array<char, 4> * buf);

	void onwrite (const error_code & err, size_t bytes)
	{
		std::cout << "wrote: " << bytes << std::endl;
		my_socket.async_receive_from (boost::asio::buffer (buf1), sender_endpoint, boost::bind (&vasya::onread, _1, _2, &buf1));
	}

	void onread (const error_code & err, size_t bytes, boost::array<char, 4> * buf)
	{
		std::cout << "num of bytes: " << bytes << std::endl;
		for (int i = 0; i < 4; i++) std::cout << (*buf)[i];
		std::cout << std::endl;
		std::cout << (sender_endpoint.address ()).to_string () << std::endl;
		std::cout << sender_endpoint.port () << std::endl;
		std::cout << (sender_endpoint.protocol ()).type () << std::endl;
		(*buf)[1] = 'p';
		my_socket.async_send_to (boost::asio::buffer (*buf), sender_endpoint, &vasya::onwrite);
		
	}


}

int main ()
{


	// Client binds to any address on port 8888 (the same port on which
	// broadcast data is sent from server).



	// Receive data.
	
	my_socket.async_receive_from (boost::asio::buffer (buf1), sender_endpoint, boost::bind (&vasya::onread, _1, _2, &buf1));
	
	my_socket.async_receive_from (boost::asio::buffer (buf2), sender_endpoint, boost::bind (&vasya::onread, _1, _2, &buf2));
	//std::cout << "got " << bytes_transferred << " bytes." << std::endl;
	std::system ("pause");

	
	io_service.run ();
}
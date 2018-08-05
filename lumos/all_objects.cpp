#include "all_objects.h"
#include "talk_to_db.h"

namespace tester {
	compilers all_cm;
	solutions all_sol;
	problems all_prob;
	statuses all_statuses;
	std::vector <CLS_ptr> all_cls;

	compiler_ptr compilers::operator[] (id index)
	{
		auto it = all.find (index);
		compiler_ptr & tmp = (it == all.end()) ? nullptr : it->second;
		if (tmp == nullptr) {
			tmp = db->get_compiler (index);
			all.insert (std::make_pair (index, tmp));
		}
		return tmp;
	}

	problem_ptr problems::operator[] (id index)
	{
		auto it = all.find (index);		
		problem_ptr & cur = (it == all.end ()) ? nullptr : it->second;
		if (cur == nullptr) 
		{
			cur = db->get_problem (index);
			all.insert (std::make_pair (index, cur));
		}
		else 
		{
			if (db->get_problem_timestamp(index) != cur->get_timestamp())
			cur = db->get_problem(index);
		}
		return cur;
	}

	solution_ptr solutions::operator[] (id index)
	{
		auto it = all.find (index);
		solution_ptr & tmp = (it == all.end ()) ? nullptr : it->second;
		if (tmp == nullptr) {
			tmp = db->get_solution (index);
			all.insert (std::make_pair (index, tmp));
		}
		return tmp;
	}	

	void lti_update_grade(solution_ptr sol)
	{
		using boost::asio::ip::tcp;
		try
		{
			const char host[] = "softgrader.itmm.unn.ru";
			
			// Get a list of endpoints corresponding to the server name.
			tcp::resolver resolver(service);
			tcp::resolver::query query(host, "http");
			tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

			// Try each endpoint until we successfully establish a connection.
			tcp::socket socket(service);
			boost::asio::connect(socket, endpoint_iterator);

			// Form the request. We specify the "Connection: close" header so that the
			// server will close the socket after transmitting the response. This will
			// allow us to treat all data up until the EOF as the content.
			std::stringstream ss;
			ss << "id_user=" << sol->get_id_user();
			ss << "&id_course=" << sol->get_id_course();
			ss << "&id_problem=" << sol->get_id_problem();
			ss << "&official=" << 1;
			std::string post = ss.str();

			boost::asio::streambuf request;
			std::ostream request_stream(&request);

			request_stream << "POST /actions/lti/grade/update HTTP/1.1 \r\n";
			request_stream << "Host: " << host << "\r\n";
			request_stream << "User-Agent: lumos/1.0\r\n";
			request_stream << "Content-Type: application/x-www-form-urlencoded; charset=UTF-8\r\n";
			request_stream << "Accept: */*\r\n";
			request_stream << "Content-Length: " << post.length() << "\r\n";
			request_stream << "Connection: close\r\n\r\n";  // NOTE: double line feed needed
			request_stream << post;
			
			// Send the request.
			boost::asio::write(socket, request);

			// Read the response status line. The response streambuf will automatically
			// grow to accommodate the entire line. The growth may be limited by passing
			// a maximum size to the streambuf constructor.
			boost::asio::streambuf response;
			boost::asio::read_until(socket, response, "\r\n");

			// Check that response is OK.
			std::istream response_stream(&response);
			std::string http_version;
			response_stream >> http_version;
			unsigned int status_code;
			response_stream >> status_code;
			std::string status_message;
			std::getline(response_stream, status_message);
			if (!response_stream || http_version.substr(0, 5) != "HTTP/")
			{
				std::cout << "Invalid response\n";
				return;
			}
			if (status_code != 200)
			{
				std::cout << "Response returned with status code " << status_code << "\n";
				return;
			}

			// Read the response headers, which are terminated by a blank line.
			boost::asio::read_until(socket, response, "\r\n\r\n");

			// Process the response headers.
			std::string header;
			while (std::getline(response_stream, header) && header != "\r");
				//std::cout << header << "\n";
			//std::cout << "\n";

			// Write whatever content we already have to output.
			if (response.size() > 0)
				std::cout << &response;

			// Read until EOF, writing data to output as we go.
			boost::system::error_code error;
			while (boost::asio::read(socket, response,
				boost::asio::transfer_at_least(1), error))
				std::cout << &response;
			if (error != boost::asio::error::eof)
				throw boost::system::system_error(error);
		}
		catch (std::exception& e)
		{
			std::cout << "Exception: " << e.what() << "\n";
		}
	}

};
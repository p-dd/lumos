#ifndef PIPE_H
#define PIPE_H

#include "WinAPI_helpers.h"
#include "objects.h"
#include "../lumos/declaration.h"
//#include "tester.h"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp> 


//#include <windows.h>

namespace tester {
	class pipe
	{
	protected:
		HANDLE hPipeRead, hPipeWrite;
		bool read;
	public:
		pipe (bool read_)
		{
			read = read_;
			SECURITY_ATTRIBUTES sa;
			sa.nLength = sizeof (SECURITY_ATTRIBUTES);
			sa.bInheritHandle = TRUE;
			sa.lpSecurityDescriptor = NULL;
			if (!CreatePipe (&hPipeRead, &hPipeWrite, &sa, 0))	std::cout << "CreatePipe Error : hPipeRead" << std::endl;
			if (read) if (!SetHandleInformation (hPipeRead, HANDLE_FLAG_INHERIT, 0)) std::cout << "SetHandleInformation Error : hPipeRead" << std::endl;
			else if (!SetHandleInformation (hPipeWrite, HANDLE_FLAG_INHERIT, 0)) std::cout << "SetHandleInformation Error : hPipeWrite" << std::endl;
		}
		~pipe () { CloseHandle (hPipeRead); CloseHandle (hPipeWrite); }
	};

	class opipe;
	class ipipe : public pipe
	{
	public:
		ipipe () : pipe (true) {};
		template <class data_type>
		ipipe & operator >> (data_type & data)
		{
			DWORD dwRead;
			if (!ReadFile (hPipeRead, &data, sizeof (data), &dwRead, NULL)) std::cout << "Error : Not may write to file" << std::endl;
			if (dwRead != sizeof (data)) std::cout << "Some error" << std::endl;
			return (*this);
		}
		ipipe & operator >> (binary_ptr & data)
		{
			DWORD dwRead;
			char buf[1024];
			while (ReadFile (hPipeRead, buf, sizeof (buf), &dwRead, NULL)) data->insert (data->end (), (char *)&buf, (char *)&buf + dwRead);
			if (GetLastError () != ERROR_BROKEN_PIPE) std::cout << "Error : Not may write to file" << std::endl;
			return (*this);
		}
		HANDLE get_pipe () { return hPipeWrite; }
		void close_handle () { CloseHandle (hPipeWrite); }
		friend void redirect (ipipe_ptr pipe_from, opipe_ptr pipe_to);
	};
	typedef boost::shared_ptr<ipipe> ipipe_ptr;

	class opipe : public pipe
	{
	public:
		opipe () : pipe (false) {};
		template <class data_type>
		opipe & operator << (data_type & data)
		{
			DWORD dwWritten;
			if (!WriteFile (hPipeWrite, &data, sizeof (data), &dwWritten, NULL)) std::cout << "Error : Not may write to file" << std::endl;
			return (*this);
		}
		opipe & operator << (binary_ptr & data)
		{
			DWORD dwWritten;
			if (!WriteFile (hPipeWrite, &(*data)[0], data->size (), &dwWritten, NULL)) std::cout << "Error : Not may write to file" << std::endl;
			if (dwWritten != data->size ()) std::cout << "Error : Write to pipe but not all" << std::endl;
			return (*this);
		}
		HANDLE get_pipe () { return hPipeRead; }
		void close_handle () { CloseHandle (hPipeRead); }
		friend void redirect (ipipe_ptr pipe_from, opipe_ptr pipe_to);
	};
	typedef boost::shared_ptr<opipe> opipe_ptr;

};

#endif
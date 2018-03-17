#ifndef PIPE_H
#define PIPE_H

#include "WinAPI_helpers.h"
#include "simple_objects.h"
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
			if (read_) { if (!SetHandleInformation (hPipeRead, HANDLE_FLAG_INHERIT, 0)) std::cout << "SetHandleInformation Error : hPipeRead " << GetLastError() << std::endl; }
			else if (!SetHandleInformation (hPipeWrite, HANDLE_FLAG_INHERIT, 0)) std::cout << "SetHandleInformation Error : hPipeWrite" << std::endl;
		}
		~pipe () { if (hPipeRead) CloseHandle (hPipeRead); if (hPipeWrite) CloseHandle (hPipeWrite); }
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
			if (!ReadFile (hPipeRead, &data, sizeof (data), &dwRead, NULL)) std::cout << "Error : Can't read from pipe" << std::endl;
			return (*this);
		}
		ipipe & operator >> (binary_ptr & data);
		ipipe & operator >> (text & data);
		HANDLE get_pipe () { return hPipeWrite; }
		void close_handle () { CloseHandle (hPipeWrite); hPipeWrite = nullptr; }
		friend void redirect (ipipe & pipe_from, opipe & pipe_to);
		
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
			if (!WriteFile (hPipeWrite, &data, sizeof (data), &dwWritten, NULL)) std::cout << "Error : Can't write to pipe" << std::endl;
			return (*this);
		}
		opipe & operator << (binary_ptr & data);
		opipe & operator << (text & data);
		HANDLE get_pipe () { return hPipeRead; }
		void close_handle () { CloseHandle (hPipeRead); hPipeRead = nullptr; }
		friend void redirect (ipipe & pipe_from, opipe & pipe_to);
	};
	typedef boost::shared_ptr<opipe> opipe_ptr;

};

#endif
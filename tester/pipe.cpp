#include "pipe.h"
namespace tester {
	void redirect (ipipe & pipe_from, opipe & pipe_to)
	{
		DWORD dwRead, dwWritten;
		char buf[1024];
		while (ReadFile (pipe_from.hPipeRead, buf, sizeof (buf), &dwRead, NULL)) if (!WriteFile (pipe_to.hPipeWrite, &buf, dwRead, &dwWritten, NULL)) std::cout << "Error : Not may write to file" << std::endl;
		if (GetLastError () != ERROR_BROKEN_PIPE) std::cout << "Error : Not may write to file" << std::endl;
	}

	ipipe & ipipe::operator >> (text & data)
	{
		DWORD dwRead;
		char buf[1024];
		while (ReadFile (hPipeRead, buf, sizeof (buf), &dwRead, NULL)) {
			data.append ((char *)&buf, (char *)&buf + dwRead);
			std::cout << GetLastError () << std::endl;
		}
		if (GetLastError () != ERROR_BROKEN_PIPE) std::cout << "Error : Can't read from pipe" << std::endl;
		return (*this);
	}
	ipipe & ipipe::operator >> (binary_ptr & data)
	{
		DWORD dwRead;
		char buf[1024];
		while (ReadFile (hPipeRead, buf, sizeof (buf), &dwRead, NULL)) data->insert (data->end (), (char *)&buf, (char *)&buf + dwRead);
		if (GetLastError () != ERROR_BROKEN_PIPE) std::cout << "Error : Can't read from pipe" << std::endl;
		return (*this);
	}


	opipe & opipe::operator << (binary_ptr & data)
	{
		DWORD dwWritten;
		if (!WriteFile(hPipeWrite, data->data(), (DWORD)data->size(), &dwWritten, NULL)) std::cout << "Error : Not may write to pipe" << std::endl; // &(*data)[0]
		if (dwWritten != data->size ()) std::cout << "Error : Write to pipe but not all" << std::endl;
		return (*this);
	}
	opipe & opipe::operator << (text & data)
	{
		DWORD dwWritten;
		if (!WriteFile(hPipeWrite, &data[0], (DWORD)data.size(), &dwWritten, NULL)) std::cout << "Error : Not may write to pipe" << std::endl;
		if (dwWritten != data.size ()) std::cout << "Error : Write to pipe but not all" << std::endl;
		return (*this);
	}
};
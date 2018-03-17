#ifndef DECLARATION_H
#define DECLARATION_H
// I/O
#include <cstdio>

// Windows Socket

const int MAX_LEN = 1024;
const int Port = 1001;

extern char hello_question[];
extern char hello_answer[];
extern char hello_connect[];

#define BUF_LEN 16
extern char send_buf[BUF_LEN];

/*#ifndef UNICODE
#define UNICODE
#endif

#define BOOST_NO_ANSI_APIS*/

// boost::asio
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

#define MEM_FN(x)			boost::bind(&self_type::x, shared_from_this())
#define MEM_FN1(x,y)		boost::bind(&self_type::x, shared_from_this(),y)
#define MEM_FN2(x,y,z)		boost::bind(&self_type::x, shared_from_this(),y,z)
#define MEM_FN3(x,y,z,p)	boost::bind(&self_type::x, shared_from_this(),y,z,p)
#define MEM_FN4(x,y,z,p,t)	boost::bind(&self_type::x, shared_from_this(),y,z,p,t)

extern boost::asio::io_service service;

// Lumos connection

enum action { HELLO, CLS, PING, DB_SETTINGS, TASK, PROBLEM, READY, BYE };

const char PathSeparator =
#ifdef _WIN32
	'\\';
#else
	'/';
#endif


// tester

//enum t_verdict { NO, AC, WA, CE, ML, TL, DE };
//typedef size_t id;

#include <windows.h>
#include <shellapi.h>




#endif
#include "declaration.h"

// Windows Socket

char hello_question[] = "Веруете ли, что Я могу это сделать?\0"; // "Hello, Lumos\0"; //
char hello_answer[] = "Ей, Господи!\0"; // "Hello, Client\0"; //
char hello_connect[] = "По вере вашей да будет вам.\0";

char send_buf[BUF_LEN];

// boost::asio

boost::asio::io_service service;
boost::asio::io_service::work service_work(service);


// MySQL DB

//char url[] = "localhost";
//static int Port = 3307;

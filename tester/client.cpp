//#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS


#include <iostream>


#include "../lumos/declaration.h"
#include "tester.h"
#include "../lumos/talk_to_db.h"

char * rus = setlocale (LC_ALL, "Russian");


//tester::problem prob(1);
std::string s;
tester::task task_ = { 1, 1, 1, 2 };



int main ()
{
	db->settings ("192.168.0.2", "lumos_client", "Flopsie!", 3307);
	db->start ();
	tester::tester che;
	che.reset (task_);
	che.start ();

	std::system ("pause");
}


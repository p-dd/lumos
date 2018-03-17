#ifndef WINAPI_HELPERS_H
#define WINAPI_HELPERS_H

//#define _SCL_SECURE_NO_WARNINGS

#ifndef UNICODE
#define UNICODE
#endif

#include <string>
#include "../lumos/declaration.h"

#include <windows.h>
#include <Lmcons.h>
#include <shellapi.h>
#include <AccCtrl.h>
#include <Aclapi.h>
#include <Ntsecapi.h>
#include <Lmaccess.h>
#include <lm.h>

#pragma comment(lib, "netapi32.lib")

class lpwstr
{
	lpwstr (const lpwstr& other);
	lpwstr& operator=(const lpwstr& other);
	wchar_t * internal_;

public:
	explicit lpwstr (const std::wstring& toCopy); 
	~lpwstr ();	
	wchar_t* str () const { return internal_; }
	const wchar_t* c_str ()  const { return internal_; }
};

bool CreateSecurity (std::wstring Name, DWORD Permissions, DWORD Inheritance, PSECURITY_DESCRIPTOR & psd, PSECURITY_DESCRIPTOR & psd_out);
bool AddUserObjectAccess (std::wstring Name, DWORD Permissions, DWORD Inheritance, HANDLE hUserObject);
bool AddAccountRight (std::wstring UserName, std::wstring PrivilegeName);
void NewUser (std::wstring TestName, std::wstring TestPassword, std::wstring home_dir);
std::wstring get_user_name ();
HANDLE get_restricted_user_token ();

#endif 
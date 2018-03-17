#include "WinAPI_helpers.h"

lpwstr::lpwstr (const std::wstring& toCopy) :
internal_ (new wchar_t[toCopy.size () + 1])
{
	std::copy (toCopy.begin (), toCopy.end (), internal_);
	internal_[toCopy.size ()] = 0;
	std::wcout << L"lpwstr: " << internal_ << std::endl;
}

lpwstr::~lpwstr ()
{
	std::wcout << "delete: " << internal_ << std::endl;
	delete[] internal_;
}

bool CreateSecurity (std::wstring Name, DWORD Permissions, DWORD Inheritance, PSECURITY_DESCRIPTOR & psd, PSECURITY_DESCRIPTOR & psd_out)
{
	EXPLICIT_ACCESS ea;
	PACL pnACL = nullptr, poACL = nullptr;
	BOOL ACLPresent, ACLDefaulted;
	printf ("some error: %u\n", GetLastError ());
	if (psd != nullptr) {
		if (!GetSecurityDescriptorDacl (psd, &ACLPresent, &poACL, &ACLDefaulted)) return false;
		printf ("GetSecurityDescriptorDacl error: %u\n", GetLastError ());
	}
	lpwstr tmp (Name);
	BuildExplicitAccessWithName (&ea, tmp.str (), Permissions, SET_ACCESS, Inheritance);
	DWORD dwError = 0; // SetEntriesInAcl (0, NULL, NULL, &pnACL);
	//printf ("SetEntriesInAcl error: %u\n", dwError);
	if ((dwError = SetEntriesInAcl (1, &ea, poACL, &pnACL)) != ERROR_SUCCESS) {
		printf ("SetEntriesInAcl error: %u\n", GetLastError ());
		return false;
	}

	psd_out = new SECURITY_DESCRIPTOR;
	if (!InitializeSecurityDescriptor (psd_out, SECURITY_DESCRIPTOR_REVISION)) return false;
	if (!SetSecurityDescriptorDacl (psd_out, true, pnACL, false)) return false;
	return true;
}

bool AddUserObjectAccess (std::wstring Name, DWORD Permissions, DWORD Inheritance, HANDLE hUserObject)
{
	PSECURITY_DESCRIPTOR psd, psd_new;
	SECURITY_INFORMATION si;
	DWORD dwSize;
	si = DACL_SECURITY_INFORMATION;
	dwSize = 0;
	psd = nullptr;
	GetUserObjectSecurity (hUserObject, &si, psd, dwSize, &dwSize);
	if (GetLastError () == ERROR_INSUFFICIENT_BUFFER) {
		psd = new char[dwSize];
		if (!GetUserObjectSecurity (hUserObject, &si, psd, dwSize, &dwSize)) {
			printf ("no\n");
		};
		SetLastError (ERROR_SUCCESS);
	}
	printf ("some1 error: %u\n", GetLastError ());
	printf ("some2 error: %u\n", GetLastError ());
	if (!CreateSecurity (Name, Permissions, Inheritance, psd, psd_new)) return false;

	si = DACL_SECURITY_INFORMATION;
	if (!SetUserObjectSecurity (hUserObject, &si, psd_new)) return false;
	if (psd_new != nullptr) delete psd_new;
	if (psd != nullptr) delete psd;
	return true;
}

bool AddAccountRight (std::wstring UserName, std::wstring PrivilegeName)
{
#define STATUS_SUCCESS 0
	PSID psa;
	LSA_HANDLE hPolicy;
	LSA_UNICODE_STRING usPrivilegeName;
	LSA_OBJECT_ATTRIBUTES oa;
	USHORT cch;
	DWORD cbSid, cbDomain;
	DWORD status;
	SID_NAME_USE peUse;
	wchar_t * Domain;
	lpwstr lpwstrPrivilegeName(PrivilegeName);

	ZeroMemory (&oa, sizeof (LSA_OBJECT_ATTRIBUTES));
	cch = (USHORT)PrivilegeName.length();

	cbDomain = 1024; Domain = new wchar_t[cbDomain];
	cbSid = 1024;  psa = new char[cbSid];
	//ZeroMemory(psa, cbSid);
	if (cch > 0) {
		usPrivilegeName.Length = cch * sizeof (wchar_t);
		usPrivilegeName.MaximumLength = usPrivilegeName.Length + sizeof (UNICODE_NULL);
		usPrivilegeName.Buffer = lpwstrPrivilegeName.str();

		status = LsaOpenPolicy (nullptr, &oa, POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES, &hPolicy);

		if (status != STATUS_SUCCESS) {
			wprintf (L"OpenPolicy returned %lu\n", LsaNtStatusToWinError (status));
			return false;
		}

		LookupAccountName (nullptr, UserName.c_str (), psa, &cbSid, Domain, &cbDomain, &peUse);
		if (LsaAddAccountRights (hPolicy, psa, &usPrivilegeName, 1) != STATUS_SUCCESS) return false;
		LsaClose (hPolicy);
	}
	return true;
}

void NewUser (std::wstring TestName, std::wstring TestPassword, std::wstring home_dir)
{
	printf("NewUser\n");
	USER_INFO_2 UserInfo;
	DWORD Parm_Err;
	ZeroMemory (&UserInfo, sizeof (USER_INFO_2));
	lpwstr lpwstrTestName(TestName), lpwstrTestPassword(TestPassword), lpwstr_home_dir(home_dir);
	UserInfo.usri2_name = lpwstrTestName.str();
	UserInfo.usri2_password = lpwstrTestPassword.str ();
	UserInfo.usri2_priv = USER_PRIV_USER;
	UserInfo.usri2_acct_expires = TIMEQ_FOREVER;
	UserInfo.usri2_max_storage = USER_MAXSTORAGE_UNLIMITED;
	UserInfo.usri2_home_dir = lpwstr_home_dir.str();
	UserInfo.usri2_flags = UF_DONT_EXPIRE_PASSWD;
	DWORD err = 0;
	if ((err = NetUserAdd (nullptr, 2, (LPBYTE)&UserInfo, &Parm_Err)) == NERR_Success) printf ("User added. Ok.\n");
	else printf ("User doesn''t add. Error: %d\n", GetLastError ());
	printf ("parm_err: %d\n", Parm_Err);
	printf ("err: %d\n", err);

	//int t;
	//scanf_s ("%d", &t);
	std::wstring str (SE_INTERACTIVE_LOGON_NAME);
	if (AddAccountRight (TestName, str)) printf ("Right were added to user. Ok.");
	else printf ("Right weren''t added to user. Error: %d\n", GetLastError ());
}

std::wstring get_user_name ()
{
	wchar_t username[UNLEN + 1];
	DWORD username_len = UNLEN + 1;
	GetUserName (username, &username_len);
	return std::wstring (username);
}

HANDLE get_restricted_user_token ()
{
	wchar_t * TestName = L"lumos_test", // test
		*TestPassword = L"Le5g7uq35uqiwk", // dr5YEfrev
		*home_dir = L"r:";
	HANDLE hToken;

	if (LogonUser(TestName, nullptr, TestPassword, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken)) printf("Logon\n");
	else {
		DWORD res = GetLastError();
		if (res == ERROR_LOGON_FAILURE) {
			NewUser(TestName, TestPassword, home_dir);
			if (LogonUser(TestName, nullptr, TestPassword, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &hToken)) printf("Logon\n");
			else { printf("Fail logon 2. Error: %d\n", GetLastError()); exit(1); }
		} else {
			printf("Fail logon 1. Error: %d\n", res);
			exit(1);
		}
	}

	HANDLE hWindowStation = GetProcessWindowStation ();
	AddUserObjectAccess (TestName, GENERIC_ALL, SUB_CONTAINERS_AND_OBJECTS_INHERIT, hWindowStation);
	HANDLE hDesktop = GetThreadDesktop (GetCurrentThreadId ());
	AddUserObjectAccess (TestName, GENERIC_ALL, SUB_CONTAINERS_AND_OBJECTS_INHERIT, hDesktop);
	return hToken;
}
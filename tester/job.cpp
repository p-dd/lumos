#include "job.h"
#include <boost/algorithm/string/replace.hpp>
#include <algorithm>

namespace tester {
	std::wstring job::user_name;
	HANDLE job::hToken;

	job::job (test::fmt input_, test::fmt output_, test::fmt error_fmt_, unsigned long affinity_, LONGLONG time_limit_, SIZE_T memory_limit_, bool re_check_, bool ui_check_, bool idle_check_, fs::path path_bin_, fs::path path_dir_, char * enviroment_) :
		input_fmt (input_), output_fmt (output_), error_fmt (error_fmt_),
		affinity (affinity_), time_limit (time_limit_), memory_limit (memory_limit_),
		re_check (re_check_), ui_check (ui_check_), idle_check (idle_check_),
		path_bin (path_bin_), path_dir (path_dir_),
		enviroment (enviroment_)
	{
		if (!create_restricted_job ()) { printf ("Error: job don't created\n"); return; } // Death		
		if (input_fmt == test::fmt::STREAM) pipe_stdin.reset (new opipe ());
		if (output_fmt == test::fmt::STREAM) pipe_stdout.reset (new ipipe ());
		if (error_fmt == test::fmt::STREAM) pipe_stderr.reset (new ipipe ());
	};

	bool job::create_restricted_job ()
	{
		std::cout << "start: " << __FUNCSIG__ << std::endl;
		JOBOBJECT_BASIC_LIMIT_INFORMATION job_li;
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION job_eli;
		JOBOBJECT_BASIC_UI_RESTRICTIONS job_uir;
		hJob = CreateJobObject (nullptr, nullptr);
		if (hJob == nullptr) { // Job wasn''t created.
			printf("Fail create job. Error: %d\n", GetLastError()); fflush(stdout);
			return false;
		}

		ZeroMemory (&job_li, sizeof (job_li));
		ZeroMemory (&job_eli, sizeof (job_eli));
		ZeroMemory (&job_uir, sizeof (job_uir));

		job_li.PriorityClass = IDLE_PRIORITY_CLASS;
		job_li.PerProcessUserTimeLimit.QuadPart = time_limit;
		std::cout << "TL: " << time_limit << std::endl;
		job_li.SchedulingClass = 1;
		job_li.Affinity = affinity;
		job_li.ActiveProcessLimit = 1;
		job_li.LimitFlags = JOB_OBJECT_LIMIT_PRIORITY_CLASS | JOB_OBJECT_LIMIT_PROCESS_TIME | JOB_OBJECT_LIMIT_AFFINITY | JOB_OBJECT_LIMIT_SCHEDULING_CLASS | JOB_OBJECT_LIMIT_ACTIVE_PROCESS | JOB_OBJECT_LIMIT_JOB_MEMORY
			| JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION;

		job_eli.BasicLimitInformation = job_li;
		job_eli.JobMemoryLimit = memory_limit;
		if (!SetInformationJobObject (hJob, JobObjectExtendedLimitInformation, &job_eli, sizeof (job_eli))) {
			// Death // 'Extended limit information weren''t set to job. Error: '
			printf("Fail SetInformationJobObject 1. Error: %d\n", GetLastError()); fflush(stdout);
			return false;
		}

		job_uir.UIRestrictionsClass = JOB_OBJECT_UILIMIT_ALL;
		if (!SetInformationJobObject (hJob, JobObjectBasicUIRestrictions, &job_uir, sizeof (job_uir))) {
			// Death  // 'UI Restrictions weren''t set to job. Error: '
			printf("Fail SetInformationJobObject 2. Error: %d\n", GetLastError()); fflush(stdout);
			return false;
		}
		std::cout << "finish: " << __FUNCSIG__ << std::endl;
		return true;
	}

	bool job::create_sol_process ()
	{
		std::cout << "start: " << __FUNCSIG__ << std::endl;
		STARTUPINFO si;

		ZeroMemory (&si, sizeof (si));
		si.cb = sizeof (STARTUPINFO);
		if (input_fmt == test::fmt::STREAM || output_fmt == test::fmt::STREAM || error_fmt == test::fmt::STREAM) si.dwFlags |= STARTF_USESTDHANDLES;
			
		if (input_fmt == test::fmt::STREAM) si.hStdInput = pipe_stdin->get_pipe ();
		if (output_fmt == test::fmt::STREAM) si.hStdOutput = pipe_stdout->get_pipe ();
		if (error_fmt == test::fmt::STREAM) si.hStdOutput = pipe_stderr->get_pipe ();

		std::wstring command_line;
		if (line_run.size()) {
			command_line = line_run;
			boost::replace_all (command_line, L"%bin%", L"\"" + path_bin.wstring () + L"\"");
			boost::replace_all (command_line, L"%dir%", L"\"" + path_dir.wstring () + L"\"");
			//boost::replace_all (command_line, L"%sol%", L"\"" + path_code.wstring () + L"\"");
			boost::replace_all (command_line, L"%proc%", std::to_string (num_proc));
			boost::replace_all (command_line, L"%thrd%", std::to_string (num_thrd));
			boost::replace_all (command_line, L"%args%", args);
		} else {
			command_line = path_bin.wstring () + (L" ") + args;
		}
		std::wcout << command_line << std::endl;
		std::wcout << enviroment << std::endl;
		if (!CreateProcessAsUser (hToken, nullptr, lpwstr (command_line).str (), nullptr, nullptr, true, CREATE_NO_WINDOW | CREATE_SUSPENDED | CREATE_BREAKAWAY_FROM_JOB, enviroment, path_dir.wstring ().c_str (), &si, &pi)) {
			// Death // Process didn't created.
			printf("Fail CreateProcessAsUser. Error: %d\n", GetLastError()); fflush(stdout);
			return false;
		}
		if (input_fmt == test::fmt::STREAM) pipe_stdin->close_handle ();
		if (output_fmt == test::fmt::STREAM) pipe_stdout->close_handle (); 
		if (error_fmt == test::fmt::STREAM) pipe_stderr->close_handle ();
		if (!AddUserObjectAccess (user_name, GENERIC_ALL, SUB_CONTAINERS_AND_OBJECTS_INHERIT, pi.hProcess)) {
			// Death
			printf("Fail AddUserObjectAccess. Error: %d\n", GetLastError()); fflush(stdout);
			return false;
		}
		
		if (!AssignProcessToJobObject (hJob, pi.hProcess)) {
			printf ("Fail AssignProcessToJobObject. Error: %d\n", GetLastError ());
			//printf ("%d | %d %d | %d %d\n", hJob, pi.hProcess, pi.hThread, pi.dwProcessId, pi.dwThreadId);
			std::cout << hJob << " | " << pi.hProcess << " " << pi.hThread << " | " << pi.dwProcessId << " " << pi.dwThreadId << std::endl;
			fflush(stdout);
			system ("pause");
			// Death // Process didn't assigned to job
			return false;
		}
		hProcess = pi.hProcess;
		hThread = pi.hThread;
		std::cout << "finish: " << __FUNCSIG__ << std::endl;
		return true;
	}


	bool job::run ()
	{
		std::cout << "start: " << __FUNCSIG__ << std::endl;
		const size_t MAX_IOPENDING = 100;
		bool ok = false;
		HANDLE * hObjects;
		DWORD cb;
		DWORD WaitResult;
		DWORD exit_code;
		JOBOBJECT_EXTENDED_LIMIT_INFORMATION joeli;
		JOBOBJECT_BASIC_ACCOUNTING_INFORMATION jobai;
		if (!create_sol_process ()) { printf ("Error: process don't created\n");  return false; } // Death
		cb = sizeof (joeli);
		if (!QueryInformationJobObject (hJob, JobObjectExtendedLimitInformation, &joeli, cb, &cb)) { return false; } // Death
		if (joeli.PeakProcessMemoryUsed > joeli.JobMemoryLimit) {
			printf ("Memory Limit Exceed. Used: %d\n", joeli.PeakProcessMemoryUsed);
			ok = true;
			if (!TerminateProcess (hProcess, ERROR_NOT_ENOUGH_QUOTA)) { return false; } // Death
			WaitResult = WAIT_OBJECT_0 + 1;
		}
		
		clock_t be = clock (), en = be + 1;
		if (!ok) if (ResumeThread (hThread) == -1) { return false; } // Death
		
		hObjects = new HANDLE[2];
		hObjects[0] = hThread;
		hObjects[1] = hJob;

		proc_info pi_system;

		int cnt = 0;
		long long last = 0, now = 0;
		
		while (!ok) {
			WaitResult = WaitForMultipleObjects (2, hObjects, FALSE, 1);
			en = clock ();
			cb = sizeof (jobai);
			QueryInformationJobObject (hJob, JobObjectBasicAccountingInformation, &jobai, cb, &cb);
			BOOL flag = FALSE;
			//BOOL ok_io_pending = GetThreadIOPendingFlag (hThread, &flag);
			DWORD ok_io_pending = pi_system.IsThreadWaitingUserRequest(pi.dwProcessId, pi.dwThreadId, &flag);
			//if (0 == WaitForInputIdle (hProcess, 0)) flag = true;
			last = now;
			now = jobai.ThisPeriodTotalUserTime.QuadPart + jobai.ThisPeriodTotalKernelTime.QuadPart;
			if (ok_io_pending == ERROR_SUCCESS && flag) { if (last == now) cnt++; }
			else cnt = 0;

			if (cnt > MAX_IOPENDING) {
				if (!TerminateProcess (hProcess, ERROR_NOT_ENOUGH_QUOTA)) { return false; }
				WaitResult = WAIT_OBJECT_0 + 2;
				break;
			}
			if (num_thrd == 1) {
				if (jobai.TotalUserTime.QuadPart > time_limit) {
					if (!TerminateProcess (hProcess, ERROR_NOT_ENOUGH_QUOTA)) { return false; }
					WaitResult = WAIT_OBJECT_0 + 1;
					break;
				}
			} else {
				if (((long long)en - be) * 10000000ll > time_limit * CLOCKS_PER_SEC) {
					if (!TerminateProcess (hProcess, ERROR_NOT_ENOUGH_QUOTA)) { return false; }
					WaitResult = WAIT_OBJECT_0 + 1;
					break;
				}
			}
			switch (WaitResult - WAIT_OBJECT_0) {
				case 0: ok = true; break;
				case 1: ok = true; break;
			}
			if (WaitResult == WAIT_FAILED) { return false; }
			Sleep (1);
		}

		exit_code = ERROR_SUCCESS;
		if (!GetExitCodeProcess (hProcess, &exit_code)) { return false; }

		cb = sizeof (joeli);
		if (!QueryInformationJobObject (hJob, JobObjectExtendedLimitInformation, &joeli, cb, &cb)) { return false; }
		cb = sizeof (jobai);
		if (!QueryInformationJobObject (hJob, JobObjectBasicAccountingInformation, &jobai, cb, &cb)) { return false; }
		if (exit_code != 0) printf ("exit_code: %d\n", exit_code);
		switch (WaitResult - WAIT_OBJECT_0) {
			case 0: // 'Process ended'
				if (exit_code != ERROR_SUCCESS) {
					cls->set_verdict (CLS::RE);
					printf ("exit_code: %d\n", exit_code);
				}
			case 1: // 'Time Limit Exceed' 'Memory Limit Exceed'
			{
				if (joeli.PeakProcessMemoryUsed > joeli.JobMemoryLimit) {
					cls->set_verdict (CLS::ML);
					printf ("Memory Limit Exceed. Used: %d\n", joeli.PeakProcessMemoryUsed);
				}
				if (jobai.TotalUserTime.QuadPart > time_limit) {
					cls->set_verdict (CLS::TL);
					printf ("Time Limit Exceed. Used: %I64d\n", jobai.TotalUserTime.QuadPart);
				}
				break;
			}
			case 2: {
				cls->set_verdict (CLS::IL);
				printf ("Process is idle\n");
			}
		}
		WaitForSingleObject (hThread, 10);
		if (WaitForSingleObject(hProcess, 1000) == WAIT_TIMEOUT) {
			if (!TerminateProcess(hProcess, ERROR_NOT_ENOUGH_QUOTA)) { return false; }
		}
		if (cls->is_ext ()) {
			cls->get_ext ()->set_exit_code (exit_code);
			cls->get_ext ()->set_peak_memory (joeli.PeakProcessMemoryUsed);
			LONGLONG par_time = std::max (1ll, (LONGLONG) (((long long)en - be) * 10000000ll / CLOCKS_PER_SEC));
			//if (num_thrd == 1) cls->get_ext ()->set_run_time (jobai.TotalUserTime.QuadPart);
			//else 
			FILETIME creation_time, exit_time, kernel_time, user_time;
			GetProcessTimes (hProcess, &creation_time, &exit_time, &kernel_time, &user_time);
			LONGLONG process_time = reinterpret_cast<PLARGE_INTEGER>(&exit_time)->QuadPart - reinterpret_cast<LARGE_INTEGER*>(&creation_time)->QuadPart;
			GetThreadTimes (hThread, &creation_time, &exit_time, &kernel_time, &user_time);
			LONGLONG thread_time = reinterpret_cast<PLARGE_INTEGER>(&exit_time)->QuadPart - reinterpret_cast<LARGE_INTEGER*>(&creation_time)->QuadPart;
			cls->get_ext ()->set_run_time (process_time);
			
			std::cout << "TotalUserTime: " << jobai.TotalUserTime.QuadPart << std::endl;
			std::cout << "TotalKernelTime: " << jobai.TotalKernelTime.QuadPart << std::endl;
			std::cout << "ThisPeriodTotalKernelTime: " << jobai.ThisPeriodTotalKernelTime.QuadPart << std::endl;
			std::cout << "ThisPeriodTotalUserTime: " << jobai.ThisPeriodTotalUserTime.QuadPart << std::endl;
			std::cout << "TotalPageFaultCount: " << jobai.TotalPageFaultCount << std::endl;
			std::cout << "TotalProcesses: " << jobai.TotalProcesses << std::endl;
			std::cout << "ActiveProcesses: " << jobai.ActiveProcesses << std::endl;
			std::cout << "TotalPageFaultCount: " << jobai.TotalTerminatedProcesses << std::endl;
			//std::cout << (en - be) << std::endl;
			/*std::cout << ((long double)en - be) / CLOCKS_PER_SEC << std::endl;
			std::cout << (((long long)en - be) * 10000000ll / CLOCKS_PER_SEC) << std::endl;*/
			std::cout << "Process time: " << process_time << std::endl;
			std::cout << "Thread time: " << thread_time << std::endl;
			std::cout << "clock_t : " << par_time << std::endl;
			
		}
		/*if (output_fmt == test::fmt::STREAM) {
			*pipe_stdout >> output;
		}
		if (error_fmt == test::fmt::STREAM) {
			*pipe_stderr >> error;
		}*/
		//system ("pause");
		CloseHandle (hThread);
		CloseHandle (hJob);
		CloseHandle (hProcess);
		std::cout << "finish: " << __FUNCSIG__ << std::endl;
		return true;
	}
};
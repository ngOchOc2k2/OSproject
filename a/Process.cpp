#define UNICODE 
#pragma once
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <windows.h>
#include <time.h>
#include <sys/types.h>
#include <psapi.h>
#include <tlhelp32.h>
#include "Analyse.cpp"
#include "Command.cpp"

using namespace std;

#define MAX_CWD_LENS 512
#define MAX_BUFFER_SIZE 256
#define MAX_TOK_BUFSIZE 256
#define TOKEN_DELIMETERS " \t\r\n\a"

HANDLE hForeProcess;

///////////////////////////////////
//////// Xử lí tiến trình /////////
///////////////////////////////////

/**
 * Đón tín hiệu ngắt Ctrl + C
 **/
void sighandler(int signum) {
    /**
     * Đón tín hiệu ngắt Ctrl + C
     **/

    if (hForeProcess != NULL) {
        TerminateProcess(hForeProcess, 0);
        hForeProcess = NULL;
    }
    exit(1);
}

/**
 * In ra các tiến trình đang hoạt động
 * In ra màn hình tên tiến trình, Process ID, Parent PID
 * Câu lệnh: pc all
 * 
 **/
int getProcessListAll() {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32; /* Cấu trúc của tiến trình khi được gọi snap
    https://docs.microsoft.com/en-us/windows/win32/api/tlhelp32/ns-tlhelp32-processentry32*/

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); /* Chụp lại các tiến trình
    https://docs.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-createtoolhelp32snapshot 
    */
    // Nếu hProcessSnap trả về lỗi return 0
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        cout << "ERROR: CreateToolhelp32Snapshot Fail " << GetLastError() << endl;
        return 0;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);
    // Kiểm tra thằng đầu tiên
    if (!Process32First(hProcessSnap, &pe32)) {
        // Nếu lỗi in ra...
        cout << "ERROR: Process32First Fail " << GetLastError() << endl;
        return 0;
    }
    printf("%-50s%-20s%-20s\n", "Process Name", "Process ID", "Parent Process ID");
	printf("%-50s%-20s%-20s\n", "----------------------------------", "----------", "-----------");    
    
    do {
    	printf("%-50S%-20d%-20d\n", pe32.szExeFile, pe32.th32ProcessID, pe32.th32ParentProcessID);
    } while (Process32Next(hProcessSnap, &pe32)); CloseHandle(hProcessSnap);
    return 1;
}

/**
 * Tìm tiến trình bằng tên
 * In ra màn hình tên tiến trình, Process ID, Parent PID
 * Câu lệnh pc find [name_process]
 * 
 **/
int findProcessID(char *name_process) {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32; /* Cấu trúc của tiến trình khi được gọi snap
    https://docs.microsoft.com/en-us/windows/win32/api/tlhelp32/ns-tlhelp32-processentry32*/

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); /* Chụp lại các tiến trình
    https://docs.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-createtoolhelp32snapshot 
    */

    // Nếu trả về lỗi return 0
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return 0;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32); // đọc cái link ở PROCESSENTRY32 là hiểu
    
    // Kiểm tra thằng đầu tiên
    /*hàm Process32First sẽ lưu ảnh chụp các TT vào con trỏ &pe32*/
    if (!Process32First(hProcessSnap, &pe32)) {
        return 0;
    }
    printf("%-50s%-20s%-20s\n", "Process Name", "Process ID", "Parent Process ID");
	printf("%-50s%-20s%-20s\n", "----------------------------------", "----------", "-----------");
    do {
        if (strcmp(name_process, getChar(pe32.szExeFile)) == 0) {
            // Nếu pe32.szExeFile trùng với tên tiến trình thì in ra
            printf("%-50S%-20d%-20d\n", pe32.szExeFile, pe32.th32ProcessID, pe32.th32ParentProcessID);
        }
    } while (Process32Next(hProcessSnap, &pe32)); CloseHandle(hProcessSnap);
    return 1;
}

/**
 * Đóng tiến trình bằng Process ID 
 * Câu lệnh: pc kill [process_id]
 * 
 **/

int killProcessID(DWORD process_id) {
    // Mở tiến trình đang chạy có Process ID là...
    HANDLE hprocess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
    // Nếu hProcess trả về NULL thì báo lỗi
    if (hprocess == NULL) {
        cout << "ERROR: Failed!" << endl;
        return 1;
    }
    // Đóng tiến trình hProcess
    if (!TerminateProcess(hprocess, 0)) {
        return 0;
    }
    return 1;
}

/**
 * Đóng tất cả tiến trình có tên là name_process
 * Câu lệnh pc kill [Name_Process]
 * 
 * */
int killProcess(char *name_process) {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32; // Cấu trúc của tiến trình khi được gọi snapshot

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // Chụp lại các tiến trình
    // Nếu trả về lỗi return 0
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        return 0;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);
    
    // Kiểm tra thằng đầu tiên
    if (!Process32First(hProcessSnap, &pe32)) {
        return 0;
    }
    do {
        if (strcmp(name_process, getChar(pe32.szExeFile)) == 0) {
            killProcessID(pe32.th32ProcessID);
        }
    } while (Process32Next(hProcessSnap, &pe32));
    CloseHandle(hProcessSnap);
    return 1;
}

/**
 * Đình chỉ một tiến trình đang thực hiện 
 * Câu lệnh pc suspend [process_id]
 * 
 **/
int suspendProcess(DWORD process_id) {
    // Chụp lại tất cả các luồng 
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    THREADENTRY32 th32; // Cấu trúc của luồng khi được gọi snapshot
    HANDLE hthread;
    // Kiểm tra xem hThreadSnap có lỗi không nếu có thì in ra lỗi    
    if (hThreadSnap == INVALID_HANDLE_VALUE) { 
	cout << "ERROR: CreateToolhelp32Snapshot" << GetLastError();
	return 0;
    }
    th32.dwSize = sizeof(THREADENTRY32);
    // Kiểm tra thông tin của luồng đầu tiên
    if (!Thread32First(hThreadSnap, &th32)) {
	cout << "Thread32First Fail " <<  GetLastError(); // Nếu lỗi in ra lỗi
	CloseHandle(hThreadSnap);          // Đóng Handle snapshot
	return 0;
    }
    // Duyệt các luồng khác
    do {
    // Kiểm tra xem các luồng này có thuộc tiến trình cần dừng không
	if (th32.th32OwnerProcessID == process_id) {
	    hthread = OpenThread(THREAD_ALL_ACCESS, FALSE, th32.th32ThreadID); // Mở một luồng đang chạy
	    // Đình chỉ luồng đó
            if (SuspendThread(hthread) == -1) {
		return 0;
	    }
	}
    } while (Thread32Next(hThreadSnap, &th32));	CloseHandle(hThreadSnap);
	return 1;
}

/**
 * Tiếp tục một tiến trình bị đình chỉ
 * Câu lệnh pc resume [process_id]
 * 
 **/
int resumeProcess(DWORD process_id) {
    // Chụp lại tất cả các luồng 
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    THREADENTRY32 th32; // Cấu trúc của luồng khi được gọi snapshot
    HANDLE hthread;
    // Kiểm tra xem hThreadSnap có lỗi không nếu có thì in ra lỗi
    if (hThreadSnap == INVALID_HANDLE_VALUE) { 
	cout << "ERROR: CreateToolhelp32Snapshot" << GetLastError();
	return 0;
    }
    th32.dwSize = sizeof(THREADENTRY32);
    // Kiểm tra thông tin của luồng đầu tiên
    if (!Thread32First(hThreadSnap, &th32)) {
	cout << "Thread32First Fail " <<  GetLastError(); // Nếu lỗi in ra lỗi
	CloseHandle(hThreadSnap);          // Đóng Handle snapshot
	return 0;
    }
    // Duyệt các luồng khác
    do {
        // Kiểm tra xem các luồng này có thuộc tiến trình cần dừng không
	if (th32.th32OwnerProcessID == process_id) {
	    hthread = OpenThread(THREAD_ALL_ACCESS, FALSE, th32.th32ThreadID); // Mở một luồng đang chạy
            // Đình chỉ luồng đó
            if (ResumeThread(hthread) == -1) {
	        return 0;
	    }
	}
    } while (Thread32Next(hThreadSnap, &th32)); CloseHandle(hThreadSnap);
    return 1;
}

/**
 * Tạo một tiến trình con 
 * Câu lệnh: pc bg [name_process/path](background mode)
 *           pc fg [name_process/path](foreground mode)
 * 
 **/
int createNewProcess(char **args) {
    /*
        https://docs.microsoft.com/en-us/windows/win32/procthread/creating-processes
    */
    // Cài wait time cho các tiến trình
    int wait_time;
    if (strcmp(args[1], "bg") == 0) {
        wait_time = 0;
    } else {
        wait_time = INFINITE;
    }
    char *run_file = combinePath(args, 2); // Ghép lại tên tiến trình hoặc đường dẫn
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.wShowWindow = SW_SHOW;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.lpTitle = GetWC(args[1]);
    ZeroMemory(&pi, sizeof(pi));

    // Khởi tạo tiến trình con
    if (!CreateProcess(NULL, GetWC(run_file), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
	int error = GetLastError(); // Kiểm tra lỗi
    if (error == 2)
	cout << "The batch file or execute file '" << run_file << "' is not found." << endl;
    else
	cout << "Can't run this file" << endl;
        return 1;
    }
    // gán handle cho tiến trình con vừa tạo
    if (strcmp(args[1], "fg") == 0) {
	    hForeProcess = pi.hProcess;
    }
    // Thời gian đợi 1 tiến trình con
    printf("Create successfully !\n");
    WaitForSingleObject(pi.hProcess, wait_time);
    // Đóng các handle
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 1;
}

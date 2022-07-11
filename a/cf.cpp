#include <windows.h>
#include <tchar.h>
#include <bits/stdc++.h>
using namespace std;
#define STRSAFE_LIB
#define MAX_THREADS 3
#define BUF_SIZE 255


// Sample custom data structure for threads to use.
// This is passed by void pointer so it can be any data type
// that can be passed using a single void pointer (LPVOID).
typedef struct MyData {
    int x;
} MYDATA, *PMYDATA;
void T1(int x);
void T2(int x);
void T3(int x);
void (*activate_command[])(int x) = {  
    /**
     * đây là một mảng ,mỗi phần tử của mảng là một con trỏ đến hàm mang tham số char**
     * 
     */
    &T1,
    &T2,
    &T3
   
   
}; 
void T1(int x){
  while(1) printf(" 1 ");
};
void T2(int x){
  while(1) printf(" 2 ");
};
void T3(int x){
  while(1) printf(" 3 ");
}
int _tmain()
{
    PMYDATA  pDataArray[MAX_THREADS];
    DWORD   dwThreadIdArray[MAX_THREADS];
    HANDLE  hThreadArray[MAX_THREADS]; 
    // Create MAX_THREADS worker threads.

    for( int i=0; i<MAX_THREADS; i++ )
    {
       pDataArray[i] = (PMYDATA) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                sizeof(MYDATA));
        hThreadArray[i] = CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            (LPTHREAD_START_ROUTINE)activate_command[i],       // thread function name
            pDataArray[i],          // argument to thread function 
            0,                      // use default creation flags 
            &dwThreadIdArray[i]);   // returns the thread identifier 


    
        if (hThreadArray[i] == NULL) 
        {
           cout << "error";
           ExitProcess(3);
        }
    } // End of main thread creation loop.

    // Wait until all threads have terminated.

    WaitForMultipleObjects(MAX_THREADS, hThreadArray, TRUE, 10);

    // Close all thread handles and free memory allocations.

    for(int i=0; i<MAX_THREADS; i++)
    {
        CloseHandle(hThreadArray[i]);
        if(pDataArray[i] != NULL)
        {
            HeapFree(GetProcessHeap(), 0, pDataArray[i]);
            pDataArray[i] = NULL;    // Ensure address is not reused.
        }
    }

    return 0;
}



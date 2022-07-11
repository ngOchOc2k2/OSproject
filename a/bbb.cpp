#include <bits/stdc++.h>
#include <windows.h>
#include <ctime>
#include <sys/time.h>
#include <chrono>
using std::cout; using std::endl;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;
using namespace std;
#define ll long long
#define MAX_THREADS 2
typedef struct MyData {
    int start,
     end,index;
} MYDATA, *PMYDATA;
#define MAX 10000001
ll sums[MAX_THREADS];
vector <ll> a(MAX);

void tong( LPVOID para){
    PMYDATA p = (PMYDATA) para;
    for(ll i=p->start; i<=p->end;i++){sums[p->index] += a[i];};
    
}

int main(){
    for(int i=1;i<MAX;i++){
        a[i] = i;
    }
    clock_t start1 = clock();
    
    ll sum1 = 0;
    for(int i=1;i<=MAX;i++){sum1+=a[i];};
    clock_t end1 = clock();
    double cpu_time_used1 = ((double) (end1 - start1)) / CLOCKS_PER_SEC;
    cout << sum1 << endl;
    cout << cpu_time_used1 <<endl;
    
   
    
   

    PMYDATA  pDataArray[MAX_THREADS];
    DWORD   dwThreadIdArray[MAX_THREADS];
    HANDLE  hThreadArray[MAX_THREADS]; 

    for( int i=0; i<MAX_THREADS; i++ )
    {
       pDataArray[i] = (PMYDATA) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                sizeof(MYDATA));
                pDataArray[i]->start = i*(MAX/MAX_THREADS)+1;
                pDataArray[i]->end = (i+1)*(MAX/MAX_THREADS);
                pDataArray[i]->index = i;
        hThreadArray[i] = CreateThread( 
            NULL,                   // default security attributes
            0,                      // use default stack size  
            (LPTHREAD_START_ROUTINE)tong,       // thread function name
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
    clock_t start2 = clock();
    WaitForMultipleObjects(MAX_THREADS, hThreadArray, TRUE, INFINITE);

    // Close all thread handles and free memory allocations.
    ll sumall = 0;
    for(int i=0;i<MAX_THREADS;i++){
        //printf("%ld\n",sums[i]);
        sumall += sums[i];
    }
    cout << sumall << endl;
    clock_t end2 = clock();
    double cpu_time_used2 = ((double) (end2 - start2)) / CLOCKS_PER_SEC;
    cout << cpu_time_used2;
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
#include <chrono>
#include <iostream>
#include <sys/time.h>
#include <ctime>

using std::cout; using std::endl;

int main() {
    time_t now = time(nullptr);
    time_t mnow = now * 1000;

    cout << "seconds since epoch: " << now << endl;
    cout << "milliseconds since epoch: " << mnow << endl << endl;

    return EXIT_SUCCESS;
}
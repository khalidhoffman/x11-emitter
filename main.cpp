#include <cstdlib>
#include <iostream>
#include "xutils/XWatchDaemon.h"

using namespace std;

int main() {
    cout<<"listening..."<<endl;
    XWatchDaemon *daemon = new XWatchDaemon();
    daemon->start();
    return 0;
}
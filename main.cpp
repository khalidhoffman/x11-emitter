#include <cstdlib>
#include <iostream>
#include "xutils/XWatcherDaemon.h"

using namespace std;

int main() {
    cout<<"listening..."<<endl;
    XWatcherDaemon *daemon = new XWatcherDaemon();
    daemon->start();
    return 0;
}
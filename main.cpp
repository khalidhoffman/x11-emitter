#include <cstdlib>
#include <iostream>
#include "mongo/bson/bson.h"
#include "mongodb/MongoDB.h"
#include "xutils/XWatcherDaemon.h"

using namespace std;


void printArray(char** arr, int numOfElements){

    for (int index = 0; index < numOfElements; index++){
        cout << arr[index] << endl;
    }
}

int main() {
    cout << "starting..." << endl;
//    MongoDB* dpPtr = new MongoDB();
//    MongoDB& mongoDB = *dpPtr;
//    bool isConnected = mongoDB.connect();
//    if(isConnected){
//        cout << "successfully connected to: " << mongoDB.getDatabaseName() << endl;
//    } else {
//        cout << "failed to connect to: " << mongoDB.getDatabaseName() << endl;
//    }
//    auto_ptr<mongo::DBClientCursor> cursor = mongoDB.query("com_khalidhoffman_mongodb.Deadlines", mongo::BSONObj());
//
//    while (cursor->more()) {
//        cout << cursor->next().toString() << endl;
//    }
//    WindowWatcher *watcher = new WindowWatcher();
//    unsigned long numOfWindows;
//
//    char **windowsV2;
//    watcher->getWindowsV2(windowsV2, numOfWindows);
//    cout << "V2 found " << numOfWindows << " windows." << endl;
//    printArray(windowsV2, numOfWindows);

//    char **windowsV3;
//    WindowMonitor *wMonitor = new WindowMonitor();
//    cout << "V3:" << endl;
//    wMonitor->getWindows();

    cout<<"listening..."<<endl;
    XWatcherDaemon *daemon = new XWatcherDaemon();
    daemon->start();
//    cout << "V3 found " << numOfWindows << " windows." << endl;
//    printArray(windowsV2, numOfWindows);
    return 0;
}
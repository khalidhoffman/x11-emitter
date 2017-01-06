//
// Created by kah8br on 8/24/15.
//

#ifndef TIMEWATCHER_XWATCHERDAEMON_H
#define TIMEWATCHER_XWATCHERDAEMON_H

#include <iostream>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xmu/WinUtil.h>
#include <X11/Xos.h>
#include <X11/Xutil.h>

struct XWatchDaemonState{
    bool isRunning, isInitialized, shouldUpdate;
    char* prevWindowName = (char*)"";
};

class XWatcherDaemon {

public:
    XWatcherDaemon();
    ~XWatcherDaemon();

    bool isRunning();
    void start();
    void stop();

private:

    Display* display;

    Window namedClientWindow, topFocusedWindow, focusWindow, subClientWindow, clientWindow, rootWindow;

    Display *openDisplay();

    XWatchDaemonState* state;

    void init();

    void run();

    void update();

    void notify();

    bool hasValidData();

    bool isInitialized();

    void onWindowChange();

    void updateFocusWindow();

    void updateTopFocusedWindow();

    void updateSubClientWindow();

    void updateClientWindow();

    void updateNamedWindow();

    void updateRootWindow();

    Window getRootWindow();

    char* searchWindowChildrenForName(Window window);

    char* getClientWindowName();

    void printWindowInfo(Window w);

    void printWindowClass(Window w);

    void printWindowName(Window w);

    void printWindows();
};


#endif //TIMEWATCHER_XWATCHERDAEMON_H

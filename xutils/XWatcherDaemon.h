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

class XWatcherDaemon {

public:
    XWatcherDaemon();

    ~XWatcherDaemon();
    bool isDaemonRunning();
    void start();
    void quit();
    void updateWindows();

private:
    Display *display;
    bool isRunning, isInit = false, shouldUpdate;

    Window namedWindow, topFocusedWindow, focusWindow, subClientWindow, clientWindow;

    char *searchForName(Window window);

    Display *get_client_display(Window window);

    Display *open_display();

    Window getNamedSubWindow(Window window);

    Window get_root_window();

    Window getClientWindow(Window window);

    Window getParentWindow(Window start);

    Window get_focus_window();

    void onQuit();
    void init();
    void captureWindowChange();
    void stop();

    void print_window_info(Window w);

    void print_window_class(Window w);

    void print_window_name(Window w);

    void printWindows();

    void sendString(char *windowName, Window window);

//    void sendV8String(char *windowName, Window window);

    char *getClientWindowName(Window w);

    Window Window_With_Name(Window top, const char *name);

    void updateDaemon();

    void processWindow();

    void processWindowName(char *windowName, Window window);
};


#endif //TIMEWATCHER_XWATCHERDAEMON_H

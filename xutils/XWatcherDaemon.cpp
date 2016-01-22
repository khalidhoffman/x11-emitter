#include "XWatcherDaemon.h"

//using namespace v8;

Bool xerror = false;
const unsigned argc = 1;
char* prevWindowName = (char*)"";
Window prevParentWindowId = NULL;

int handle_error(Display* display, XErrorEvent* error){
    char* errorDescription;
    int errorDescriptionLength;
//    XGetErrorText(display, error->error_code, errorDescription, errorDescriptionLength);
//    printf("ERROR: X11 error\n");
    xerror = True;
    return 1;
}

XWatcherDaemon::XWatcherDaemon() {
}

XWatcherDaemon::~XWatcherDaemon(){
    XFlush(this->display);
    XUngrabServer(this->display);
    XCloseDisplay(this->display );
}

void XWatcherDaemon::updateWindows(){
    xerror = false;
    do {
        this->focusWindow = this->get_focus_window();
    } while(this->focusWindow == NULL);
    this->topFocusedWindow = this->getParentWindow(this->focusWindow);
    this->namedWindow = this->getNamedSubWindow(this->topFocusedWindow);
    if(!xerror){
        this->subClientWindow = XmuClientWindow (this->display, this->namedWindow);
        this->clientWindow = this->getClientWindow(this->subClientWindow);
    } else{
        this->updateWindows();
    }
}

void XWatcherDaemon::init(){
    this->display = open_display();
    this->isInit = true;
    this->isRunning = false;
    this->shouldUpdate = true;

    XSetErrorHandler(handle_error);

    // for XmbTextPropertyToTextList
    setlocale(LC_ALL, ""); // see man locale
}

void XWatcherDaemon::start(){
    if(!this->isInit){
        this->init();
    }
    while(this->shouldUpdate){
        this->updateWindows();
        this->captureWindowChange();
    }
}

void XWatcherDaemon::captureWindowChange(){
    XEvent event;
    this->isRunning = true;
    if(this->namedWindow == NULL){
        this->shouldUpdate = true;
        return;
    }

//    long eventmask = PropertyChangeMask | FocusChangeMask | StructureNotifyMask | SubstructureNotifyMask | EnterWindowMask | LeaveWindowMask;;
    long eventmask = PropertyChangeMask | FocusChangeMask ;
//    long eventmask = FocusChangeMask;

    XSelectInput(this->display, this->namedWindow, eventmask);
    char* name;
    do {
//        this->printWindows();
//        XWindowEvent(display, root_win, eventmask, &event);
        XNextEvent(this->display, &event);

        switch (event.type) {
            case FocusOut:
                this->updateDaemon();
                break;
            case FocusIn:
            case PropertyNotify:
                this->processWindow();
//                name = searchForName(this->topFocusedWindow);
//                if( name == NULL){
//                    name = getClientWindowName(this->clientWindow);
//                }
//                if( name == NULL){
//                    name = (char *) "(null name)";
//                };
//                this->processWindowName(name);
                break;
            default:
                std::cout << "other: "<< event.type << std::endl;

        }
    } while (this->isRunning);
}

void XWatcherDaemon::processWindow(){
    char* name = searchForName(this->topFocusedWindow);
    if( name == NULL){
        name = getClientWindowName(this->clientWindow);
    }
    if( name == NULL){
        name = (char *) "(null name)";
    };
//    if(getParentWindow(this->clientWindow) != prevParentWindowId){
//        prevParentWindowId = getParentWindow(this->clientWindow);
//    }
    this->processWindowName(name, getParentWindow(this->clientWindow));
}

void XWatcherDaemon::processWindowName(char* windowName, Window window){
//    printf("%s == %s ? %i \n\n", windowName, prevWindowName, strcmp (windowName, prevWindowName));
    if( strcmp (windowName, prevWindowName) != 0 ) {
        prevWindowName = windowName;
        sendString(prevWindowName, window);
    }
}

void XWatcherDaemon::sendString(char* windowName, Window window){
    printf("%s - %i\n", windowName, window);
}

//void XWatcherDaemon::sendString(char* windowName, Window window){
//    Local<Object> obj = Object::New(isolate);
//    obj->Set(String::NewFromUtf8(isolate, "raw"), String::NewFromUtf8(this->isolate, windowName));
//    obj->Set( String::NewFromUtf8(isolate, "parent"), Number::New(this->isolate, window));
//    Local<Value> argumentVector[argc] = { obj };
//    this->callbackFunction->Call(this->isolate->GetCurrentContext()->Global(), argc, argumentVector);
//}

void XWatcherDaemon::quit(){
    this->isRunning = false;
    this->shouldUpdate = false;
}

void XWatcherDaemon::stop(){
    this->isRunning = false;
}

void XWatcherDaemon::updateDaemon(){
    this->shouldUpdate = true;
    this->stop();
};

void XWatcherDaemon::onQuit(){
    if(this->shouldUpdate){
        this->updateWindows();
        this->start();
    }
}

char* XWatcherDaemon::searchForName(Window window){
    Window returnedRoot,
            returnedParent,
            *children;
    unsigned int numOfChildWindows;
    XQueryTree(this->display, window, &returnedRoot, &returnedParent, &children, &numOfChildWindows);
    char *name = NULL;
    int status;
    if(numOfChildWindows == 1){
        status = XFetchName(this->display, children[0], &name);
        if(status >= Success && name!= NULL && strlen(name) > 1){
            // successfully retrieved name
        } else {
            name = this->searchForName( children[0]);
        }
    } else {
        for(int i = 0; i < numOfChildWindows; i++) {
            name = this->searchForName(children[i]);
        }
    }
    if(children) {
        XFree(children);
    }
    return name;
}

Window XWatcherDaemon::get_root_window(){
    Window w;
    int screen_num = DefaultScreen(this->display);
    Screen *screen = XScreenOfDisplay(this->display, screen_num);
    Window root_win = RootWindow(this->display, XScreenNumberOfScreen(screen));
    return root_win;
}

Display* XWatcherDaemon::open_display(){
    char* displayName = NULL;
    Display *display = XOpenDisplay(displayName);
    if(display == NULL){
        printf("fail -(XWatcherDaemon::open_display)\n");
        exit(1);
    }
    return display;
}

Window XWatcherDaemon::get_focus_window(){
    Window w;
    int focusState;
    do {
        XGetInputFocus(this->display, &w, &focusState); // see man
    } while( w== NULL);
    if(xerror){
        // xerror
    }else if(w == None){
        // there is no focus window
    }

    return w;
}

// get the top window.
// a top window have the following specifications.
//  * the start window is contained the descendent windows.
//  * the parent window is the root window.
Window XWatcherDaemon::getParentWindow(Window start){
    Window w = start;
    Window parent = start;
    Window root = None;
    Window *children;
    unsigned int nchildren;
    Status s;

    if(xerror){
        return this->get_root_window();
    }

    while (parent != root) {
        w = parent;
        s = XQueryTree(this->display, w, &root, &parent, &children, &nchildren); // see man

        if (s)
            XFree(children);

        if(xerror){
            return this->get_root_window();
        }
    }

    return w;
}

// search a named window (that has a WM_STATE prop)
// on the descendent windows of the argment Window.
Window XWatcherDaemon::getNamedSubWindow(Window window){
    Window w;
//    printf("getting named window ... ");
    if(xerror){
        return window;
    }
    w = XmuClientWindow(this->display, window); // see man
    if(w == window || w == 0) {
        return window;
    }
    return w;
}


Window XWatcherDaemon::getClientWindow(Window window) {
    Window* leader = NULL;
    unsigned long nitems = 0;
    unsigned char* data = 0;
    Atom actual_type;
    Atom xProperty = XInternAtom(this->display, "WM_CLIENT_LEADER" , true);
    int actual_format;
    unsigned long bytes;
    // WM_CLIENT_LEADER is an interned Atom for the WM_CLIENT_LEADER property
    int status = XGetWindowProperty (this->display,
                                     window,
                                     xProperty,
                                     0L,
                                     (~0L),
                                     False,
                                     AnyPropertyType,
                                     &actual_type,
                                     &actual_format,
                                     &nitems,
                                     &bytes,
                                     &data);

    if (status != Success ) {
//        std::cout << "Unsucessfully grabed WM_CLIENT_LEADER." << std::endl;
        return window;
    }

    if (actual_type == None) {
//        std::cout << "WM_CLIENT_LEADER actual Type is equal to none." << std::endl;
        return window;
    }

    if (status != Success || actual_type == None) {
        return (*leader);
    }
    leader = reinterpret_cast<Window*> (data);
    return (*leader);
}

bool XWatcherDaemon::isDaemonRunning() {
    return this->isRunning;
}

// (XFetchName cannot get a name with multi-byte chars)
void XWatcherDaemon::print_window_name(Window w){
    XTextProperty prop;
    Status s;

    s = XGetWMName(this->display, w, &prop); // see man
    if(!xerror && s){
        int count = 0, result;
        char **list = NULL;
        result = XmbTextPropertyToTextList(this->display, &prop, &list, &count); // see man
        if(result == Success){
            printf("\t%s\n", list[0]);
        }else{
            printf("ERROR: XmbTextPropertyToTextList\n");
        }
    }else{
        printf("ERROR: XGetWMName\n");
    }
}

// (XFetchName cannot get a name with multi-byte chars)
char* XWatcherDaemon::getClientWindowName(Window w){
    XTextProperty prop;
    Status s;
    char* name = (char*)"n/a";

    s = XGetWMName(this->display, w, &prop); // see man
    if(!xerror && s){
        int count = 0, result;
        char **list = NULL;
        result = XmbTextPropertyToTextList(this->display, &prop, &list, &count); // see man
        if(result == Success){
//            printf("\t%s\n", list[0]);
            name = list[0];
        } else{
            printf("ERROR: XmbTextPropertyToTextList\n");
        }
    } else if(xerror){
        printf("ERROR: XGetWMName\n");
    } else if(s == NULL){
        printf("Couldn't get name of client window.\n");
    }
    return name;
}

void XWatcherDaemon::printWindows(){
    std::cout << "namedWindow: " << this->namedWindow << std::endl;
    std::cout << "topFocusedWindow: " << this->topFocusedWindow << std::endl;
    std::cout << "focusWindow: " << this->focusWindow << std::endl;
    std::cout << "subClientWindow: " << this->subClientWindow << std::endl;
    std::cout << "clientWindow: " << this->clientWindow << std::endl;
}

void XWatcherDaemon::cPrintWindows(){
    printf("namedWindow: ", this->namedWindow );
    printf("topFocusedWindow: ", this->topFocusedWindow );
    printf("focusWindow: ", this->focusWindow );
    printf("subClientWindow: ", this->subClientWindow );
    printf("clientWindow: ", this->clientWindow );
}


Window XWatcherDaemon::Window_With_Name( Window top, const char *name){
    Window *children, dummy;
    unsigned int nchildren;
    int i;
    Window w=0;
    char *window_name;

    if (XFetchName(this->display, top, &window_name) && !strcmp(window_name, name))
        return(top);

    if (!XQueryTree(this->display, top, &dummy, &dummy, &children, &nchildren))
        return(0);

    for (i=0; i<nchildren; i++) {
        w = this->Window_With_Name(children[i], name);
        if (w)
            break;
    }
    if (children) XFree ((char *)children);
    return(w);
}

void XWatcherDaemon::print_window_class(Window w){
    Status s;
    XClassHint*classhint;

    printf("application: \n");

    classhint = XAllocClassHint(); // see man
    if(xerror){
        printf("ERROR: XAllocClassHint\n");
    }

    s = XGetClassHint(this->display, w, classhint); // see man
    if(xerror || s){
        printf("\tname: %s\n\tclasshint: %s\n", classhint->res_name, classhint->res_class);
    }else{
        printf("ERROR: XGetClassHint\n");
    }
}

void XWatcherDaemon::print_window_info(Window w){
    printf("--\n");
    print_window_name(w);
    print_window_class(w);
}

Display* XWatcherDaemon::get_client_display(Window window) {
    unsigned long *data = NULL;
    Atom prop = XInternAtom(this->display, "_NET_WM_DESKTOP", False);
    Atom type_ret;
    int format_ret;
    unsigned long after_ret, count;
    int result = XGetWindowProperty(this->display,
                                    window,
                                    prop,
                                    0,
                                    0x7FFFFFFF,
                                    False,
                                    XA_CARDINAL,
                                    &type_ret,
                                    &format_ret,
                                    &count,
                                    &after_ret,
                                    (unsigned char **) &data);
    if ((result != Success)) {
//        std::cout << "Failed to find client display." << std::endl;
        if (data) {
            XFree(data);
        }
    } else{
        // successfully found client display
    }
    Display* clientDisplay = (Display*) data;
    return (Display*) data;
}
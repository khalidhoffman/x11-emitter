#include "XWatchDaemon.h"

// uncomment namespace for v8 compatability
//using namespace v8;

Bool xerror = false;

int handle_error(Display* display, XErrorEvent* error){
    char* errorDescription;
    int errorDescriptionLength;
    XGetErrorText(display, error->error_code, errorDescription, errorDescriptionLength);
    // handle error

/*    printf("ERROR: X11 error: %s\n", errorDescription);*/
    xerror = true;
    return 1;
}

XWatchDaemon::XWatchDaemon() {
    this->state = new XWatchDaemonState();
}

XWatchDaemon::~XWatchDaemon(){
    XFlush(this->display);
    XUngrabServer(this->display);
    XCloseDisplay(this->display );
}

void XWatchDaemon::init(){
    this->display = this->openDisplay();

    XSetErrorHandler(handle_error);

    // for XmbTextPropertyToTextList
    setlocale(LC_ALL, ""); // see man locale

    this->state->isInitialized = true;
}

void XWatchDaemon::start(){
    if(!this->isInitialized()){
        this->init();
    }
    this->state->shouldUpdate = true;
    this->run();
}

void XWatchDaemon::run(){
    // options include: PropertyChangeMask | FocusChangeMask | StructureNotifyMask | SubstructureNotifyMask | EnterWindowMask | LeaveWindowMask;;
    long eventmask = PropertyChangeMask | FocusChangeMask ;
    XEvent event;
    do {
        this->state->isRunning = true;
        if (this->hasValidData()){
            XSelectInput(this->display, this->namedClientWindow, eventmask);
            XNextEvent(this->display, &event);

            switch (event.type) {
                case FocusIn:
                case PropertyNotify:
                    this->onWindowChange();
                    break;
                default:
//                    std::cout << "other: "<< event.type << std::endl;
                    break;

            }
        }
        this->update();
    } while (this->state->shouldUpdate);
}

void XWatchDaemon::stop(){
    this->state->shouldUpdate = false;
}

void XWatchDaemon::update(){
    xerror = false;
    this->updateFocusWindow();
    this->updateTopFocusedWindow();
    this->updateNamedWindow();
    if (this->hasValidData() && !xerror){
        this->updateSubClientWindow();
        this->updateClientWindow();
    }
}

void XWatchDaemon::notify(){
    char* name = this->searchWindowChildrenForName(this->topFocusedWindow);
    if( name == NULL){
        name = this->getClientWindowName();
    }
    if( name == NULL){
        name = (char *) "(null name)";
    };
    if( strcmp (name, this->state->prevWindowName) != 0 ) {
        this->state->prevWindowName = name;
        printf("%s - %i\n", name, (int)this->clientWindow);

        /* replace printf with below for v8 compatibility */
        /*
        Local<Object> obj = Object::New(isolate);
        obj->Set(String::NewFromUtf8(isolate, "raw"), String::NewFromUtf8(this->isolate, name));
        obj->Set( String::NewFromUtf8(isolate, "parentWindow"), Number::New(this->isolate, this->topFocusedWindow));
        obj->Set( String::NewFromUtf8(isolate, "clientWindow"), Number::New(this->isolate, this->clientWindow));
        obj->Set( String::NewFromUtf8(isolate, "namedClientWindow"), Number::New(this->isolate, this->namedClientWindow));
        obj->Set( String::NewFromUtf8(isolate, "focusWindow"), Number::New(this->isolate, this->focusWindow));
        Local<Value> argumentVector[argc] = { obj };
        this->callbackFunction->Call(this->isolate->GetCurrentContext()->Global(), argc, argumentVector);
        */
    }
}

bool XWatchDaemon::isInitialized() {
    return this->state->isInitialized;
}

bool XWatchDaemon::isRunning() {
    return this->state->isRunning;
}

bool XWatchDaemon::hasValidData(){
    return this->namedClientWindow != NULL;
}

void XWatchDaemon::onWindowChange(){
    this->notify();
}

void XWatchDaemon::updateFocusWindow(){
    Window focusWindow;
    int focusState;

    XGetInputFocus(this->display, &focusWindow, &focusState); // see man

    if(xerror){
        // xerror
    } else if(focusWindow == None){
        // there is no focus window
    }

    this->focusWindow = focusWindow;
}

// get the top window.
// a top window have the following specifications.
//  * the start window is contained the descendent windows.
//  * the parent window is the root window.
void XWatchDaemon::updateTopFocusedWindow(){
    if (xerror || this->focusWindow == NULL){
         this->topFocusedWindow = NULL;
    } else {
        Window highestFocusWindow = this->focusWindow;
        Window parent = this->focusWindow;
        Window root = None;
        Window *children;
        unsigned int nchildren;
        Status XQueryStatus;
        while(parent != root) {
            highestFocusWindow = parent;
            XQueryStatus = XQueryTree(this->display, highestFocusWindow, &root, &parent, &children, &nchildren); // see man

            if (XQueryStatus){
                XFree(children);
            }

            if(xerror){
                this->topFocusedWindow =this->getRootWindow();
                return;
            }
        };
        this->topFocusedWindow = highestFocusWindow;
    }


}

// search a named window (that has a WM_STATE prop)
// on the descendent windows of the argment Window.
void XWatchDaemon::updateNamedWindow(){
    Window topFocusedWindow = this->topFocusedWindow;
    Window namedClientWindow ;
//    printf("getting named window ... ");
    if(xerror || topFocusedWindow == NULL){
        this->namedClientWindow = NULL;
    } else {
        namedClientWindow = XmuClientWindow(this->display, topFocusedWindow); // see man

        if(namedClientWindow == topFocusedWindow || namedClientWindow == 0) {
            this->namedClientWindow = topFocusedWindow;
        } else {
            this->namedClientWindow =  namedClientWindow;
        }
    }
}

void XWatchDaemon::updateSubClientWindow(){
    if (this->namedClientWindow == NULL) {
        this->subClientWindow = NULL;
    } else {
        this->subClientWindow = XmuClientWindow (this->display, this->namedClientWindow);
    }
}

void XWatchDaemon::updateClientWindow() {
    if (this->subClientWindow == NULL) {
        this->clientWindow = NULL;
    } else {
        Window window = this->subClientWindow;
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
//        std::cout << "Unsuccessfully grabbed WM_CLIENT_LEADER." << std::endl;
            this->clientWindow = window;
        } else if (actual_type == None) {
//        std::cout << "WM_CLIENT_LEADER actual Type is equal to none." << std::endl;
            this->clientWindow =  window;
        } else {
            leader = reinterpret_cast<Window*> (data);
            this->clientWindow =  (*leader);
        }
    }

}

void XWatchDaemon::updateRootWindow(){
    int screen_num = DefaultScreen(this->display);
    Screen *screen = XScreenOfDisplay(this->display, screen_num);
    Window root_win = RootWindow(this->display, XScreenNumberOfScreen(screen));
    this->rootWindow = root_win;
}

Window XWatchDaemon::getRootWindow() {
    this->updateRootWindow();
    return this->rootWindow;
}

// (XFetchName cannot get a name with multi-byte chars)
char* XWatchDaemon::getClientWindowName(){
    char* name = (char*)"n/a";

    if (this->clientWindow == NULL){
        // ignore invalid request
//        printf("getClientWindowName(): invalid request\n");
    } else {
        Window w = this->clientWindow;
        XTextProperty prop;
        Status s;
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
    }
    return name;
}

Display* XWatchDaemon::openDisplay(){
    char* displayName = NULL;
    Display *display = XOpenDisplay(displayName);
    if(display == NULL){
        printf("fail -(XWatchDaemon::openDisplay)\n");
        exit(1);
    }
    return display;
}

char* XWatchDaemon::searchWindowChildrenForName(Window window){
    Window returnedRoot,
            returnedParent,
            *children;

    unsigned int childrenCount;
    char *name = NULL;

    XQueryTree(this->display, window, &returnedRoot, &returnedParent, &children, &childrenCount);

    if (xerror) {

    } else if(childrenCount> 0){
        // reverse order because highest windows are last in children
        for(int i = childrenCount - 1; i >= 0; i--) {
            if (name!= NULL && strlen(name) > 1) {
                // quit early if a name was found
                break;
            } else if (children[i] != NULL){
                // otherwise keep searching valid children recursively

                int fetchNameResult = XFetchName(this->display, children[i], &name);
                if(fetchNameResult >= Success && name!= NULL && strlen(name) > 1){
                    // successfully retrieved name
                } else {
                    name = this->searchWindowChildrenForName(children[i]);
                }
            }
        }
    }
    XFree(children);
    return name;
}

// (XFetchName cannot get a name with multi-byte chars)
void XWatchDaemon::printWindowName(Window w){
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

void XWatchDaemon::printWindows(){
    std::cout << "namedClientWindow: " << this->namedClientWindow << std::endl;
    std::cout << "topFocusedWindow: " << this->topFocusedWindow << std::endl;
    std::cout << "focusWindow: " << this->focusWindow << std::endl;
    std::cout << "subClientWindow: " << this->subClientWindow << std::endl;
    std::cout << "clientWindow: " << this->clientWindow << std::endl;
}

void XWatchDaemon::printWindowClass(Window w){
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

void XWatchDaemon::printWindowInfo(Window w){
    printf("--\n");
    printWindowName(w);
    printWindowClass(w);
}


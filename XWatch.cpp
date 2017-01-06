// addon.cc

#include <node.h>
#include <stdlib.h>
#include <vector>
#include <string.h>
#include "xutils/XWatchDaemon.h"

using namespace v8;

void RunCallback(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    Local<Function> cb = Local<Function>::Cast(args[0]);
    const unsigned argc = 1;
    unsigned long numOfWindows;
    char **windowsV2;
    XWatchDaemon *daemon = new XWatchDaemon(cb, isolate);
    daemon->start();
}

void init(Handle<Object> exports, Handle<Object> module) {
    NODE_SET_METHOD(module, "exports", RunCallback);
}

NODE_MODULE(addon, init)
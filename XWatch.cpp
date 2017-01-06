// addon.cc

#include <node.h>
#include <stdlib.h>
#include <vector>
#include <string.h>
#include "xutils/XWatchDaemon.h"

using namespace v8;

void runCallback(const FunctionCallbackInfo<Value>& args) {
    const unsigned argc = 1;
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    Local<Function> callback = Local<Function>::Cast(args[0]);
    XWatchDaemon *daemon = new XWatchDaemon(callback, isolate);
    daemon->start();
}

void init(Handle<Object> exports, Handle<Object> module) {
    NODE_SET_METHOD(module, "exports", runCallback);
}

NODE_MODULE(addon, init)
# x11-activity-monitor
Outputs window focus changes in X11

Can be used with node.js to log activity on x11 environments (unix, linux, ubuntu, etc.)

## How to build for V8

### on Linux
1. `apt-get install python make gcc` (may require `sudo` privileges)
2. `npm install`

## How to use in node.js
```
const X11Emitter = require('./build/Release/x11-emitter');

X11Emitter(function (msg) {
    // do something with msg json object
});

```

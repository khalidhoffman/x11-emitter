# x11-activity-monitor
Outputs window focus changes in X11

Can be used with node.js to log activity on x11 environments (unix, linux, ubuntu, etc.)

## How to build for V8

1. `apt-get install python make gcc` (may require `sudo` privileges)
2. `npm i -g node-gyp` (may require `sudo` privileges)
3. `node-gyp configure`
4. `node-gyp build`

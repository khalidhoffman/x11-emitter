{
  "targets": [
    {
      "target_name": "X11Emitter",
      "sources": [ "XWatcherDaemon.h", "XWatcherDaemon.cpp", "XWatcher.cpp" ],
      "libraries": ["-lX11", "-lXmu"],
      "conditions": [
        ["OS=='linux'", {
          "cflags": [ "-w" ]
        }]
      ]
    }
  ]
}
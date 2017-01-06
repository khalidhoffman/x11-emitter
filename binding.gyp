{
  "targets": [
    {
      "target_name": "x11-emitter",
      "sources": [ "xutils/XWatchDaemon.h", "xutils/XWatchDaemon.cpp", "./XWatch.cpp" ],
      "libraries": ["-lX11", "-lXmu"],
      "conditions": [
        ["OS=='linux'", {
          "cflags": [ "-w" ]
        }]
      ]
    }
  ]
}

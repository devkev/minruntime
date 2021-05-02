minruntime
==========

Kevin Pulo, kev@pulo.com.au

https://github.com/devkev/minruntime/

Copyright (c) 2021

Licensed under the GNU GPL v3 or higher, as per the COPYING file.


SUMMARY
-------

An LD_PRELOAD library to ensure the process stays running for at least the given minimum duration.  The extra time, if any, occurs after the main program itself has ended/exited.

Example use case: On Ubuntu 18.04, NetworkManager has a bug where async auth checks (via polkit) aren't cancelled when the client (nmcli) exits (or results ignored if they're received after the client has gone away).  Since nmcli usually only runs briefly, its process has already finished by the time polkit gets around to checking its auth.  Polkit then (correctly) returns auth failures (because it gets `ENOENT No such file or directory` when trying to open /proc/pid/stat or /proc/pid/status), and NetworkManager (incorrectly) spams these failures into the system logs.  The problem doesn't occur in Ubuntu 20.04, and the affected code path has been completely rewritten in recent NetworkManager.  So, the least-invasive workaround is to make nmcli invocations run just a little bit longer (500ms is usually enough), instead of exiting immediately.


BUILD / INSTALL
---------------

Standard autoconf process:

    ./configure
    make
    make install


USAGE
-----

To use, add a reference to `libminruntime.so` to the start of `$LD_PRELOAD`, and then set `$MINRUNTIME_SECS` and `$MINRUNTIME_NSECS` to specify the minimum duration for the program (as seconds and nanoseconds, respectively).  If either is omitted, it defaults to a value of 0.  Only affects the next process, and not processes that it may subsequently fork (ie. removes itself from `$LD_PRELOAD`).


EXAMPLES
--------

* Basic demo/testing:

    ```
    $ time sleep 0.1
    real    0m0.110s
    user    0m0.006s
    sys     0m0.005s
    $ time LD_PRELOAD=/usr/local/lib/libminruntime.so MINRUNTIME_NSECS=500000000 sleep 0.1
    real    0m0.524s
    user    0m0.014s
    sys     0m0.014s
    $ time LD_PRELOAD=/usr/local/lib/libminruntime.so MINRUNTIME_NSECS=500000000 sleep 0.7
    real    0m0.724s
    user    0m0.008s
    sys     0m0.019s
    $ time LD_PRELOAD=/usr/local/lib/libminruntime.so MINRUNTIME_SECS=1 MINRUNTIME_NSECS=500000000 sleep 0.7
    real    0m1.518s
    user    0m0.007s
    sys     0m0.014s
    $ time LD_PRELOAD=/usr/local/lib/libminruntime.so MINRUNTIME_SECS=2 sleep 0.7
    real    0m2.024s
    user    0m0.010s
    sys     0m0.017s
    $ time LD_PRELOAD=/usr/local/lib/libminruntime.so MINRUNTIME_SECS=2 sleep 2.3
    real    0m2.324s
    user    0m0.012s
    sys     0m0.016s
    ```

* `nmcli` wrapper script (using [shwrapnel](https://github.com/devkev/shwrapnel)):

    ```
    $ type -a nmcli
    nmcli is /usr/local/bin/nmcli
    nmcli is /usr/bin/nmcli
    $ cat /usr/local/bin/nmcli
    #!/bin/bash
    
    . shwrapnel
    
    export LD_PRELOAD=/usr/local/lib/libminruntime.so
    
    export MINRUNTIME_NSECS=500000000
    
    shwrapnel launch
    
    $ type shwrapnel
    shwrapnel is /usr/local/bin/shwrapnel
    $ ls -la /usr/local/bin/shwrapnel
    -rw-r--r-- 1 root root 8787 Oct  8  2014 /usr/local/bin/shwrapnel
    $
    $ tail /var/log/syslog | grep -c 'error requesting auth for'
    0
    $ time nmcli general
    STATE      CONNECTIVITY  WIFI-HW  WIFI     WWAN-HW  WWAN
    connected  full          enabled  enabled  enabled  enabled
    
    real    0m0.572s
    user    0m0.066s
    sys     0m0.035s
    $ tail /var/log/syslog | grep -c 'error requesting auth for'
    0
    $ time /usr/bin/nmcli general
    STATE      CONNECTIVITY  WIFI-HW  WIFI     WWAN-HW  WWAN
    connected  full          enabled  enabled  enabled  enabled
    
    real    0m0.080s
    user    0m0.039s
    sys     0m0.024s
    $ tail /var/log/syslog | grep -c 'error requesting auth for'
    10
    ```


LIMITATIONS
-----------

- Only dynamically linked binaries.

- SELinux/Apparmor can get in the way, denying permission to map the dynamic library (necessary to load it) and giving an error message like:
    ```
    ERROR: ld.so: object '/usr/local/lib/libminruntime.so' from LD_PRELOAD cannot be preloaded (failed to map segment from shared object): ignored.
    ```


FEEDBACK
--------

Comments, feature suggestions, bug reports, patches, etc are most welcome, please send them to Kevin Pulo <kev@pulo.com.au>.


## NOTE:

Developement of pinthread is on https://git.devuan.org/packages-base/pinthread
Here on github there is only a stub and it can be outdated.

For any issue, merge request or other suggestion, please
use Devuan gitlab.

## pinthread

pinthread is a little library intended for use with LD_PRELOAD environment var.

I've wrote it cause in devuan we intesively use qemu-static to build packages for 
different archs, but it doesn't work well with some processes using threads
when the base host is multicore, so, this library will override the 
pthread_create() call and will pin the threads and the main process
to run only on a specific core, making qemu happy and avoiding a lot 
of segfaults.

### Build:
 
just type make and, optionally, make install

It will build a library (pinthread.so) and a binary
(dotprod_mutex)

dotprod_mutex is just a test binary you can use for testing the library. 

### Usage:

 LD_PRELOAD=/path/to/pinthread.so  [binary_to_launch]
 
 Environment variables:
   * PINTHREAD_CORE: set to the core number you want to pin to.
                     if omitted, it defaults to the result of 
                     sched_getcpu()

   * PINTHREAD_PNAMES: accept a space separated list of strings,
                       if any string match the process name or
                       PINTHREAD_PNAMES is empty/omitted, it will 
                       pin all thread according to PINTHREAD_CORE, 
                       otherwise it will just call the "real" 
                       pthread_create without any pinning.

### Known bugs:

 When used in qemu-static for arm* architectures setting
 PINTHREAD_CORE is mandatory as internally we use the 
 sched_getcpu() syscall that isn't (yet?) supported in qemu.

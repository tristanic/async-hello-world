A super-simple "Hello World" style demonstration of the use of C++11 threading
via std::async and how to interact with it via Python.

Due to Python's Global Interpreter Lock (GIL), threading within Python itself
is very limited in its functionality. Mostly, it should be limited to tasks like
file or network I/O, where the majority of the time is taken up by a single call
that releases the GIL (i.e. to open and read a file, or to run a long
computation in pure C/C++). Even then, it tends to be quite fiddly to get right,
and the overhead of creating and destroying threads is quite large. In many
cases, a better approach is to do your threading outside of Python entirely:
as long as you are working on objects outside of the Python API (e.g. the data
contained within a NumPy array), there is no need for Python to know about your
threads at all. With C++11 and std::async this can be quite easy.

In `simple_async.h` we have a simple C++ class `Foo` implementing a function,
`do_something_slow()`. This can be fairly trivially wrapped using std::async to
make a function  `do_something_slow_asynchronously()` which simply runs
`do_something_slow()` in a separate thread, setting a Boolean flag so that the
main thread knows when  it's running and when it's done.
`do_something_slow_asynchronously()` returns a `std::future` object whose role
it is to provide the result of `do_something_slow()` once the thread is
finished.

There are (at least) two ways to use this in the Python environment:

1. You have a set of long-running tasks that each need to be run before you can
get your final result, but each task is independent of the others. Simply
call each task's `do_something_slow_asynchronously()` method, then do a loop
over them calling their `get_asynchronous_result()` (your main thread will hang
until they're all finished, but the overall speed should be much faster than
the original serial implementation).

2. You have a task that runs for a long time ("a long time" may be a few seconds
if it only has to run once, or just 20 milliseconds if it has to run repeatedly)
and you don't want to interrupt the main thread (i.e. significantly reduce the
frame rate or hang the graphics entirely). Here, the best approach is to
call `do_something_slow_asynchronously()`, then add a callback (to run once in
each iteration of the main event loop) which checks `thread_done()` and, if it
returns `True`, runs `get_asynchronous_result()` and does something with it
before cleaning itself up. An example of this approach in ChimeraX is
demonstrated in `__init__.py` with `do_something_slow_with_callbacks()`.

To build:

Make sure the PyBind11 headers are in your include path (or in C:\include in
Windows). Building in Linux requires at least GCC 4.9. If you want to do
something more complicated that links with the ChimeraX C++ libraries or want to
make sure your code will run on most other Linux distributions, then use
`devtoolset-3` in CentOS 7 (as at the time of writing - a move to a more recent
`devtoolset` is expected in the near future).

Change to the directory containing `bundle_info.xml`.

(Linux or MacOS) if you have a release version of ChimeraX:

`export RELEASE=1`

(Linux) `scl enable devtoolset-3 bash`

`make app-install`

(Windows)
`"C:\Program Files\ChimeraX\bin\ChimeraX-console.exe" --nogui --cmd "devel install .; exit"`

Then, start ChimeraX. To see the effects of different actions, it's helpful to
open a model and set it continuously rotating ("roll" on the ChimeraX command
line). Then, open the shell (Tools/General/Shell):

```
from chimerax.simple_async import simple_async, do_something_slow_with_callbacks
foo = simple_async.AsyncHelloWorld()
foo.do_something_slow(2)
        # Will hang the GUI for 2 seconds before printing
foo.do_something_slow_asynchronously(10) # No hang, but ChimeraX won't
        # automatically know about the result
foo.thread_done()
        # Will return False while the thread is running
foo.get_asynchronous_result() # Will return immediately if thread is finished,
        # otherwise will hang until it's done. If you never called
        # do_something_slow_asynchronously(), this will return an error.
do_something_slow_with_callbacks(session, foo, 5)
        # Will print the thread result to the log after 5 seconds, with no
        # interruption to the GUI.
```

One very important point if you wish to use this approach in earnest within
ChimeraX: operations on its core C++ `atomstruct` objects (`Atom`, `Bond`,
`Residue` etc.) are **not** thread safe. If you wish to have a thread running
while you don't have complete control over what's happening in the main thread
(e.g. across multiple iterations of the main graphics loop) you should make
copies of the properties your function needs. Otherwise, deletion of an atom
while your thread is running will kill the application.

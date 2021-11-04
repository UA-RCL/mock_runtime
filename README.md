# Mock Runtime
In this repository is a mock runtime for use in developing applications that can be run in heterogeneous runtimes that rely on dynamic dispatch of application kernels.

The runtime model assumed here is as follows:
- The user would like to run functions that can be dynamically scheduled and dispatched to heterogeneous hardware units without needing to perform this mapping themselves.
- As such, they would rather rely on APIs that can encode computational kernels and then have the runtime determine the optimal means for dispatching that kernel based on the system state.
- However, they would also not like to have their application tied to using this runtime and would like to fall back to "CPU-only" API implementations that allow them to bypass needing a runtime at all without any code changes on their end.

To help enable these goals, this repository is composed of three main parts:
- **mock\_runtime**: this mock runtime provides a mock version of a heterogeneous runtime that can accept these "dynamically dispatched" application implementations.
- **test\_app**: a test application that uses the provided set of APIs to perform some computations
- **libdash**: the kernel library that actually provides all of the API implementations.

## Basic Usage

The basic way of using this repository is as follows:

### Step 1: Build and test application using provided APIs 
1. First, build an application that uses the APIs provided in `libdash/dash.h`.
Then, if wanting to just test that your application works as expected (standalone, without using the **mock_runtime**), begin by building the `CMake` project in `libdash` folder to produce a static library archive of CPU-only function implementations of the API calls (`libdash.a`).

    ```bash
    cd libdash
    mkdir build
    cd build
    cmake ../
    make
    ```

2. After this, compile your application and link against `libdash.a` produced in this `libdash/build` directory.
One open issue is that you also need to link against `libmath` (i.e. `-lm`) as it has issues being statically compiled into `libdash.a`.
With this, you should have a standalone binary that can be used to at least test that your application is functional.

    As an example, the test application provided in `test_app` folder can be compiled into a standalone binary using the `test_app/Makefile` with the following commands (from repository root):

    ```bash
    cd test_app
    make test_app.out
    ```
    The generated `test_app.out` file can then be executed as a standalone executable binary as `./test_app.out`, which should generate the following output:

    ```
    Array 1: 0.000000 1.000000 2.000000 3.000000 4.000000 5.000000 6.000000 7.000000 8.000000 9.000000 
    Array 2: 0.000000 1.000000 2.000000 3.000000 4.000000 5.000000 6.000000 7.000000 8.000000 9.000000 
    Output: 0.000000 2.000000 4.000000 6.000000 8.000000 10.000000 12.000000 14.000000 16.000000 18.000000 
    ```

### Step 2: Run and test the API-based application in mock\_runtime
Next, you can move on to testing your application's behavior in the provided **mock_runtime**.

1. First build the `mock_runtime` binary from the repository root

    ```bash
    mkdir build
    cd build
    cmake ../
    make
    ```

2. Now, rather than compile your application as a standalone binary, compile it as a shared object, and do not link against any libdash implementation. Instead, there is a different version that is compiled and linked by the runtime itself.
After this, you can test your application with
    ```bash
    ./mock_runtime ./my_app.so
    ```
    And the behavior should match what was observed with the standalone execution.

    For example, the test application in `test_app` folder can be compiled for execution in `mock_runtime` using the `test_app/Makefile` in the following manner (from repository root)

    ```bash
    cd test_app
    make test_app.so
    ```
    Running this generated `test_app.so` through `mock_runtime` should produce the following output

    ```
    [cedr] Launching 1 instances of the received application!
    [nk] I am inside the runtime's codebase, unpacking my args to enqueue a new ZIP task
    [nk] I have finished initializing my ZIP node, pushing it onto the task list
    [nk] I have pushed a new task onto the work queue, time to go sleep until it gets scheduled and completed
    [cedr] I have a task to do!
    [cedr] It is time for me to process a node named ZIP
    [cedr] Calling the implementation of this node
    [cedr] Execution is complete. Triggering barrier so that the other thread continues execution
    [cedr] Barrier finished, going to delete this task node now
    Array 1: 0.000000 1.000000 2.000000 3.000000 4.000000 5.000000 6.000000 7.000000 8.000000 9.000000 
    Array 2: 0.000000 1.000000 2.000000 3.000000 4.000000 5.000000 6.000000 7.000000 8.000000 9.000000 
    Output: 0.000000 2.000000 4.000000 6.000000 8.000000 10.000000 12.000000 14.000000 16.000000 18.000000 
    [nk] I am inside the runtime's codebase, injecting a poison pill to tell the host thread that I'm done executing
    [nk] I have pushed the poison pill onto the task list
    [cedr] I have a task to do!
    [cedr] I received a poison pill task!
    [cedr] Time to break out of my loop and die...
    [cedr] The worker thread has joined, shutting down...
    ```
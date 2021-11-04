# Mock Runtime
In this repository is a mock runtime for use in developing applications that can be run in heterogeneous runtimes that rely on dynamic dispatch of application kernels.

The runtime model assumed here is as follows:
- The user would like to run functions that can be dynamically scheduled and dispatched to heterogeneous hardware units without needing to perform this mapping themselves.
- As such, they would rather rely on APIs that can encode computational kernels and then have the runtime determine the optimal means for dispatching that kernel based on the system state.
- However, they would also not like to have their application tied to using this runtime and would like to fall back to "CPU-only" API implementations that allow them to bypass needing a runtime at all without any code changes on their end.

To help enable these goals, this repository is composed of three main parts:
- mock\_runtime: this mock runtime provides a mock version of a heterogeneous runtime that can accept these "dynamically dispatched" application implementations.
- test\_app: a test application that uses the provided set of APIs to perform some computations
- libdash: the kernel library that actually provides all of the API implementations.

## Basic Usage

The basic way of using this repository is as follows:

First, build an application that uses the APIs provided in `libdash/dash.h`.
Then, if wanting to just test that your application works as expected, begin by building the `CMake` project in `libdash` to produce a static library archive of CPU-only function implementations.

```bash
cd libdash
mkdir build
cd build
cmake ../
make
```

After this, compile your application and link against `libdash.a` produced in this build directory. One open issue is that you also need to link against `libmath` (i.e. `-lm`) as it has issues being statically compiled into `libdash.a`.
With this, you should have a standalone binary that can be used to at least test that your application is functional.

Next, you can move on to testing your application's behavior in the provided `mock_runtime`

To do this, build the `mock_runtime` from the repository root

```bash
mkdir build
cd build
cmake ../
make
```

And then, rather than compile your application as a standalone binary, compile it as a shared object, and do not link against any libdash implementation -- there is a different version that is compiled and linked by the runtime itself.
After this, you can test your application with
```bash
./mock_runtime ./my_app.so
```
And the behavior should match what was observed with the standalone execution

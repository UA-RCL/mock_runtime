This test application provides a basis for building applications that can be executed in this mock runtime.

If building for the runtime, the resulting binary must be a shared object, and as such, it's fine to leave functions like DASH\_FFT unlinked.
However, you can optionally also build your binary to 

This is a template for a simple project for learning some of the LLVM APIs
relevant for dynamic analysis. It involves instrumenting an LLVM module in
order to produce a new program that collects frequency information for edges
in the call graph.


Building with CMake
==============================================

1. Create a new directory for building.

        mkdir cgbuild

2. Change into the new directory.

        cd cgbuild

3. Run CMake with the path to the LLVM source.

        cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=True \
            -DLLVM_DIR=</path/to/LLVM/build>/lib/cmake/llvm/ ../callgraph-profiler

4. Run make inside the build directory:

        make

When you have successfully completed the project, this will produce a tool
for profiling the callgraph called `bin/callgraph-profiler` along with
supporting libraries in `lib/`.

Note, building with a tool like ninja can be done by adding `-G Ninja` to
the cmake invocation and running ninja instead of make.

Running
==============================================

First suppose that you have a program compiled to bitcode:

    clang -g -c -emit-llvm ../callgraph-profiler/test/test.c -o test.bc

Running the call graph profiler:

    bin/callgraph-profiler test.bc -o test
    ./test

When you have successfully completed the exercise, running an instrumented
program like `./test` in the above example should produce a file called
`trace.txt` in the test directory of grammer-infer.





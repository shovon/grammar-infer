# Syntactic grammar inference

[Read the paper](https://github.com/shovon/grammar-infer/blob/master/paper.pdf).

## Usage

**Note**: only tested (and most likely only works on) LLVM 3.9.x.

```
# Profiling
clang -g -c -emit-llvm $SOURCE_FILE.c -o $OUTPUT.bc
$ABSOLUTE_PATH_TO_GRAMMER_INFER_BUILD $OUTPUT.bc -o $BINARY

# Execute the profiled binary
`realpath $BINARY` # plus some commands for your program to analyze

# The trace of the profiled binary should now be in `trace.txt`

# Analyzing trace
$ABSOLUTE_PATH_TO_TRACE_ANALYZER -t $PATH_TO/trace.txt -c $CSV.csv \
  -g $GRAMMAR.txt
```

## Building

```shell
# Profiler
mkdir grammar_infer_build
cd grammar_infer_build
cmake -DLLVM_DIR=<absolute/to/LLVM>/lib/cmake/llvm \
    <path/to>/grammar-infer/grammar-infer
make

# Trace analyzer
mkdir trace_build
cd trace_build
cmake <path/to>/trace-analyzer
make
```
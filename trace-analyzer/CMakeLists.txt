cmake_minimum_required(VERSION 3.0)
add_compile_options(-std=c++11)
project(analyzer)
add_executable(analyzer TraceAnalyzer.cpp Tree.cpp Tree.h)

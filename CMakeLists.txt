cmake_minimum_required(VERSION 3.22)
project(Okapy LANGUAGES CXX)

# The native core is kept independent from Android/JNI so it can be
# developed and tested separately before game integration is added.
add_subdirectory(core)

# bodge to make libraries using FIND_PACKAGE(Threads) work
add_library(Threads::Threads INTERFACE IMPORTED)

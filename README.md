## [c_vector.h](https://github.com/Flix01/c_vector_and_hashtable/blob/master/c_vector.h)
A type-safe implementation of std::vector in plain C. 
It supports custom constructors/destructors/copy operators to perform complex operations (e.g. vectors of vectors).
Its test program is here [c_vector_main.c](https://github.com/Flix01/c_vector_and_hashtable/blob/master/deprecated/c_vector_main.c).

## [c_hashtable.h](https://github.com/Flix01/c_vector_and_hashtable/blob/master/c_hashtable.h)
A type-safe implementation of std::hashtable (or std::unordered_map) in plain C. 
It supports custom constructors/destructors/copy operators to perform complex operations.
Its test program is here [c_hashtable_main.c](https://github.com/Flix01/c_vector_and_hashtable/blob/master/c_hashtable_main.c).


## [c_vector_and_hashtable.h](https://github.com/Flix01/c_vector_and_hashtable/blob/master/deprecated/c_vector_and_hashtable.h)
A simpler, type-unsafe implementation of vector and hashtable in plain C. 
Its test program is here [main.c](https://github.com/Flix01/c_vector_and_hashtable/blob/master/deprecated/main.c).

### How to compile
Compilation instructions can be found at the top of each .c file, but generally they are as simple as something like:
gcc -O2 -no-pie -fno-pie main.c -o main


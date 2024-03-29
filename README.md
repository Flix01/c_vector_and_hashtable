## [c_vector.h](https://github.com/Flix01/c_vector_and_hashtable/blob/master/include/c_vector.h)
A type-safe implementation of std::vector in plain C. 
It supports custom constructors/destructors/copy operators to perform complex operations (e.g. vectors of vectors), and serialization/deserialization as well.
Its test program is here [c_vector_main.c](https://github.com/Flix01/c_vector_and_hashtable/blob/master/examples/c_vector_main.c).

## [c_hashtable.h](https://github.com/Flix01/c_vector_and_hashtable/blob/master/include/c_hashtable.h)
A type-safe implementation of std::hashtable (or std::unordered_map) in plain C. 
It supports custom constructors/destructors/copy operators to perform complex operations.
Its test program is here [c_hashtable_main.c](https://github.com/Flix01/c_vector_and_hashtable/blob/master/examples/c_hashtable_main.c).

## [c_vector_type_unsafe.h](https://github.com/Flix01/c_vector_and_hashtable/blob/master/include/c_vector_type_unsafe.h)
A type-unsafe version of c_vector.h. 
Its test program is here [c_vector_type_unsafe_main.c](https://github.com/Flix01/c_vector_and_hashtable/blob/master/examples/c_vector_type_unsafe_main.c).

## [c_hashtable_type_unsafe.h](https://github.com/Flix01/c_vector_and_hashtable/blob/master/include/c_hashtable_type_unsafe.h)
A type-unsafe version of c_hashtable.h. 
Its test program is here [c_hashtable_type_unsafe_main.c](https://github.com/Flix01/c_vector_and_hashtable/blob/master/examples/c_hashtable_type_unsafe_main.c).

## [c_vector_and_hashtable.h](https://github.com/Flix01/c_vector_and_hashtable/blob/master/include/deprecated/c_vector_and_hashtable.h)
A **deprecated**, simpler, type-unsafe implementation of vector and hashtable in plain C. 
Its test program is here [main.c](https://github.com/Flix01/c_vector_and_hashtable/blob/master/examples/deprecated/main.c).


### How to compile
Compilation instructions can be found at the top of each .c file, but generally they are as simple as something like:

gcc -O2 -no-pie -fno-pie main.c -o main


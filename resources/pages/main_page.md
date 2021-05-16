@mainpage

# What is libcaramel?

**libcaramel** is a library of data structures that I am developing for fun and for learning purposes. All data
structures implemented in the library are compatible with the STL.

# Libcaramel Libraries

## Containers

The **containers** folder holds all data structures that are used as building block for other data structures. The
containers may belong to categories as follows:

### Sequence Containers

**Sequence** containers are data structures that may be accessed sequentially.

- caramel::dynamic_array - See @ref Dynamic-Array for more info
- caramel::deque

## Adaptors

**Adaptors** are data structure that provide a new API over data structures defined in the [Containers](#Containers)
section.

- caramel::queue - Work in progress
- caramel::stack - Work in progress

## Iterators

The **iterators** folder implements a set of iterators using the iterator_facade class template.

- caramel::random_access_iterator
- caramel::iterator_facade - See @ref Iterator-Facade for more info

## Memory

The **memory** folder implement allocators and memory resources used to specify allocation strategies in
[containers](#Containers). The memory_resource base class can be inherited from to allow for third party resources to be
used in all containers.

- caramel::global_resource
- caramel::memory_resource
- caramel::memory_allocator

See @ref Memory-Resources for more info

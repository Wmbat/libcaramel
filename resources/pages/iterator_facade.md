@page Iterator-Facade

# Iterator Facade

## What is it?

caramel::iterator_facade is a class that provides an interface for iterators based an a set of functions defined in the child
class through CRTP

## How to use it

For the interface to be used, the following member functions **must** be provided:

* `my_iterator::dereference()` - Used to implement `operator*()`.
* `my_iterator::increment()` or `my_iterator::advance(std::ptrdiff_t)` or both - Used to implement `operator++()` and
  `operator++(int)`. If both functions are provided, `my_iterator::increment` will be prefered.

With these two functions defined, the iterator is a **forward iterator**. If `advance(std::ptrdiff_t)` is provided, the
iterator is also **bidirectional**.

### Iterator Equality

The generated iterator type is equality-comparible with any object of type S if the derived class implements 
`distance_to(S)`. This includes sentinel types and other instances of the iterator.

### Single-pass Input

If the class provides a static member `single_pass_iterator` that is `true`, then the iterator will be an 
**input_iterator**, and the `operator++(int)` (postfix decrement) will return `void`.

### Bidirectional

If the following are provided, the iterator is **bidirectional**:

- `my_iterator::decrement()` or `my_iterator::advance(std::ptrdiff_t)` - Used to implement `operator--()` and
  `operator--(int)`.


Note that unless the requirements of [Random Access](#Random-Access) are met, `advance(std::ptrdiff_t)` will only be called with 1 or -1.
  
### Random Access

If the following are provided, the iterator is a **random access**:

- `my_iterator::advance(std::ptrdiff_t)` - Move the iterator by a positive or negative value.
- `my_iterator::distance_to(my_iterator other)` - Return the "distance" to `other`, that is: the number of times
  `*this` must be incremented or decremented to be equal to `other`.
  
These two functions are used to implement the remainder of the iterator functionality.

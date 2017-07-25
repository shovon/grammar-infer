#include <tuple>

// Taken from https://stackoverflow.com/a/24881903/538570

template <typename T>
class enumerate_impl {
public:
  // The return value of the operator* of the iterator, this
  // is what you will get inside of the for loop
  struct item {
    size_t index;
    typename T::value_type & item;
  };
  typedef item value_type;

  // Custom iterator with minimal interface
  struct iterator {
    iterator(typename T::iterator _it, size_t counter=0) :
      it(_it), counter(counter)
    {}

    iterator operator++() {
      return iterator(++it, ++counter);
    }

    bool operator!=(iterator other) {
      return it != other.it;
    }

    typename T::iterator::value_type item() {
      return *it;
    }

    value_type operator*() {
      return value_type{counter, *it};
    }

    size_t index() {
      return counter;
    }

  private:
    typename T::iterator it;
    size_t counter;
  };

  enumerate_impl(T & t) : container(t) {}

  iterator begin() {
    return iterator(container.begin());
  }

  iterator end() {
    return iterator(container.end());
  }

private:
  T & container;
};

// A templated free function allows you to create the wrapper class
// conveniently 
template <typename T>
enumerate_impl<T> enumerate(T & t) {
  return enumerate_impl<T>(t);
}
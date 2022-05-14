#ifndef MPC_SMALLVECTOR
#define MPC_SMALLVECTOR

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>

namespace mpc {
template <typename T, size_t N = 8>
class smallVector {
  // Member variables
  T *m_data;
  T *m_buffptr;
  alignas(alignof(T)) char m_buff[sizeof(T) * N];
  size_t m_alloc;
  size_t m_size;

 public:
  // Public member types
  typedef T value_type;
  typedef T &reference;
  typedef const T &const_reference;
  typedef T *pointer;
  typedef const T *const_pointer;
  typedef T *iterator;
  typedef const T *const_iterator;

  //====================Ctors and Dtors====================

  // Default constructor
  smallVector()
      : m_data(nullptr),
        m_buffptr(reinterpret_cast<T *>(m_buff)),
        m_alloc(0),
        m_size(0) {}

  // Constructor with given size
  smallVector(const size_t sz) : smallVector() { resize(sz); }

  // Copy constructor
  smallVector(const smallVector &other) : smallVector() {
    // std::cout<<"Copy constr called"<<std::endl;
    try {
      reserve(other.m_size);
    } catch (std::exception &e) {
      this->nearlyDestroy();
      std::cout << e.what() << std::endl;
      throw;
    }
    std::uninitialized_copy(other.begin(), other.end(), begin());
    m_size = other.m_size;
    m_alloc = other.m_alloc;
  }

  // Move constructor
  smallVector(smallVector &&other) noexcept {
    // std::cout<<"Move constr called"<<std::endl;
    if (other.begin() == other.m_buffptr) {
      m_buffptr = other.m_buffptr;
      other.m_buffptr = nullptr;
    } else {
      m_data = other.m_data;
      other.m_data = nullptr;
    }
    m_alloc = other.m_alloc;
    m_size = other.m_size;
    other.m_size = 0;
    other.m_alloc = 0;
  }

  // Conversion constructor
  smallVector(std::initializer_list<T> init) : smallVector() {
    try {
      reserve(init.size());
    } catch (std::exception &e) {
      this->nearlyDestroy();
      std::cout << e.what() << std::endl;
      throw;
    }
    for (auto it = init.begin(); it != init.end(); it++) {
      new (end()) T(*it);
      m_size++;
    }
  }

  // Destructor
  ~smallVector() {
    clear();
    if (m_alloc) ::operator delete(m_data);
  }

  //___________________________Operators_______________________________

  // Copy op =
  smallVector &operator=(const smallVector &other) {
    // std::cout<<"Copy op= called"<<std::endl;
    if (this == &other) return *this;
    this->nearlyDestroy();
    try {
      reserve(other.m_size);
    } catch (std::exception &e) {
      this->nearlyDestroy();
      std::cout << e.what() << std::endl;
      throw;
    }
    std::uninitialized_copy(other.begin(), other.end(), begin());
    m_size = other.m_size;
    m_alloc = other.m_alloc;
    return *this;
  }

  // Move op =
  smallVector &operator=(smallVector &&other) noexcept {
    // std::cout<<"Move op= called"<<std::endl;
    this->nearlyDestroy();
    if (other.begin() == other.m_buffptr) {
      m_buffptr = other.m_buffptr;
      other.m_buffptr = nullptr;
    } else {
      m_data = other.m_data;
      other.m_data = nullptr;
    }
    m_alloc = other.m_alloc;
    m_size = other.m_size;
    other.m_size = 0;
    other.m_alloc = 0;
    return *this;
  }

  // [] op
  reference operator[](size_t ind) { return *(begin() + ind); }

  // const [] op
  const_reference operator[](size_t ind) const { return *(begin() + ind); }

  //___________________________Element
  // manipulation_______________________________

  // Copies inp at the end of vec
  void push_back(const T &inp) {
    // std::cout<<"Copy PB called"<<std::endl;
    try {
      PbEbCheck(m_size + 1);
    } catch (std::exception &e) {
      std::cout << e.what() << std::endl;
      throw;
    }
    new (end()) T(inp);
    m_size++;
  }

  // Moves inp at the end of vec
  void push_back(T &&inp) {
    // std::cout<<"Move PB called"<<std::endl;
    try {
      PbEbCheck(m_size + 1);
    } catch (std::exception &e) {
      std::cout << e.what() << std::endl;
      throw;
    }
    new (end()) T(std::move(inp));
    m_size++;
  }

  // Emplace back
  template <typename... Ts>
  void emplace_back(Ts &&...params) {
    // std::cout<<"Emplace Back called"<<std::endl;
    try {
      PbEbCheck(m_size + 1);
    } catch (std::exception &e) {
      std::cout << e.what() << std::endl;
      throw;
    }
    new (end()) T(std::forward<Ts>(params)...);
    m_size++;
  }

  // Reserves at least inp in vec
  // Strong exc. guar.
  void reserve(size_t inp) {
    // std::cout<<"Reserve called"<<std::endl;
    if (m_alloc > inp || N >= inp) return;
    T *temp = (T *)::operator new(inp * sizeof(T));
    size_t origSize = m_size;
    size_t i;
    try {
      for (i = 0; i < m_size; i++)
        new (temp + i) T(std::move_if_noexcept(*(begin() + i)));
    } catch (std::exception &e) {
      while (i) {
        (temp + i - 1)->~T();
        i--;
      }
      ::operator delete(temp);
      std::cout << e.what() << std::endl;
      throw;
    }
    this->nearlyDestroy();
    m_data = temp;
    m_size = origSize;
    m_alloc = inp;
  }

  // strong exc. guarantee
  void resize(size_t size, const T &val = T()) {
    if (size == m_size) return;
    if (size < m_size) {
      while (size < m_size) {
        (end() - 1)->~T();
        m_size--;
      }
    } else {
      try {
        reserve(size);
      } catch (std::exception &e) {
        std::cout << e.what() << std::endl;
        throw;
      }
      while (m_size < size) {
        new (end()) T(val);
        m_size++;
      }
    }
  }

  // Destructs objs in vec, aloc is the same
  void clear() noexcept {
    while (m_size) {
      (end() - 1)->~T();
      m_size--;
    }
  }

  //___________________________Iterator_______________________________

  iterator begin() {
    if (m_alloc)
      return m_data;
    else
      return m_buffptr;
  }

  const_iterator begin() const {
    if (m_alloc)
      return m_data;
    else
      return m_buffptr;
  }

  iterator end() { return begin() + m_size; }

  const_iterator end() const { return begin() + m_size; }

  //___________________________Getters_______________________________

  size_t size() const noexcept { return m_size; }

  size_t capacity() const noexcept {
    if (m_alloc)
      return m_alloc;
    else
      return N;
  }

  pointer data() { return begin(); }

  const_pointer data() const { return begin(); }

  //___________________________Misc_______________________________

  void swap(smallVector &other) noexcept {
    // std::cout<<"swap called"<<std::endl;
    if (begin() == m_data && other.begin() == other.m_data)
      std::swap(m_data, other.m_data);
    else if (begin() == m_buffptr && other.begin() == other.m_buffptr)
      std::swap(m_buffptr, other.m_buffptr);
    else {
      std::swap(m_data, other.m_data);
      std::swap(m_buffptr, other.m_buffptr);
    }
    std::swap(m_size, other.m_size);
    std::swap(m_alloc, other.m_alloc);
  }

  //___________________________Debug_______________________________

  size_t getAlloc() { return m_alloc; }

  //___________________________Private func_______________________________

 private:
  void PbEbCheck(size_t chckSize) {
    if (chckSize > N) {
      try {
        reserve(N * 2);
      } catch (std::exception &e) {
        throw;
      }
    }
    if (chckSize > m_alloc) {
      try {
        reserve(m_alloc * 2);
      } catch (std::exception &e) {
        throw;
      }
    }
  }

  // Near Destructor
  void nearlyDestroy() noexcept {
    clear();
    if (m_alloc) ::operator delete(m_data);
    m_alloc = 0;
    m_size = 0;
  }

};  // class small vector

// outside swap function
template <typename T, size_t N>
void swap(smallVector<T, N> &avec, smallVector<T, N> &bvec) noexcept {
  avec.swap(bvec);
}

}  // namespace mpc

#endif  // MPC_SMALLVECTOR

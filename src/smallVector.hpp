#ifndef MPC_SMALLVECTOR
#define MPC_SMALLVECTOR

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace mpc {
template <typename T, size_t N = 8>
class smallVector {
  // Constants
  static constexpr size_t LARGE_SIZE_THRESHOLD = 1024;

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
    try {
      reserve(other.m_size);
    } catch (std::exception &e) {
      this->nearlyDestroy();
      handleException(e);
    }
    std::uninitialized_copy(other.begin(), other.end(), begin());
    m_size = other.m_size;
    m_alloc = other.m_alloc;
  }

  // Move constructor
  smallVector(smallVector &&other) noexcept : smallVector() {
    if (other.begin() == other.m_buffptr) {
      // Other uses stack storage - we need to move construct elements
      std::uninitialized_move(other.begin(), other.end(), begin());
      m_size = other.m_size;
      other.clear();
      return;
    }
    // Other uses heap storage - we can steal the pointer
    m_data = other.m_data;
    m_alloc = other.m_alloc;
    m_size = other.m_size;
    other.m_data = nullptr;
    other.m_alloc = 0;
    other.m_size = 0;
  }

  // Conversion constructor
  smallVector(std::initializer_list<T> init) : smallVector() {
    try {
      reserve(init.size());
    } catch (std::exception &e) {
      this->nearlyDestroy();
      handleException(e);
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
    if (this == &other) return *this;
    this->nearlyDestroy();
    try {
      reserve(other.m_size);
    } catch (std::exception &e) {
      this->nearlyDestroy();
      handleException(e);
    }
    std::uninitialized_copy(other.begin(), other.end(), begin());
    m_size = other.m_size;
    m_alloc = other.m_alloc;
    return *this;
  }

  // Move op =
  smallVector &operator=(smallVector &&other) noexcept {
    if (this == &other) return *this;

    this->nearlyDestroy();
    if (other.begin() == other.m_buffptr) {
      // Other uses stack storage - we need to move construct elements
      std::uninitialized_move(other.begin(), other.end(), begin());
      m_size = other.m_size;
      other.clear();
    } else {
      // Other uses heap storage - we can steal the pointer
      m_data = other.m_data;
      m_alloc = other.m_alloc;
      m_size = other.m_size;
      other.m_data = nullptr;
      other.m_alloc = 0;
      other.m_size = 0;
    }
    return *this;
  }

  // [] op
  reference operator[](size_t ind) { return *(begin() + ind); }

  // const [] op
  const_reference operator[](size_t ind) const { return *(begin() + ind); }

  // Bounds-checked access
  reference at(size_t ind) {
    if (ind >= m_size) {
      throw std::out_of_range("smallVector::at: index out of range");
    }
    return *(begin() + ind);
  }

  // Const bounds-checked access
  const_reference at(size_t ind) const {
    if (ind >= m_size) {
      throw std::out_of_range("smallVector::at: index out of range");
    }
    return *(begin() + ind);
  }

  //_________________Element manipulation_______________

  // Copies inp at the end of vec
  void push_back(const T &inp) {
    try {
      ensureCapacity(m_size + 1);
    } catch (std::exception &e) {
      handleException(e);
    }
    new (end()) T(inp);
    m_size++;
  }

  // Moves inp at the end of vec
  void push_back(T &&inp) {
    try {
      ensureCapacity(m_size + 1);
    } catch (std::exception &e) {
      handleException(e);
    }
    new (end()) T(std::move(inp));
    m_size++;
  }

  // Emplace back
  template <typename... Ts>
  void emplace_back(Ts &&...params) {
    try {
      ensureCapacity(m_size + 1);
    } catch (std::exception &e) {
      handleException(e);
    }
    new (end()) T(std::forward<Ts>(params)...);
    m_size++;
  }

  // Reserves at least inp in vec
  // Strong exc. guar.
  void reserve(size_t inp) {
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
      handleException(e);
    }
    this->nearlyDestroy();
    m_data = temp;
    m_size = origSize;
    m_alloc = inp;
  }

  // Strong exc. guarantee
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
        handleException(e);
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
    // Handle the simple case: both use heap or both use stack
    const auto this_use_stack = (begin() == m_buffptr);
    const auto othe_use_stack = (other.begin() == other.m_buffptr);

    if (!this_use_stack && !othe_use_stack) {
      // Both use heap - just swap pointers and sizes
      std::swap(m_data, other.m_data);
      std::swap(m_size, other.m_size);
      std::swap(m_alloc, other.m_alloc);
    } else if (this_use_stack && othe_use_stack) {
      // Both use stack - swap element by element
      const auto min_sz = std::min(m_size, other.m_size);
      const auto max_sz = std::max(m_size, other.m_size);
      // Swap the common elements
      for (size_t i = 0; i < min_sz; ++i) {
        std::swap(*(begin() + i), *(other.begin() + i));
      }
      // Move remaining elements from larger to smaller
      if (m_size > other.m_size) {
        std::uninitialized_move(begin() + min_sz, end(), other.end());
        for (size_t i = min_sz; i < max_sz; ++i) {
          (begin() + i)->~T();
        }
      } else if (other.m_size > m_size) {
        std::uninitialized_move(other.begin() + min_sz, other.end(), end());
        for (size_t i = min_sz; i < max_sz; ++i) {
          (other.begin() + i)->~T();
        }
      }

      std::swap(m_size, other.m_size);
    } else {
      // Mixed case - use temporary (less efficient but correct)
      smallVector temp(std::move(*this));
      *this = std::move(other);
      other = std::move(temp);
    }
  }

  //___________________________Debug_______________________________

  size_t getAlloc() const { return m_alloc; }

  //___________________________Private func_______________________________

 private:
  void ensureCapacity(size_t req_sz) {
    if (req_sz <= capacity()) return;  // cap ensured

    // For very large sizes, use a smaller growth factor to save memory
    const auto new_cap_suggestion = capacity() > LARGE_SIZE_THRESHOLD
                                        ? (capacity() + capacity() / 2)
                                        : (capacity() * 2);
    const auto new_cap = std::max(new_cap_suggestion, req_sz);

    reserve(new_cap);
  }

  void handleException(const std::exception &e) {
    std::cout << e.what() << std::endl;
    throw;
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

#ifndef MPC_SMALLVECTOR
#define MPC_SMALLVECTOR

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <memory>
#include <stdexcept>

namespace mpc {

// Compatibility wrapper for uninitialized_move
#if __cplusplus >= 201703L
using std::uninitialized_move;  // C++17 has this in STL
#else
template <typename InputIt, typename ForwardIt>
ForwardIt uninitialized_move(InputIt first, InputIt last, ForwardIt d_first) {
  typedef typename std::iterator_traits<ForwardIt>::value_type Value;
  ForwardIt current = d_first;
  try {
    for (; first != last; ++first, (void)++current) {
      ::new (static_cast<void *>(std::addressof(*current)))
          Value(std::move(*first));
    }
    return current;
  } catch (...) {
    for (; d_first != current; ++d_first) {
      d_first->~Value();
    }
    throw;
  }
}
#endif

// Main Small Vector class
template <typename T, size_t N = sizeof(void *)>
class small_vector {
  // Constants
  static constexpr size_t LARGE_SIZE_THRESHOLD = 1024;

  // Member variables
  union {
    T *m_data;
    alignas(alignof(T)) char m_buff[sizeof(T) * N];
  };
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
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  //====================Ctors and Dtors====================

  // Default constructor
  small_vector() : m_data(nullptr), m_alloc(0), m_size(0) {}

  // Constructor with given size
  small_vector(const size_t sz) : small_vector() { resize(sz); }

  // Copy constructor
  small_vector(const small_vector &other) : small_vector() {
    try {
      reserve(other.m_size);
    } catch (std::exception &e) {
      this->nearly_destroy();
      handle_exception(e);
    }
    std::uninitialized_copy(other.begin(), other.end(), begin());
    m_size = other.m_size;
    m_alloc = other.m_alloc;
  }

  // Move constructor
  small_vector(small_vector &&other) noexcept : small_vector() {
    if (!other.m_alloc) {
      // Other uses stack storage - we need to move construct elements
      mpc::uninitialized_move(other.begin(), other.end(), begin());
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
  small_vector(std::initializer_list<T> init) : small_vector() {
    try {
      reserve(init.size());
    } catch (std::exception &e) {
      this->nearly_destroy();
      handle_exception(e);
    }
    for (auto it = init.begin(); it != init.end(); it++) {
      new (end()) T(*it);
      m_size++;
    }
  }

  // Destructor
  ~small_vector() {
    clear();
    if (m_alloc) ::operator delete(m_data);
  }

  //___________________________Operators_______________________________

  // Copy op =
  small_vector &operator=(const small_vector &other) {
    if (this == &other) return *this;
    this->nearly_destroy();
    try {
      reserve(other.m_size);
    } catch (std::exception &e) {
      this->nearly_destroy();
      handle_exception(e);
    }
    std::uninitialized_copy(other.begin(), other.end(), begin());
    m_size = other.m_size;
    m_alloc = other.m_alloc;
    return *this;
  }

  // Move op =
  small_vector &operator=(small_vector &&other) noexcept {
    if (this == &other) return *this;

    this->nearly_destroy();
    if (!other.m_alloc) {
      // Other uses stack storage - we need to move construct elements
      mpc::uninitialized_move(other.begin(), other.end(), begin());
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
  reference operator[](const size_t ind) { return *(begin() + ind); }

  // const [] op
  const_reference operator[](const size_t ind) const {
    return *(begin() + ind);
  }

  // Bounds-checked access
  reference at(const size_t ind) {
    if (ind >= m_size) {
      throw std::out_of_range("small_vector::at: index out of range");
    }
    return *(begin() + ind);
  }

  // Const bounds-checked access
  const_reference at(const size_t ind) const {
    if (ind >= m_size) {
      throw std::out_of_range("small_vector::at: index out of range");
    }
    return *(begin() + ind);
  }

  //_________________Element manipulation_______________

  // Copies inp at the end of vec
  void push_back(const T &inp) {
    try {
      ensure_capacity(m_size + 1);
    } catch (std::exception &e) {
      handle_exception(e);
    }
    new (end()) T(inp);
    m_size++;
  }

  // Moves inp at the end of vec
  void push_back(T &&inp) {
    try {
      ensure_capacity(m_size + 1);
    } catch (std::exception &e) {
      handle_exception(e);
    }
    new (end()) T(std::move(inp));
    m_size++;
  }

  // Emplace back
  template <typename... Ts>
  void emplace_back(Ts &&...params) {
    try {
      ensure_capacity(m_size + 1);
    } catch (std::exception &e) {
      handle_exception(e);
    }
    new (end()) T(std::forward<Ts>(params)...);
    m_size++;
  }

  void pop_back() {
    if (m_size > 0) {
      (end() - 1)->~T();
      m_size--;
    }
  }

  // Insert element at position
  iterator insert(const_iterator pos, const T &value) {
    return insert_impl(pos, value);
  }

  iterator insert(const_iterator pos, T &&value) {
    return insert_impl(pos, std::move(value));
  }

  // Erase element at position
  iterator erase(const_iterator pos) { return erase(pos, pos + 1); }

  // Erase range of elements
  iterator erase(const_iterator first, const_iterator last) {
    const auto first_offset = first - cbegin();
    const auto last_offset = last - cbegin();

    if (first_offset < 0 || last_offset > static_cast<ptrdiff_t>(m_size) ||
        first_offset > last_offset) {
      throw std::out_of_range("small_vector::erase: iterator range invalid");
    }

    if (first == last) {
      return begin() + first_offset;
    }

    auto erase_first = begin() + first_offset;
    auto erase_last = begin() + last_offset;

    // Move remaining elements to fill the gap and destroy trailing
    std::move(erase_last, end(), erase_first);
    auto new_end = end() - (last_offset - first_offset);
    for (auto it = new_end; it != end(); ++it) {
      it->~T();
    }

    m_size -= (last_offset - first_offset);

    return erase_first;
  }

  // Reserves at least inp in vec
  // Strong exc. guar.
  void reserve(const size_t inp) {
    if (m_alloc > inp || N >= inp) return;
    T *temp = (T *)::operator new(inp * sizeof(T));
    const auto orig_sz = m_size;
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
      handle_exception(e);
    }
    this->nearly_destroy();
    m_data = temp;
    m_size = orig_sz;
    m_alloc = inp;
  }

  // Strong exc. guarantee
  void resize(const size_t size, const T &val = T()) {
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
        handle_exception(e);
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

  iterator begin() { return m_alloc ? m_data : reinterpret_cast<T *>(m_buff); }
  const_iterator begin() const { return cbegin(); }
  const_iterator cbegin() const noexcept {
    return m_alloc ? m_data : reinterpret_cast<T *>(const_cast<char *>(m_buff));
  }
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const { return crbegin(); }
  const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  iterator end() { return begin() + m_size; }
  const_iterator end() const { return cend(); }
  const_iterator cend() const noexcept { return begin() + m_size; }
  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const { return crend(); }
  const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator(begin());
  }

  //___________________________Getters_______________________________

  size_t size() const noexcept { return m_size; }

  size_t capacity() const noexcept { return m_alloc ? m_alloc : N; }

  pointer data() { return begin(); }
  const_pointer data() const { return begin(); }

  //___________________________Element Access_______________________________

  reference front() { return *begin(); }
  const_reference front() const { return *begin(); }

  reference back() { return *(end() - 1); }
  const_reference back() const { return *(end() - 1); }

  //___________________________Misc_______________________________

  void swap(small_vector &other) noexcept {
    if (m_alloc && other.m_alloc) {
      // Both use heap - just swap pointers and sizes
      std::swap(m_data, other.m_data);
      std::swap(m_size, other.m_size);
      std::swap(m_alloc, other.m_alloc);
    } else if (!m_alloc && !other.m_alloc) {
      // Both use stack - swap element by element
      const auto min_sz = std::min(m_size, other.m_size);
      const auto max_sz = std::max(m_size, other.m_size);
      // Swap the common elements
      for (size_t i = 0; i < min_sz; ++i) {
        std::swap(*(begin() + i), *(other.begin() + i));
      }
      // Move remaining elements from larger to smaller
      if (m_size > other.m_size) {
        mpc::uninitialized_move(begin() + min_sz, end(), other.end());
        for (auto i = min_sz; i < max_sz; ++i) {
          (begin() + i)->~T();
        }
      } else if (other.m_size > m_size) {
        mpc::uninitialized_move(other.begin() + min_sz, other.end(), end());
        for (auto i = min_sz; i < max_sz; ++i) {
          (other.begin() + i)->~T();
        }
      }

      std::swap(m_size, other.m_size);
    } else {
      // Mixed case - use temporary (less efficient but correct)
      small_vector temp(std::move(*this));
      *this = std::move(other);
      other = std::move(temp);
    }
  }

  //___________________________Debug_______________________________

  size_t get_alloc() const { return m_alloc; }

  //___________________________Private func_______________________________

 private:
  iterator insert_impl(const_iterator pos, T &&value) {
    const auto offset = pos - cbegin();
    if (offset < 0 || offset > static_cast<ptrdiff_t>(m_size)) {
      throw std::out_of_range("small_vector::insert: iterator out of range");
    }

    try {
      ensure_capacity(m_size + 1);
    } catch (std::exception &e) {
      handle_exception(e);
    }

    auto insert_pos = begin() + offset;

    if (m_size > 0) {
      // Move-construct last element and move existing elements backward
      new (end()) T(std::move(*(end() - 1)));
      std::move_backward(insert_pos, end() - 1, end());
    }

    *insert_pos = std::forward<T>(value);
    ++m_size;

    return insert_pos;
  }

  void ensure_capacity(const size_t req_sz) {
    if (req_sz <= capacity()) return;  // cap ensured

    // For very large sizes, use a smaller growth factor to save memory
    const auto new_cap_suggestion = capacity() > LARGE_SIZE_THRESHOLD
                                        ? (capacity() + capacity() / 2)
                                        : (capacity() * 2);
    const auto new_cap = std::max(new_cap_suggestion, req_sz);

    reserve(new_cap);
  }

  void handle_exception(const std::exception &e) {
    printf("%s\n", e.what());
    throw;
  }

  // Near Destructor
  void nearly_destroy() noexcept {
    clear();
    if (m_alloc) {
      ::operator delete(m_data);
    }
    m_alloc = 0;
    m_size = 0;
  }

};  // class small vector

// outside swap function
template <typename T, size_t N>
void swap(small_vector<T, N> &avec, small_vector<T, N> &bvec) noexcept {
  avec.swap(bvec);
}

}  // namespace mpc

#endif  // MPC_SMALLVECTOR

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "small_vector.hpp"

using namespace mpc;

///==================== Constructor ====================
TEST(SmallVectorTest, DefaultConstructor) {
  small_vector<int> vec;
  EXPECT_EQ(vec.size(), 0);
  EXPECT_GE(vec.capacity(), 8);  // default N = 8
}

TEST(SmallVectorTest, SizeConstructor) {
  small_vector<int> vec(5);
  EXPECT_EQ(vec.size(), 5);
  EXPECT_GE(vec.capacity(), 5);
}

TEST(SmallVectorTest, InitializerListConstructor) {
  small_vector<int> vec{1, 2, 3, 4, 5};
  EXPECT_EQ(vec.size(), 5);
  EXPECT_EQ(vec[0], 1);
  EXPECT_EQ(vec[1], 2);
  EXPECT_EQ(vec[2], 3);
  EXPECT_EQ(vec[3], 4);
  EXPECT_EQ(vec[4], 5);
}

TEST(SmallVectorTest, CopyConstructor) {
  small_vector<int> vec1{1, 2, 3};
  small_vector<int> vec2(vec1);

  vec1[0] = 5;
  EXPECT_EQ(vec2.size(), 3);
  EXPECT_EQ(vec2[0], 1);
  EXPECT_EQ(vec2[1], 2);
  EXPECT_EQ(vec2[2], 3);
}

TEST(SmallVectorTest, MoveConstructor) {
  small_vector<int> vec1{1, 2, 3};
  small_vector<int> vec2(std::move(vec1));

  EXPECT_EQ(vec2.size(), 3);
  EXPECT_EQ(vec2[0], 1);
  EXPECT_EQ(vec2[1], 2);
  EXPECT_EQ(vec2[2], 3);
  EXPECT_EQ(vec1.size(), 0);  // moved from
}

TEST(SmallVectorTest, MoveConstructorStackStorage) {
  small_vector<std::string, 4> vec1;
  vec1.push_back("hello");
  vec1.push_back("world");

  // ensure vec1 is using stack storage
  EXPECT_EQ(vec1.get_alloc(), 0);

  small_vector<std::string, 4> vec2(std::move(vec1));
  EXPECT_EQ(vec2.size(), 2);
  EXPECT_EQ(vec2[0], "hello");
  EXPECT_EQ(vec2[1], "world");

  EXPECT_EQ(vec1.size(), 0);
}

TEST(SmallVectorTest, MoveConstructorHeapStorage) {
  small_vector<int, 2> vec1;
  for (int i = 0; i < 10; ++i) {
    vec1.push_back(i);
  }

  // ensure vec1 is using heap storage
  EXPECT_GT(vec1.get_alloc(), 0);

  small_vector<int, 2> vec2(std::move(vec1));
  EXPECT_EQ(vec2.size(), 10);
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(vec2[i], i);
  }

  EXPECT_EQ(vec1.size(), 0);
}

// ==================== Assignment ====================
TEST(SmallVectorTest, CopyAssignment) {
  small_vector<int> vec1{1, 2, 3};
  small_vector<int> vec2;
  vec2 = vec1;
  vec1[0] = 5;

  EXPECT_EQ(vec2.size(), 3);
  EXPECT_EQ(vec2[0], 1);
  EXPECT_EQ(vec2[1], 2);
  EXPECT_EQ(vec2[2], 3);
}

TEST(SmallVectorTest, MoveAssignment) {
  small_vector<int> vec1{1, 2, 3};
  small_vector<int> vec2;
  vec2 = std::move(vec1);

  EXPECT_EQ(vec2.size(), 3);
  EXPECT_EQ(vec2[0], 1);
  EXPECT_EQ(vec2[1], 2);
  EXPECT_EQ(vec2[2], 3);
  EXPECT_EQ(vec1.size(), 0);  // moved from
}

TEST(SmallVectorTest, MoveAssignmentStackStorage) {
  small_vector<std::string, 4> vec1;
  vec1.push_back("hello");
  vec1.push_back("world");

  small_vector<std::string, 4> vec2;
  vec2.push_back("existing");

  vec2 = std::move(vec1);
  EXPECT_EQ(vec2.size(), 2);
  EXPECT_EQ(vec2[0], "hello");
  EXPECT_EQ(vec2[1], "world");

  EXPECT_EQ(vec1.size(), 0);
}

TEST(SmallVectorTest, MoveAssignmentHeapStorage) {
  small_vector<int, 2> vec1;
  for (int i = 0; i < 10; ++i) {
    vec1.push_back(i);
  }

  small_vector<int, 2> vec2;
  vec2.push_back(999);

  vec2 = std::move(vec1);
  EXPECT_EQ(vec2.size(), 10);
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(vec2[i], i);
  }

  EXPECT_EQ(vec1.size(), 0);
}

TEST(SmallVectorTest, SelfAssignment) {
  small_vector<int> vec{1, 2, 3, 4, 5};

  // self-assignment should be safe
  vec = vec;

  EXPECT_EQ(vec.size(), 5);
  for (int i = 0; i < 5; ++i) {
    EXPECT_EQ(vec[i], i + 1);
  }
}

TEST(SmallVectorTest, SelfMoveAssignment) {
  small_vector<int> vec{1, 2, 3, 4, 5};

  // self-move-assignment should be safe
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-move"
  vec = std::move(vec);
#pragma GCC diagnostic pop

  EXPECT_EQ(vec.size(), 5);
  for (int i = 0; i < 5; ++i) {
    EXPECT_EQ(vec[i], i + 1);
  }
}

// ==================== Generic methods ====================
TEST(SmallVectorTest, PushBackCopy) {
  small_vector<int> vec;
  int val = 42;
  vec.push_back(val);
  val = 2;

  EXPECT_EQ(vec.size(), 1);
  EXPECT_EQ(vec[0], 42);
}

TEST(SmallVectorTest, PushBackMove) {
  small_vector<std::string> vec;
  std::string str = "hello";
  vec.push_back(std::move(str));

  EXPECT_EQ(vec.size(), 1);
  EXPECT_EQ(vec[0], "hello");
  EXPECT_TRUE(str.empty());  // string was moved
}

TEST(SmallVectorTest, EmplaceBack) {
  small_vector<std::string> vec;
  vec.emplace_back("hello");
  vec.emplace_back(5, 'a');  // string with 5 'a's

  EXPECT_EQ(vec.size(), 2);
  EXPECT_EQ(vec[0], "hello");
  EXPECT_EQ(vec[1], "aaaaa");
}

TEST(SmallVectorTest, PopBack) {
  // simple type
  {
    small_vector<int> vec{1, 2, 3, 4, 5};

    vec.pop_back();
    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(vec[3], 4);

    vec.pop_back();
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[2], 3);

    vec.pop_back();
    vec.pop_back();
    vec.pop_back();
    EXPECT_EQ(vec.size(), 0);

    vec.pop_back();
    EXPECT_EQ(vec.size(), 0);
  }
  // string
  {
    small_vector<std::string> vec;
    vec.push_back("hello");
    vec.push_back("world");
    vec.push_back("test");

    vec.pop_back();
    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "world");
  }
}

TEST(SmallVectorTest, Insert) {
  small_vector<int> vec{2, 3, 4};

  // beginning
  auto it = vec.insert(vec.cbegin(), 1);
  EXPECT_EQ(vec.size(), 4);
  for (int i = 0; i < static_cast<int>(vec.size()); i++) {
    EXPECT_EQ(vec[i], i + 1);
  }
  EXPECT_EQ(it, vec.begin());

  // end
  it = vec.insert(vec.cend(), 5);
  EXPECT_EQ(vec.size(), 5);
  for (int i = 0; i < static_cast<int>(vec.size()); i++) {
    EXPECT_EQ(vec[i], i + 1);
  }
  EXPECT_EQ(it, vec.end() - 1);
}

TEST(SmallVectorTest, InsertInMiddle) {
  small_vector<int> vec{1, 3, 4};

  auto it = vec.insert(vec.cbegin() + 1, 2);
  EXPECT_EQ(vec.size(), 4);
  for (int i = 0; i < static_cast<int>(vec.size()); i++) {
    EXPECT_EQ(vec[i], i + 1);
  }
  EXPECT_EQ(it, vec.begin() + 1);
}

TEST(SmallVectorTest, InsertIntoEmpty) {
  small_vector<int> vec;

  auto it = vec.insert(vec.cbegin(), 42);
  EXPECT_EQ(vec.size(), 1);
  EXPECT_EQ(vec[0], 42);
  EXPECT_EQ(it, vec.begin());
}

TEST(SmallVectorTest, InsertMoveSemantics) {
  small_vector<std::string> vec{"hello", "world"};
  std::string str = "inserted";

  auto it = vec.insert(vec.cbegin() + 1, std::move(str));
  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec[0], "hello");
  EXPECT_EQ(vec[1], "inserted");
  EXPECT_EQ(vec[2], "world");
  EXPECT_TRUE(str.empty());
  EXPECT_EQ(it, vec.begin() + 1);
}

TEST(SmallVectorTest, InsertCausesReallocation) {
  small_vector<int, 2> vec{1, 2};
  EXPECT_EQ(vec.get_alloc(), 0);  // using stack

  vec.insert(vec.cbegin() + 1, 99);
  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec[0], 1);
  EXPECT_EQ(vec[1], 99);
  EXPECT_EQ(vec[2], 2);
  EXPECT_GT(vec.get_alloc(), 0);  // now using heap
}

TEST(SmallVectorTest, InsertOutOfRange) {
  small_vector<int> vec{1, 2, 3};

  EXPECT_THROW(vec.insert(vec.cbegin() + 4, 42), std::out_of_range);
  EXPECT_THROW(vec.insert(vec.cbegin() - 1, 42), std::out_of_range);
}

TEST(SmallVectorTest, Erase) {
  small_vector<int> vec{1, 2, 3, 4};

  // beginning
  auto it = vec.erase(vec.cbegin());
  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec[0], 2);
  EXPECT_EQ(vec[1], 3);
  EXPECT_EQ(vec[2], 4);
  EXPECT_EQ(it, vec.begin());

  // end
  it = vec.erase(vec.cend() - 1);
  EXPECT_EQ(vec.size(), 2);
  EXPECT_EQ(vec[0], 2);
  EXPECT_EQ(vec[1], 3);
  EXPECT_EQ(it, vec.end());
}

TEST(SmallVectorTest, EraseInMiddle) {
  small_vector<int> vec{1, 2, 3, 4};

  auto it = vec.erase(vec.cbegin() + 1);
  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec[0], 1);
  EXPECT_EQ(vec[1], 3);
  EXPECT_EQ(vec[2], 4);
  EXPECT_EQ(it, vec.begin() + 1);
}

TEST(SmallVectorTest, EraseLastElement) {
  small_vector<int> vec{42};

  auto it = vec.erase(vec.cbegin());
  EXPECT_EQ(vec.size(), 0);
  EXPECT_EQ(it, vec.end());
}

TEST(SmallVectorTest, EraseStrings) {
  small_vector<std::string> vec{"hello", "world", "test"};

  vec.erase(vec.cbegin() + 1);
  EXPECT_EQ(vec.size(), 2);
  EXPECT_EQ(vec[0], "hello");
  EXPECT_EQ(vec[1], "test");
}

TEST(SmallVectorTest, EraseRange) {
  small_vector<int> vec{1, 2, 3, 4, 5, 6};

  // remove a part
  auto it = vec.erase(vec.cbegin() + 1, vec.cbegin() + 4);
  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec[0], 1);
  EXPECT_EQ(vec[1], 5);
  EXPECT_EQ(vec[2], 6);
  EXPECT_EQ(it, vec.begin() + 1);

  // remove everything
  it = vec.erase(vec.cbegin(), vec.cend());
  EXPECT_EQ(vec.size(), 0);
  EXPECT_EQ(it, vec.end());
}

TEST(SmallVectorTest, EraseRangeEmpty) {
  small_vector<int> vec{1, 2, 3, 4, 5};

  auto it = vec.erase(vec.cbegin() + 2, vec.cbegin() + 2);
  EXPECT_EQ(vec.size(), 5);
  EXPECT_EQ(it, vec.begin() + 2);
  for (int i = 0; i < 5; ++i) {
    EXPECT_EQ(vec[i], i + 1);
  }
}

TEST(SmallVectorTest, EraseRangeAtBeginning) {
  small_vector<int> vec{1, 2, 3, 4, 5};

  auto it = vec.erase(vec.cbegin(), vec.cbegin() + 2);
  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec[0], 3);
  EXPECT_EQ(vec[1], 4);
  EXPECT_EQ(vec[2], 5);
  EXPECT_EQ(it, vec.begin());
}

TEST(SmallVectorTest, EraseRangeAtEnd) {
  small_vector<int> vec{1, 2, 3, 4, 5};

  auto it = vec.erase(vec.cbegin() + 3, vec.cend());
  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec[0], 1);
  EXPECT_EQ(vec[1], 2);
  EXPECT_EQ(vec[2], 3);
  EXPECT_EQ(it, vec.end());
}

TEST(SmallVectorTest, EraseOutOfRange) {
  small_vector<int> vec{1, 2, 3};

  EXPECT_THROW(vec.erase(vec.cbegin() + 3), std::out_of_range);
  EXPECT_THROW(vec.erase(vec.cbegin() - 1), std::out_of_range);
  EXPECT_THROW(vec.erase(vec.cbegin() + 2, vec.cbegin() + 1),
               std::out_of_range);
}

TEST(SmallVectorTest, InsertEraseSequence) {
  small_vector<int> vec;

  vec.insert(vec.cend(), 1);
  vec.insert(vec.cend(), 2);
  vec.insert(vec.cbegin(), 0);
  vec.insert(vec.cbegin() + 2, 15);
  EXPECT_EQ(vec.size(), 4);
  EXPECT_EQ(vec[0], 0);
  EXPECT_EQ(vec[1], 1);
  EXPECT_EQ(vec[2], 15);
  EXPECT_EQ(vec[3], 2);

  vec.erase(vec.cbegin() + 2);
  vec.erase(vec.cbegin());
  EXPECT_EQ(vec.size(), 2);
  EXPECT_EQ(vec[0], 1);
  EXPECT_EQ(vec[1], 2);

  vec.pop_back();
  vec.pop_back();
  EXPECT_EQ(vec.size(), 0);
}

TEST(SmallVectorTest, Reserve) {
  small_vector<int> vec;
  vec.reserve(20);

  EXPECT_GE(vec.capacity(), 20);
  EXPECT_EQ(vec.size(), 0);
}

TEST(SmallVectorTest, Resize) {
  small_vector<int> vec;
  vec.resize(5, 42);

  EXPECT_EQ(vec.size(), 5);
  for (size_t i = 0; i < 5; ++i) {
    EXPECT_EQ(vec[i], 42);
  }
}

TEST(SmallVectorTest, ResizeDown) {
  small_vector<int> vec{1, 2, 3, 4, 5};
  vec.resize(3);

  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec[0], 1);
  EXPECT_EQ(vec[1], 2);
  EXPECT_EQ(vec[2], 3);
}

TEST(SmallVectorTest, Clear) {
  small_vector<int> vec{1, 2, 3, 4, 5};
  vec.clear();

  EXPECT_EQ(vec.size(), 0);
}

// ==================== Element Access ====================
TEST(SmallVectorTest, Iterators) {
  small_vector<int> vec{1, 2, 3, 4, 5};

  int expected = 1;
  for (auto it = vec.begin(); it != vec.end(); ++it) {
    EXPECT_EQ(*it, expected++);
  }
  expected = 1;
  for (const auto& val : vec) {
    EXPECT_EQ(val, expected++);
  }
  expected = 1;
  for (auto it = vec.cbegin(); it != vec.cend(); ++it) {
    EXPECT_EQ(*it, expected++);
  }
  expected = 5;
  for (auto it = vec.rbegin(); it != vec.rend(); ++it) {
    EXPECT_EQ(*it, expected--);
  }

  // modify through reverse iterator
  for (auto it = vec.rbegin(); it != vec.rend(); ++it) {
    *it *= 10;
  }
  EXPECT_EQ(vec[0], 10);
  EXPECT_EQ(vec[1], 20);
  EXPECT_EQ(vec[2], 30);
  EXPECT_EQ(vec[3], 40);
  EXPECT_EQ(vec[4], 50);
}

TEST(SmallVectorTest, ConstIterators) {
  const small_vector<int> vec{1, 2, 3, 4, 5};

  int expected = 1;
  for (auto it = vec.begin(); it != vec.end(); ++it) {
    EXPECT_EQ(*it, expected++);
  }
}

TEST(SmallVectorTest, FrontBack) {
  small_vector<int> vec{1, 2, 3, 4, 5};

  EXPECT_EQ(vec.front(), 1);
  EXPECT_EQ(vec.back(), 5);

  vec.front() = 10;
  vec.back() = 50;

  EXPECT_EQ(vec.front(), 10);
  EXPECT_EQ(vec.back(), 50);
  EXPECT_EQ(vec[0], 10);
  EXPECT_EQ(vec[4], 50);
}

TEST(SmallVectorTest, Data) {
  small_vector<int> vec{1, 2, 3};
  int* ptr = vec.data();

  EXPECT_EQ(ptr[0], 1);
  EXPECT_EQ(ptr[1], 2);
  EXPECT_EQ(ptr[2], 3);
}

TEST(SmallVectorTest, BoundsChecking) {
  const auto check_l = [](auto&& vec) {
    EXPECT_EQ(vec.at(0), 1);
    EXPECT_EQ(vec.at(1), 2);
    EXPECT_EQ(vec.at(2), 3);

    EXPECT_THROW(vec.at(3), std::out_of_range);
    EXPECT_THROW(vec.at(10), std::out_of_range);
  };

  small_vector<int> vec1{1, 2, 3};
  check_l(vec1);
  const small_vector<int> cvec{1, 2, 3};
  check_l(cvec);
}

// ==================== Swap ====================
TEST(SmallVectorTest, Swap) {
  small_vector<int> vec1{1, 2, 3};
  small_vector<int> vec2{4, 5};

  vec1.swap(vec2);

  EXPECT_EQ(vec1.size(), 2);
  EXPECT_EQ(vec1[0], 4);
  EXPECT_EQ(vec1[1], 5);

  EXPECT_EQ(vec2.size(), 3);
  EXPECT_EQ(vec2[0], 1);
  EXPECT_EQ(vec2[1], 2);
  EXPECT_EQ(vec2[2], 3);
}

TEST(SmallVectorTest, FreeSwapFunction) {
  small_vector<int> vec1{1, 2, 3};
  small_vector<int> vec2{4, 5};

  swap(vec1, vec2);

  EXPECT_EQ(vec1.size(), 2);
  EXPECT_EQ(vec1[0], 4);
  EXPECT_EQ(vec1[1], 5);

  EXPECT_EQ(vec2.size(), 3);
  EXPECT_EQ(vec2[0], 1);
  EXPECT_EQ(vec2[1], 2);
  EXPECT_EQ(vec2[2], 3);
}

TEST(SmallVectorTest, SwapBothStack) {
  small_vector<int, 8> vec1{1, 2, 3};
  small_vector<int, 8> vec2{4, 5};

  vec1.swap(vec2);

  EXPECT_EQ(vec1.size(), 2);
  EXPECT_EQ(vec1[0], 4);
  EXPECT_EQ(vec1[1], 5);

  EXPECT_EQ(vec2.size(), 3);
  EXPECT_EQ(vec2[0], 1);
  EXPECT_EQ(vec2[1], 2);
  EXPECT_EQ(vec2[2], 3);
}

TEST(SmallVectorTest, SwapBothHeap) {
  small_vector<int, 2> vec1;
  small_vector<int, 2> vec2;

  for (int i = 0; i < 10; ++i) {
    vec1.push_back(i);
  }
  for (int i = 10; i < 15; ++i) {
    vec2.push_back(i);
  }

  vec1.swap(vec2);

  EXPECT_EQ(vec1.size(), 5);
  for (int i = 10; i < 15; ++i) {
    EXPECT_EQ(vec1[i - 10], i);
  }
  EXPECT_EQ(vec2.size(), 10);
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(vec2[i], i);
  }
}

TEST(SmallVectorTest, SwapMixedStackHeap) {
  small_vector<int, 4> vec1{1, 2, 3};
  small_vector<int, 4> vec2;

  for (int i = 0; i < 10; ++i) {
    vec2.push_back(i + 10);
  }

  EXPECT_EQ(vec1.get_alloc(), 0);  // Stack
  EXPECT_GT(vec2.get_alloc(), 0);  // Heap

  vec1.swap(vec2);

  EXPECT_EQ(vec1.size(), 10);
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(vec1[i], i + 10);
  }

  EXPECT_EQ(vec2.size(), 3);
  EXPECT_EQ(vec2[0], 1);
  EXPECT_EQ(vec2[1], 2);
  EXPECT_EQ(vec2[2], 3);
}

// ==================== Behaviour ====================
TEST(SmallVectorTest, SmallBufferOptimization) {
  // test that small vectors use the internal buffer
  small_vector<int, 4> vec;
  for (int i = 0; i < 4; ++i) {
    vec.push_back(i);
  }

  EXPECT_EQ(vec.size(), 4);
  EXPECT_EQ(vec.capacity(), 4);
  EXPECT_EQ(vec.get_alloc(), 0);  // should still be using buffer
}

TEST(SmallVectorTest, GrowthStrategy) {
  small_vector<int, 4> vec;

  for (int i = 0; i < 4; ++i) {
    vec.push_back(i);
  }
  EXPECT_EQ(vec.get_alloc(), 0);  // still using stack

  vec.push_back(4);
  EXPECT_GT(vec.get_alloc(), 4);  // now using heap

  const auto first_cap = vec.capacity();
  while (vec.size() < first_cap) {
    vec.push_back(static_cast<int>(vec.size()));
  }

  const auto old_cap = vec.capacity();
  vec.push_back(999);
  const auto new_cap = vec.capacity();

  EXPECT_GT(new_cap, old_cap);

  for (size_t i = 0; i < vec.size() - 1; ++i) {
    EXPECT_EQ(vec[i], static_cast<int>(i));
  }
  EXPECT_EQ(vec[vec.size() - 1], 999);
}

TEST(SmallVectorTest, GrowthBeyondBuffer) {
  // test growth beyond the internal buffer
  small_vector<int, 4> vec;
  for (int i = 0; i < 10; ++i) {
    vec.push_back(i);
  }

  EXPECT_EQ(vec.size(), 10);
  EXPECT_GT(vec.capacity(), 4);
  EXPECT_GT(vec.get_alloc(), 0);  // should now be using heap

  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(vec[i], i);
  }
}

// ==================== Misc ====================
TEST(SmallVectorTest, StringVector) {
  small_vector<std::string> vec;
  vec.push_back("hello");
  vec.push_back("world");
  vec.emplace_back("test");

  EXPECT_EQ(vec.size(), 3);
  EXPECT_EQ(vec[0], "hello");
  EXPECT_EQ(vec[1], "world");
  EXPECT_EQ(vec[2], "test");
}

// ==================== Main ====================
int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "smallVector.hpp"

using namespace mpc;

TEST(SmallVectorTest, DefaultConstructor) {
    smallVector<int> vec;
    EXPECT_EQ(vec.size(), 0);
    EXPECT_GE(vec.capacity(), 8);  // default N = 8
}

TEST(SmallVectorTest, SizeConstructor) {
    smallVector<int> vec(5);
    EXPECT_EQ(vec.size(), 5);
    EXPECT_GE(vec.capacity(), 5);
}

TEST(SmallVectorTest, InitializerListConstructor) {
    smallVector<int> vec{1, 2, 3, 4, 5};
    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);
    EXPECT_EQ(vec[3], 4);
    EXPECT_EQ(vec[4], 5);
}

TEST(SmallVectorTest, CopyConstructor) {
    smallVector<int> vec1{1, 2, 3};
    smallVector<int> vec2(vec1);
    
    vec1[0] = 5;
    EXPECT_EQ(vec2.size(), 3);
    EXPECT_EQ(vec2[0], 1);
    EXPECT_EQ(vec2[1], 2);
    EXPECT_EQ(vec2[2], 3);
}

TEST(SmallVectorTest, MoveConstructor) {
    smallVector<int> vec1{1, 2, 3};
    smallVector<int> vec2(std::move(vec1));
    
    EXPECT_EQ(vec2.size(), 3);
    EXPECT_EQ(vec2[0], 1);
    EXPECT_EQ(vec2[1], 2);
    EXPECT_EQ(vec2[2], 3);
    EXPECT_EQ(vec1.size(), 0);  // moved from
}

TEST(SmallVectorTest, MoveConstructorStackStorage) {
    smallVector<std::string, 4> vec1;
    vec1.push_back("hello");
    vec1.push_back("world");
    
    // ensure vec1 is using stack storage
    EXPECT_EQ(vec1.getAlloc(), 0);
    
    smallVector<std::string, 4> vec2(std::move(vec1));
    EXPECT_EQ(vec2.size(), 2);
    EXPECT_EQ(vec2[0], "hello");
    EXPECT_EQ(vec2[1], "world");
    
    EXPECT_EQ(vec1.size(), 0);
}

TEST(SmallVectorTest, MoveConstructorHeapStorage) {
    smallVector<int, 2> vec1;
    for (int i = 0; i < 10; ++i) {
        vec1.push_back(i);
    }
    
    // ensure vec1 is using heap storage
    EXPECT_GT(vec1.getAlloc(), 0);
    
    smallVector<int, 2> vec2(std::move(vec1));
    EXPECT_EQ(vec2.size(), 10);
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(vec2[i], i);
    }
    
    EXPECT_EQ(vec1.size(), 0);
}

TEST(SmallVectorTest, CopyAssignment) {
    smallVector<int> vec1{1, 2, 3};
    smallVector<int> vec2;
    vec2 = vec1;
    vec1[0] = 5;
    
    EXPECT_EQ(vec2.size(), 3);
    EXPECT_EQ(vec2[0], 1);
    EXPECT_EQ(vec2[1], 2);
    EXPECT_EQ(vec2[2], 3);
}

TEST(SmallVectorTest, MoveAssignment) {
    smallVector<int> vec1{1, 2, 3};
    smallVector<int> vec2;
    vec2 = std::move(vec1);
    
    EXPECT_EQ(vec2.size(), 3);
    EXPECT_EQ(vec2[0], 1);
    EXPECT_EQ(vec2[1], 2);
    EXPECT_EQ(vec2[2], 3);
    EXPECT_EQ(vec1.size(), 0);  // moved from
}

TEST(SmallVectorTest, MoveAssignmentStackStorage) {
    smallVector<std::string, 4> vec1;
    vec1.push_back("hello");
    vec1.push_back("world");
    
    smallVector<std::string, 4> vec2;
    vec2.push_back("existing");
    
    vec2 = std::move(vec1);
    EXPECT_EQ(vec2.size(), 2);
    EXPECT_EQ(vec2[0], "hello");
    EXPECT_EQ(vec2[1], "world");
    
    EXPECT_EQ(vec1.size(), 0);
}

TEST(SmallVectorTest, MoveAssignmentHeapStorage) {
    smallVector<int, 2> vec1;
    for (int i = 0; i < 10; ++i) {
        vec1.push_back(i);
    }
    
    smallVector<int, 2> vec2;
    vec2.push_back(999);
    
    vec2 = std::move(vec1);
    EXPECT_EQ(vec2.size(), 10);
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(vec2[i], i);
    }
    
    EXPECT_EQ(vec1.size(), 0);
}

TEST(SmallVectorTest, PushBackCopy) {
    smallVector<int> vec;
    int val = 42;
    vec.push_back(val);
    val = 2;
    
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], 42);
}

TEST(SmallVectorTest, PushBackMove) {
    smallVector<std::string> vec;
    std::string str = "hello";
    vec.push_back(std::move(str));
    
    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_TRUE(str.empty());  // string was moved
}

TEST(SmallVectorTest, EmplaceBack) {
    smallVector<std::string> vec;
    vec.emplace_back("hello");
    vec.emplace_back(5, 'a');  // string with 5 'a's
    
    EXPECT_EQ(vec.size(), 2);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "aaaaa");
}

TEST(SmallVectorTest, Reserve) {
    smallVector<int> vec;
    vec.reserve(20);
    
    EXPECT_GE(vec.capacity(), 20);
    EXPECT_EQ(vec.size(), 0);
}

TEST(SmallVectorTest, GrowthStrategy) {
    smallVector<int, 4> vec;
    
    for (int i = 0; i < 4; ++i) {
        vec.push_back(i);
    }
    EXPECT_EQ(vec.getAlloc(), 0); // still using stack
    
    vec.push_back(4);
    EXPECT_GT(vec.getAlloc(), 4); // now using heap
    
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

TEST(SmallVectorTest, Resize) {
    smallVector<int> vec;
    vec.resize(5, 42);
    
    EXPECT_EQ(vec.size(), 5);
    for (size_t i = 0; i < 5; ++i) {
        EXPECT_EQ(vec[i], 42);
    }
}

TEST(SmallVectorTest, ResizeDown) {
    smallVector<int> vec{1, 2, 3, 4, 5};
    vec.resize(3);
    
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);
}

TEST(SmallVectorTest, Clear) {
    smallVector<int> vec{1, 2, 3, 4, 5};
    vec.clear();
    
    EXPECT_EQ(vec.size(), 0);
}

TEST(SmallVectorTest, Iterators) {
    smallVector<int> vec{1, 2, 3, 4, 5};
    
    int expected = 1;
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        EXPECT_EQ(*it, expected++);
    }
    
    expected = 1;
    for (const auto& val : vec) {
        EXPECT_EQ(val, expected++);
    }
}

TEST(SmallVectorTest, ConstIterators) {
    const smallVector<int> vec{1, 2, 3, 4, 5};
    
    int expected = 1;
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        EXPECT_EQ(*it, expected++);
    }
}

TEST(SmallVectorTest, Data) {
    smallVector<int> vec{1, 2, 3};
    int* ptr = vec.data();
    
    EXPECT_EQ(ptr[0], 1);
    EXPECT_EQ(ptr[1], 2);
    EXPECT_EQ(ptr[2], 3);
}

TEST(SmallVectorTest, Swap) {
    smallVector<int> vec1{1, 2, 3};
    smallVector<int> vec2{4, 5};
    
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
    smallVector<int> vec1{1, 2, 3};
    smallVector<int> vec2{4, 5};
    
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
    smallVector<int, 8> vec1{1, 2, 3};
    smallVector<int, 8> vec2{4, 5};
    
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
    smallVector<int, 2> vec1;
    smallVector<int, 2> vec2;
    
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
    smallVector<int, 4> vec1{1, 2, 3};
    smallVector<int, 4> vec2;
    
    for (int i = 0; i < 10; ++i) {
        vec2.push_back(i + 10);
    }
    
    EXPECT_EQ(vec1.getAlloc(), 0); // Stack
    EXPECT_GT(vec2.getAlloc(), 0); // Heap
    
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

TEST(SmallVectorTest, SmallBufferOptimization) {
    // test that small vectors use the internal buffer
    smallVector<int, 4> vec;
    for (int i = 0; i < 4; ++i) {
        vec.push_back(i);
    }
    
    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(vec.capacity(), 4);
    EXPECT_EQ(vec.getAlloc(), 0);  // should still be using buffer
}

TEST(SmallVectorTest, GrowthBeyondBuffer) {
    // test growth beyond the internal buffer
    smallVector<int, 4> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }
    
    EXPECT_EQ(vec.size(), 10);
    EXPECT_GT(vec.capacity(), 4);
    EXPECT_GT(vec.getAlloc(), 0);  // should now be using heap
    
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(vec[i], i);
    }
}

TEST(SmallVectorTest, StringVector) {
    smallVector<std::string> vec;
    vec.push_back("hello");
    vec.push_back("world");
    vec.emplace_back("test");
    
    EXPECT_EQ(vec.size(), 3);
    EXPECT_EQ(vec[0], "hello");
    EXPECT_EQ(vec[1], "world");
    EXPECT_EQ(vec[2], "test");
}

TEST(SmallVectorTest, BoundsChecking) {
    smallVector<int> vec{1, 2, 3};
    
    EXPECT_EQ(vec.at(0), 1);
    EXPECT_EQ(vec.at(1), 2);
    EXPECT_EQ(vec.at(2), 3);
    
    EXPECT_THROW(vec.at(3), std::out_of_range);
    EXPECT_THROW(vec.at(10), std::out_of_range);
}

TEST(SmallVectorTest, ConstBoundsChecking) {
    const smallVector<int> vec{1, 2, 3};
    
    EXPECT_EQ(vec.at(0), 1);
    EXPECT_EQ(vec.at(1), 2);
    EXPECT_EQ(vec.at(2), 3);
    
    EXPECT_THROW(vec.at(3), std::out_of_range);
}

TEST(SmallVectorTest, SelfAssignment) {
    smallVector<int> vec{1, 2, 3, 4, 5};
    
    // self-assignment should be safe
    vec = vec;
    
    EXPECT_EQ(vec.size(), 5);
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(vec[i], i + 1);
    }
}

TEST(SmallVectorTest, SelfMoveAssignment) {
    smallVector<int> vec{1, 2, 3, 4, 5};
    
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

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
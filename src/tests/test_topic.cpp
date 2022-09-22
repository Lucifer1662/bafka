#include <gtest/gtest.h>
// #include <topic.h>
// #include <bafka/topic.h>
// #include <bafka_client/bad.h>
// #include <topic.h>
#include <bafka/topic.h>

// #include <gah.h>

// int main(int argc, char** argv) {
//     testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// }

// Demonstrate some basic assertions.
TEST(TestTopic, ReadTest) {
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}
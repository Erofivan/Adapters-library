#include <processing.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

TEST(FlowConvertionsTest, AsDataFlow) {
    std::vector<int> input = {1, 2, 3, 4, 5};
    auto flow = AsDataFlow(input);
    ASSERT_THAT(flow, testing::ElementsAre(1, 2, 3, 4, 5));
}

TEST(FlowConvertionsTest, AsVector) {
    std::vector<int> input = {1, 2, 3, 4, 5};
    auto result = AsDataFlow(input) | AsVector();
    ASSERT_THAT(result, testing::ElementsAreArray(std::vector<int>{1, 2, 3, 4, 5}));
}

TEST(FlowConvertionsTest, MoveAsDataFlow) {
    auto flow = AsDataFlow(std::vector{1, 2, 3, 4, 5});
    ASSERT_THAT(flow, testing::ElementsAre(1, 2, 3, 4, 5));
}

TEST(FlowConvertionsTest, CopyAsDataFlow) {
    auto flow = ([]{
        std::vector<int> local_input = {1, 2, 3, 4, 5};
        return CopyAsDataFlow(local_input);
    })();
    ASSERT_THAT(flow, testing::ElementsAre(1, 2, 3, 4, 5));
}

TEST(FlowConvertionsTest, ExpiringAsVector) {
    auto result = AsDataFlow(std::vector{1, 2, 3, 4, 5}) | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(1, 2, 3, 4, 5));
}

TEST(FlowConvertionsTest, AsList){
    auto flow = AsDataFlow(std::vector{1, 2, 3, 4, 5});
    auto result = flow | AsContainer<std::list>();
    static_assert(std::same_as<decltype(result), std::list<int, std::allocator<int>>>);
    ASSERT_THAT(result, testing::ElementsAre(1, 2, 3, 4, 5));
}

#include <processing.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cctype>

TEST(FilterTest, FilterEven) {
    std::vector<int> input = {1, 2, 3, 4, 5};
    auto result = AsDataFlow(input) | Filter([](int x) { return x % 2 == 0; }) | AsVector();
    ASSERT_THAT(result, testing::ElementsAre(2, 4));
}

TEST(FilterTest, FilterUpperCase) {
    std::vector<std::string> input = {"hello", "world", "HELLO", "WORLD"};
    auto result =
        AsDataFlow(input)
            | Filter([](const std::string& x) { return std::all_of(x.begin(), x.end(), [](char c) { return std::isupper(c); }); })
            | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("HELLO", "WORLD"));
}

struct Entry {
    int left;
    int right;

    bool operator==(const Entry&) const = default;
};

bool IsGood(const Entry& e){
    return e.left <= e.right;
}

TEST(FilterRest, FilterByFunctionPtr){
    auto input = std::vector<Entry>{
        {0, 1},
        {1, 1},
        {1, 0},
        {1, 10},
        {10, 1},
        {10, 10},
    };
    auto result = AsDataFlow(input) | Filter(IsGood);
    ASSERT_THAT(result, testing::ElementsAre(
        Entry{0, 1},
        Entry{1, 1},
        Entry{1, 10},
        Entry{10, 10}
    ));
}

auto even = [](int x) { return x % 2 == 0; };

TEST(FilterRest, ContainerPreserve){
    std::vector<int> input = {1, 2, 3, 4, 5};
    auto flow = CopyAsDataFlow<std::list>(input) | Filter(even);
    auto& result = flow.content();
    static_assert(std::same_as<std::remove_cvref_t<decltype(result)>, std::list<int>>);
    ASSERT_THAT(result, testing::ElementsAre(2, 4));
}

TEST(FilterRest, ContainerSwitch){
    std::vector<int> input = {1, 2, 3, 4, 5};
    auto flow = AsDataFlow(input) | Filter<std::list>(even);
    auto& result = flow.content();
    static_assert(std::same_as<std::remove_cvref_t<decltype(result)>, std::list<int>>);
    ASSERT_THAT(result, testing::ElementsAre(2, 4));
}
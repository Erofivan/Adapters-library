#include <processing.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <list>

TEST(ReadTest, ByNewLine) {
    std::vector<std::stringstream> files(2);
    files[0] << "1\n2\n3\n4\n5";
    files[1] << "6\n7\n8\n9\n10";
    auto result = AsDataFlow(files) | Split("\n") | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("1", "2", "3", "4", "5", "6", "7", "8", "9", "10"));
}

TEST(ReadTest, BySpace) {
    std::vector<std::stringstream> files(2);
    files[0] << "1 2 3 4 5";
    files[1] << "6 7 8 9 10";
    auto result = AsDataFlow(files) | Split(" ") | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("1", "2", "3", "4", "5", "6", "7", "8", "9", "10"));
}

// new

TEST(ReadTest, FromString){
    std::vector<std::string> strings(2);
    strings[0] = "1 2 3 4 5";
    strings[1] = "6 7 8 9 10";
    auto result = AsDataFlow(strings) | Split(" ") | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("1", "2", "3", "4", "5", "6", "7", "8", "9", "10"));
}

TEST(ReadTest, SimpleWords){
    std::vector<std::string> input = {"ab c def", "4 55 666 0x7", "@@$@; L:#$SD(A)!"};
    auto result = AsDataFlow(input) | Split(" ");
    ASSERT_THAT(result, testing::ElementsAre("ab","c","def","4","55","666","0x7","@@$@;","L:#$SD(A)!"));
}

TEST(ReadTest, MultiDelim){
    std::vector<std::stringstream> files(2);
    files[0] << "1\n2 3|4;5";
    files[1] << "6.7\n8 9/10";
    auto result = AsDataFlow(files) | Split("\n ;|") | AsVector();
    ASSERT_THAT(result, testing::ElementsAre("1", "2", "3", "4", "5", "6.7", "8", "9/10"));
}

TEST(ReadTest, ContainerPreserve){
    std::vector<std::string> input = {"ab c def", "4 55 666 0x7", "@@$@; L:#$SD(A)!"};
    auto flow = CopyAsDataFlow<std::list>(input) | Split(" ");
    auto& result = flow.content();
    static_assert(std::same_as<std::remove_cvref_t<decltype(result)>, std::list<std::string>>);
    ASSERT_THAT(result, testing::ElementsAre("ab","c","def","4","55","666","0x7","@@$@;","L:#$SD(A)!"));
}

TEST(ReadTest, ContainerSwitch){
    std::vector<std::string> input = {"ab c def", "4 55 666 0x7", "@@$@; L:#$SD(A)!"};
    auto flow = AsDataFlow(input) | Split<std::list>(" ");
    auto& result = flow.content();
    static_assert(std::same_as<std::remove_cvref_t<decltype(result)>, std::list<std::string>>);
    ASSERT_THAT(result, testing::ElementsAre("ab","c","def","4","55","666","0x7","@@$@;","L:#$SD(A)!"));
}
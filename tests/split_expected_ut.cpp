#include <processing.hpp>
#include <tests/util.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <expected>
#include <list>

struct Department {
    std::string name;

    bool operator==(const Department& other) const = default;
};

std::expected<Department, std::string> ParseDepartment(const std::string& str) {
    if (str.empty()) {
        return std::unexpected("Department name is empty");
    }
    if (str.contains(' ')) {
        return std::unexpected("Department name contains space");
    }
    return Department{str};
}

TEST(SplitExpectedTest, SplitExpected) {
    std::vector<std::stringstream> files(1);
    files[0] << "good-department|bad department||another-good-department";

    auto [unexpected_flow, good_flow] = AsDataFlow(files) 
        | Split("|") 
        | Transform(ParseDepartment) 
        | SplitExpected();

    std::stringstream unexpected_file;
    unexpected_flow | Write(unexpected_file, '.');

    auto expected_result = good_flow | AsVector();

    ASSERT_EQ(unexpected_file.str(), "Department name contains space.Department name is empty.");
    ASSERT_THAT(expected_result, testing::ElementsAre(
        Department{"good-department"}, 
        Department{"another-good-department"}));
}

TEST(SplitExpectedTest, StringToInt) {
    auto [bad, good] = AsDataFlow(std::vector<std::string>{
        "1",
        "",
        "-22",
        "$10",
        "999999999999",
        "0",
        "0xCDEF",
        "xCDEF"})
        | Transform([](const std::string str) -> std::expected<int, int>{
            try{
                return std::stoi(str, nullptr, 16);
            }
            catch(std::invalid_argument&){
                return std::unexpected(0);
            }
            catch(std::out_of_range&){
                return std::unexpected(1);
            }
        }) | SplitExpected();
    
    ASSERT_THAT(good | AsVector(), testing::ElementsAre(1, -0x22, 0, 0xCDEF));
    ASSERT_THAT(bad | AsVector(), testing::ElementsAre(0, 0, 1, 0));
}

TEST(SplitExpectedTest, ContainerPreserve) {
    auto [bad, good] = CopyAsDataFlow<std::list, LoudAlloc<std::string>>(
            std::vector<std::string>{
                "1",
                "",
                "-22",
                "$10",
                "999999999999",
                "0",
                "0xCDEF",
                "xCDEF"})
        | Transform([](const std::string str) -> std::expected<int, int>{
            try{
                return std::stoi(str, nullptr, 16);
            }
            catch(std::invalid_argument&){
                return std::unexpected(0);
            }
            catch(std::out_of_range&){
                return std::unexpected(1);
            }
        }) | SplitExpected();

    auto& good_result = good.content();
    auto& bad_result = bad.content();
    
    ASSERT_THAT(good_result, testing::ElementsAre(1, -0x22, 0, 0xCDEF));
    ASSERT_THAT(bad_result, testing::ElementsAre(0, 0, 1, 0));
    
    static_assert(std::same_as<
        std::remove_cvref_t<decltype(good_result)>, 
        std::list<int, LoudAlloc<int>>>);
    static_assert(std::same_as<
        std::remove_cvref_t<decltype(bad_result)>, 
        std::list<int, LoudAlloc<int>>>);
}

TEST(SplitExpectedTest, ContainerSwitch) {
    auto [bad, good] = AsDataFlow(std::vector<std::string>{
        "1",
        "",
        "-22",
        "$10",
        "999999999999",
        "0",
        "0xCDEF",
        "xCDEF"})
        | Transform([](const std::string str) -> std::expected<int, std::string>{
            try{
                return std::stoi(str, nullptr, 16);
            }
            catch(std::invalid_argument&){
                return std::unexpected("invalid_argument");
            }
            catch(std::out_of_range&){
                return std::unexpected("out_of_range");
            }
        }) | SplitExpected<std::list, LoudAlloc<void*>>();

    auto& good_result = good.content();
    auto& bad_result = bad.content();
    
    ASSERT_THAT(good_result, testing::ElementsAre(1, -0x22, 0, 0xCDEF));
    ASSERT_THAT(bad_result, testing::ElementsAre(
        "invalid_argument",
        "invalid_argument", 
        "out_of_range", 
        "invalid_argument"));
    
    static_assert(std::same_as<
        std::remove_cvref_t<decltype(good_result)>, 
        std::list<int, LoudAlloc<int>>>);
    static_assert(std::same_as<
        std::remove_cvref_t<decltype(bad_result)>, 
        std::list<std::string, LoudAlloc<std::string>>>);
}
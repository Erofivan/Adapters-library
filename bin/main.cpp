// #include <iostream>

// #include <processing.hpp>

// // This program counts the frequency of words occrring in all files 
// // within a directory and its subdirectories recursively, 
// // and outputs the result to the console.
// int main(int argc, char *argv[]) {

//     if (argc != 2) {
//         return EXIT_FAILURE;
//     }

//     bool recursive = true;

//     Dir(argv[1], recursive)
//       | Filter([](std::filesystem::path& p){ return (p.extension() == ".hpp" || p.extension() == ".cpp"); })
//       | OpenFiles()
//       | Split("\n ,.;")
//       | Transform(
//           [](std::string& token) {
//               std::transform(token.begin(), token.end(), token.begin(), [](char c){return std::tolower(c);});
//               return token;
//           })
//       | AggregateByKey(
//           0uz,
//           [](const std::string&, size_t& count) { ++count;},
//           [](const std::string& token) { return token;}
//         )
//       | Transform([](const std::pair<std::string, size_t>& stat) { return std::format("{} - {}", stat.first, stat.second);})
//       | Out(std::cout);

//     return EXIT_SUCCESS;
// }

#include <iostream>

#include <processing.hpp>

#include <unordered_map>
#include <vector>

int main(
    // int argc, char **argv
) {

  const std::vector<KV<int, int>> v = {{1,1}, {2,2}, {3,3}, {4,4}, {5,5}};

  std::unordered_map<int, int> mp = {{1,1}, {2,2}, {3,3}, {4,4}, {5,5}};


  auto f1 = AsDataFlow(v)
     | Transform([](const KV<int, int>& a){ return KV<int,int>{a.key, 5 * a.value};})
     | Filter([](const KV<int, int>& a){ return a.value % 2 == 0;});

  auto f2 = AsDataFlow(mp) | Join(f1,  
      [](const std::pair<int, int>& val){ return KV<int, int>{val.first, val.second}; },
      [](const KV<int, int>& val){ return val; });
  
  for (auto& i : f2) {
    std::cout << i.base << '\n';
    if (i.joined.has_value()) {
      std::cout << i.joined.value();
    }
    std::cout << '\n';
  }
}
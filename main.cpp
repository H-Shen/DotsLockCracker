#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <array>
#include <unordered_map>

constexpr int N = 8;

// Mapping to PS5 keys
const std::string_view L2 = "L2";
const std::string_view R2 = "R2";
const std::string_view X = "X";

struct HashFunction {
    // Reference: https://www.boost.org/doc/libs/1_64_0/boost/functional/hash/hash.hpp
    size_t operator()(const std::array<int, N> &obj) const {
        size_t hash_value{0};
        for (const auto &i: obj) {
            hash_value ^= std::hash<int>{}(i) + 0x9e3779b9 + (hash_value << 6) + (hash_value >> 2);
        }
        return hash_value;
    }
};

int step;
std::array<int, N> target_pattern;
std::array<int, N> original_pattern;
std::unordered_map<std::array<int, N>, int, HashFunction> memo;
std::unordered_map<std::array<int, N>, std::pair<std::array<int, N>, std::string_view>, HashFunction> parent;

// Search the shortest path by Depth-First Search
void dfs(const std::array<int, N> &current_pattern,
         const std::array<int, N> &previous_pattern,
         const std::string_view &operation,
         int current_step) {
    // prune 1
    if (current_step > step) {
        return;
    }
    // prune 2
    auto iter = memo.find(current_pattern);
    if (iter != memo.end()) {
        if (current_step > iter->second) {
            return;
        } else {
            iter->second = current_step;
            parent.insert_or_assign(current_pattern, std::make_pair(previous_pattern, operation));
        }
    } else {
        memo.emplace(current_pattern, current_step);
        parent.insert_or_assign(current_pattern, std::make_pair(previous_pattern, operation));
    }
    // prune 3
    if (current_pattern == target_pattern) {
        step = current_step;
        return;
    }
    // L2
    auto pattern_copy(current_pattern);
    std::rotate(pattern_copy.begin(), pattern_copy.begin() + 2, pattern_copy.end());
    dfs(pattern_copy, current_pattern, L2, current_step + 1);
    // R2
    pattern_copy = current_pattern;
    rotate(pattern_copy.rbegin(), pattern_copy.rbegin() + 2, pattern_copy.rend());
    dfs(pattern_copy, current_pattern, R2, current_step + 1);
    // prune 4: Rotate the middle three elements if not all elements are the same
    if (!(current_pattern[3] == current_pattern[4] && current_pattern[4] == current_pattern[5])) {
        pattern_copy = current_pattern;
        std::rotate(pattern_copy.begin() + 3, pattern_copy.begin() + 4, pattern_copy.begin() + 6);
        dfs(pattern_copy, current_pattern, X, current_step + 1);
    }
}

inline
std::vector<std::pair<std::array<int, N>, std::string_view>> get_path() {
    std::vector<std::pair<std::array<int, N>, std::string_view>> path;
    std::pair<std::array<int, N>, std::string_view> current_pattern;
    current_pattern.first = target_pattern;
    while (true) {
        path.emplace_back(current_pattern);
        if (current_pattern.first == original_pattern) {
            break;
        }
        current_pattern = parent[current_pattern.first];
    }
    std::reverse(path.begin(), path.end());
    return path;
}

inline
bool valid_array(const std::string &pattern, std::array<int, N> &arr) {
    if (static_cast<int>(pattern.size()) == N && std::all_of(pattern.begin(), pattern.end(), [](const char &ch) {
        return ch >= '0' && ch <= '0' + N;
    })) {
        for (int i = 0; i < N; ++i) {
            arr[i] = pattern[i] - '0';
        }
    }
}

inline
bool valid_arguments(int argc, char **argv) {
    return argc == 3 && valid_array(argv[1], original_pattern) && valid_array(argv[2], target_pattern);
}


// Usage:   ./DotsLockCracker [original pattern] [target pattern]
// Example: ./DotsLockCracker 30000013 03100030
int main(int argc, char **argv) {
    // Suppose each number represents a color in the slot, input the colors clockwise from 12 o'clock as an array
    // You can use any number from 0-8 to represent a color, for example, 0 for no-color, 1 for yellow, 2 for blue, 3 for red, etc.
    // You need give the app two arrays to present the original pattern and the target pattern

    // The arrays are passing as two separated strings as arguments.
    if (!valid_arguments(argc, argv)) {
        std::cerr << "Invalid arguments." << std::endl;
        std::cerr << "Usage: " << argv[0] << " [original pattern] [target pattern]" << std::endl;
        std::cerr << "Example: " << argv[0] << " 30000013 03100030" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    step = std::numeric_limits<int>::max();
    dfs(original_pattern, original_pattern, std::string_view(), 0);
    // No solution found (For example, the user gives an invalid input)
    if (step == std::numeric_limits<int>::max()) {
        std::cerr << "No solution found, please check your input." << '\n';
        std::exit(EXIT_FAILURE);
    }
    // Print the number of steps
    std::cout << "Total steps: " << step << '\n';
    get_path();
    // Print the shortest path of each status
    auto path = get_path();
    std::cout << '\n';
    for (const auto &i: path) {
        for (const auto &j: i.first) {
            std::cout << j << ' ';
        }
        std::cout << '\n';
    }
    std::cout << '\n';
    // Print the shortest path of operations to reach the target status
    for (const auto &i: path) {
        std::cout << i.second << ' ';
    }
    std::cout << '\n';
    return 0;
}

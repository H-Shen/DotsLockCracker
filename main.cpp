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
std::array<int, N> target_status;
std::array<int, N> original_status;
std::unordered_map<std::array<int, N>, int, HashFunction> memo;
std::unordered_map<std::array<int, N>, std::pair<std::array<int, N>, std::string_view>, HashFunction> parent;

// Search the shortest path by DFS
void dfs(const std::array<int, N> &current_status,
         const std::array<int, N> &previous_status,
         const std::string_view &operation,
         int current_step) {
    // prune 1
    if (current_step > step) {
        return;
    }
    // prune 2
    auto iter = memo.find(current_status);
    if (iter != memo.end()) {
        if (current_step > iter->second) {
            return;
        } else {
            iter->second = current_step;
            parent.insert_or_assign(current_status, std::make_pair(previous_status, operation));
        }
    } else {
        memo.emplace(current_status, current_step);
        parent.insert_or_assign(current_status, std::make_pair(previous_status, operation));
    }
    // prune 3
    if (current_status == target_status) {
        step = current_step;
        return;
    }
    // L2
    auto status_copy(current_status);
    std::rotate(status_copy.begin(), status_copy.begin() + 2, status_copy.end());
    dfs(status_copy, current_status, L2, current_step + 1);
    // R2
    status_copy = current_status;
    rotate(status_copy.rbegin(), status_copy.rbegin() + 2, status_copy.rend());
    dfs(status_copy, current_status, R2, current_step + 1);
    // prune 4: Rotate the middle three elements if not all elements are the same
    if (!(current_status[3] == current_status[4] && current_status[4] == current_status[5])) {
        status_copy = current_status;
        std::rotate(status_copy.begin() + 3, status_copy.begin() + 4, status_copy.begin() + 6);
        dfs(status_copy, current_status, X, current_step + 1);
    }
}

inline
std::vector<std::pair<std::array<int, N>, std::string_view>> get_path() {
    std::vector<std::pair<std::array<int, N>, std::string_view>> path;
    std::pair<std::array<int, N>, std::string_view> current_status;
    current_status.first = target_status;
    while (true) {
        path.emplace_back(current_status);
        if (current_status.first == original_status) {
            break;
        }
        current_status = parent[current_status.first];
    }
    std::reverse(path.begin(), path.end());
    return path;
}


inline
bool valid_input(const std::string &pattern, std::array<int, N> &arr) {
    std::vector<std::string> vec;
    std::string temp;
    for (const char &ch: pattern) {
        if (static_cast<int>(vec.size()) > N) {
            return false;
        }
        if (isspace(ch)) {
            if (!temp.empty()) {
                vec.emplace_back(temp);
                temp.clear();
            }
        } else {
            if (!isdigit(ch)) {
                return false;
            }
            temp.push_back(ch);
        }
    }
    if (!temp.empty()) {
        vec.emplace_back(temp);
    }
    if (static_cast<int>(vec.size()) != N) {
        return false;
    }
    for (int i = 0; i < N; ++i) {
        try {
            arr[i] = std::stoi(vec[i]);
            if (arr[i] < 0 || arr[i] > N) {
                return false;
            }
        } catch (...) {
            return false;
        }
    }
    return true;
}

int main() {
    // Suppose each number represents a color in the slot, input the colors clockwise from 12 o'clock as an array
    // You can use any number from 0-8 to represent a color, for example, 0 for no-color, 1 for yellow, 2 for blue, 3 for red, etc
    // You need give the app two arrays to present the original pattern and the target pattern, just follow the prompt
    std::string original_pattern;
    std::cout << "Original Pattern: ";
    std::getline(std::cin, original_pattern);
    if (!valid_input(original_pattern, original_status)) {
        std::cerr << "Invalid input." << '\n';
        std::exit(EXIT_FAILURE);
    }
    std::string target_pattern;
    std::cout << "Target Pattern: ";
    std::getline(std::cin, target_pattern);
    if (!valid_input(target_pattern, target_status)) {
        std::cerr << "Invalid input." << '\n';
        std::exit(EXIT_FAILURE);
    }
    // original_status = {3,0,0,0,0,0,1,3};
    // target_status = {0,3,1,0,0,0,3,0};
    step = std::numeric_limits<int>::max();
    dfs(original_status, original_status, std::string_view(), 0);
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

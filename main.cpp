#include <algorithm>
#include <array>
#include <iostream>
#include <string>
#include <unordered_map>
#include <limits>
#include <vector>
#include <queue>

// #define TEST

#ifdef TEST
#include <random>
constexpr int MAX_MOVES = 50;
#endif

// Constants
constexpr int N = 8;
constexpr int NUM_ARGS = 3;
const std::string_view L2 = "L2"; // Map Playstation 5 L2 to string "L2"
const std::string_view R2 = "R2"; // Map Playstation 5 R2 to string "R2"
const std::string_view X = "X";   // Map Playstation 5 X to string "X"

// Custom hash for std::array<int, N> to use in unordered containers
struct HashFunction {
    // Reference:
    // https://www.boost.org/doc/libs/1_64_0/boost/functional/hash/hash.hpp
    size_t operator()(const std::array<int, N> &obj) const {
        size_t hash_value{0};
        for (const auto &i: obj) {
            hash_value ^= std::hash<int>{}(i) + 0x9e3779b9 + (hash_value << 6) +
                          (hash_value >> 2);
        }
        return hash_value;
    }
};

// Depth-first search solver with pruning and memoization
// current_pattern: current state
// previous_pattern: parent state to record in 'parent' map
// target_pattern: goal state
// operation: operation leading to current_pattern
// current_step: steps taken so far
// step: best known solution depth (will be updated)
// memo: map of state -> best depth found so far
// parent: map of state -> (parent, operation)
void dfs_solver(const std::array<int, N> &current_pattern,
                const std::array<int, N> &previous_pattern,
                const std::array<int, N> &target_pattern,
                const std::string_view &operation, int current_step, int &step,
                std::unordered_map<std::array<int, N>, int, HashFunction> &memo,
                std::unordered_map<std::array<int, N>,
                        std::pair<std::array<int, N>, std::string_view>,
                        HashFunction> &parent

) {
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
            parent.insert_or_assign(current_pattern,
                                    std::make_pair(previous_pattern, operation));
        }
    } else {
        memo.emplace(current_pattern, current_step);
        parent.insert_or_assign(current_pattern,
                                std::make_pair(previous_pattern, operation));
    }
    // prune 3
    if (current_pattern == target_pattern) {
        step = current_step;
        return;
    }
    // L2
    auto pattern_copy(current_pattern);
    std::rotate(pattern_copy.begin(), pattern_copy.begin() + 2,
                pattern_copy.end());
    dfs_solver(pattern_copy, current_pattern, target_pattern, L2, current_step + 1, step,
               memo, parent);
    // R2
    pattern_copy = current_pattern;
    rotate(pattern_copy.rbegin(), pattern_copy.rbegin() + 2, pattern_copy.rend());
    dfs_solver(pattern_copy, current_pattern, target_pattern, R2, current_step + 1, step,
               memo, parent);
    // prune 4: Rotate the middle three elements if not all elements are the same
    if (!(current_pattern[3] == current_pattern[4] &&
          current_pattern[4] == current_pattern[5])) {
        pattern_copy = current_pattern;
        std::rotate(pattern_copy.begin() + 3, pattern_copy.begin() + 4,
                    pattern_copy.begin() + 6);
        dfs_solver(pattern_copy, current_pattern, target_pattern, X, current_step + 1,
                   step, memo, parent);
    }
}

// Reconstruct the path from 'parent' map for DFS
// original_pattern: starting state
// target_pattern: goal state
// parent: recorded parent pointers
inline std::vector<std::pair<std::array<int, N>, std::string_view>>
get_path(const std::array<int, N> &original_pattern,
         const std::array<int, N> &target_pattern,
         std::unordered_map<std::array<int, N>,
                 std::pair<std::array<int, N>, std::string_view>,
                 HashFunction> &parent) {
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

// Validate input string and parse into integer pattern array
inline bool valid_array(const std::string &pattern, std::array<int, N> &arr) {
    if (static_cast<int>(pattern.size()) == N &&
        std::all_of(pattern.begin(), pattern.end(),
                    [](const char &ch) { return ch >= '0' && ch <= '0' + N; })) {
        for (int i = 0; i < N; ++i) {
            arr[i] = pattern[i] - '0';
        }
        return true;
    }
    return false;
}

// Breadth-first search solver that guarantees shortest path
// original_pattern: starting state
// target_pattern: goal state
// min_step: out parameter for length of shortest path (updated if found)
// path: out parameter for one shortest path (states + operations)
void bfs_solver(const std::array<int, N> &original_pattern,
                const std::array<int, N> &target_pattern,
                int &min_step,
                std::vector<std::pair<std::array<int, N>, std::string_view>> &path) {
    // Lambdas for operations
    auto apply_l2 = [](const std::array<int, N> &src) {
        std::array<int, N> dst = src;
        std::rotate(dst.begin(), dst.begin() + 2, dst.end());
        return dst;
    };
    auto apply_r2 = [](const std::array<int, N> &src) {
        std::array<int, N> dst = src;
        std::rotate(dst.begin(), dst.end() - 2, dst.end());
        return dst;
    };
    auto apply_x = [](const std::array<int, N> &src) {
        std::array<int, N> dst = src;
        std::rotate(dst.begin() + 3, dst.begin() + 4, dst.begin() + 6);
        return dst;
    };

    std::queue<std::array<int, N>> q;
    std::unordered_map<std::array<int, N>, bool, HashFunction> visited;
    std::unordered_map<std::array<int, N>, std::pair<std::array<int, N>, std::string_view>, HashFunction> parent;

    q.push(original_pattern);
    visited[original_pattern] = true;
    parent[original_pattern] = {original_pattern, std::string_view{}};

    // BFS loop
    while (!q.empty()) {
        std::array<int, N> u{q.front()};
        q.pop();
        if (u == target_pattern) {
            break;
        }
        // L2
        auto next = apply_l2(u);
        if (!visited[next]) {
            visited[next] = true;
            parent[next] = {u, L2};
            q.push(next);
        }
        // R2
        next = apply_r2(u);
        if (!visited[next]) {
            visited[next] = true;
            parent[next] = {u, R2};
            q.push(next);
        }
        // X if applicable
        if (!(u[3] == u[4] && u[4] == u[5])) {
            next = apply_x(u);
            if (!visited[next]) {
                visited[next] = true;
                parent[next] = {u, X};
                q.push(next);
            }
        }
    }

    // Check if solution found
    if (parent.find(target_pattern) == parent.end()) {
        return;  // min_step unchanged indicates no solution
    }
    // Reconstruct path
    path.clear();
    auto cur = target_pattern;
    while (true) {
        auto pr = parent[cur];
        path.emplace_back(cur, pr.second);
        if (cur == original_pattern) {
            break;
        }
        cur = pr.first;
    }
    std::reverse(path.begin(), path.end());
    min_step = static_cast<int>(path.size()) - 1;
}

#ifdef TEST
// Differential testing between DFS and BFS
void test_random(int test_count) {
    std::mt19937 rng(std::random_device{}());
    for (int case_idx = 1; case_idx <= test_count; ++case_idx) {
        // generate two random patterns with some moves to simplify the complexity of test cases
        std::array<int, N> orig{};
        std::iota(orig.begin(), orig.end(), 0);
        std::array<int, N> tgt = orig;
        int moves = (int)rng() % (MAX_MOVES + 1);
        for (int m = 0; m < moves; ++m) {
            int op = (int)rng() % 3;
            if (op == 0) {
                std::rotate(tgt.begin(), tgt.begin() + 2, tgt.end());
            } else if (op == 1) {
                std::rotate(tgt.begin(), tgt.end() - 2, tgt.end());
            } else {
                if (!(tgt[3] == tgt[4] && tgt[4] == tgt[5])) {
                    std::rotate(tgt.begin() + 3, tgt.begin() + 4, tgt.begin() + 6);
                }
            }
        }

        std::cout << "Test " << case_idx << ":\n";
        std::cout << "Orig: ";
        for (int v: orig) std::cout << v;
        std::cout << "\n";
        std::cout << "Tgt : ";
        for (int v: tgt) std::cout << v;
        std::cout << "\n";

        // DFS solver
        int min_step_dfs = std::numeric_limits<int>::max();
        std::unordered_map<std::array<int, N>, int, HashFunction> memo;
        std::unordered_map<std::array<int, N>, std::pair<std::array<int, N>, std::string_view>, HashFunction> parent;
        dfs_solver(orig, orig, tgt, std::string_view{}, 0,
                   min_step_dfs, memo, parent);

        // BFS solver
        int min_step_bfs = std::numeric_limits<int>::max();
        std::vector<std::pair<std::array<int, N>, std::string_view>> path;
        bfs_solver(orig, tgt, min_step_bfs, path);
        std::cout << "Min steps = " << min_step_bfs << '\n';

        if (min_step_dfs != min_step_bfs) {
            std::cout << "Mismatch in case " << case_idx
                      << ": DFS=" << min_step_dfs
                      << ", BFS=" << min_step_bfs << "\n";
            return;
        }
    }
    std::cout << "All " << test_count << " random tests passed!\n";
}
#endif


// Usage:   ./DotsLockCracker [original pattern] [target pattern]
// Example: ./DotsLockCracker 30000013 03100030
// Main entry point
// Without TEST defined: runs BFS from command-line input
int main(int argc, char **argv) {
#ifdef TEST
    test_random(10);
    return 0;
#endif
    // Suppose each number represents a color in the slot, input the colors
    // clockwise from 12 o'clock as an array You can use any number from 0-8 to
    // represent a color, for example, 0 for no-color, 1 for yellow, 2 for blue, 3
    // for red, etc. You need give the app two arrays to present the original
    // pattern and the target pattern
    std::array<int, N> target_pattern{};
    std::array<int, N> original_pattern{};
    // The arrays are passing as two separated strings as arguments.
    if (argc != NUM_ARGS || !valid_array(argv[1], original_pattern) ||
        !valid_array(argv[2], target_pattern)) {
        std::cerr << "Invalid arguments." << std::endl;
        std::cerr << "Usage: " << argv[0] << " [original pattern] [target pattern]"
                  << std::endl;
        std::cerr << "Example: " << argv[0] << " 30000013 03100030" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    int min_steps = std::numeric_limits<int>::max();
    std::vector<std::pair<std::array<int, N>, std::string_view>> path;
    bfs_solver(original_pattern, target_pattern, min_steps, path);
    // No solution found (For example, the user gives an invalid input)
    if (min_steps == std::numeric_limits<int>::max()) {
        std::cerr << "No solution found, please check your input." << '\n';
        std::exit(EXIT_FAILURE);
    }
    // Print the number of steps
    std::cout << "Total steps: " << min_steps << '\n';
    // Print the shortest path of each status
    std::cout << '\n';
    for (const auto &i: path) {
        for (const auto &j: i.first) {
            std::cout << j << ' ';
        }
        std::cout << '\n';
    }
    std::cout << '\n';
    // Print the shortest path of operations to reach the target status
    if (!path.empty()) {
        for (int idx = 1; idx < (int) path.size(); ++idx) {
            if (idx > 1) {
                std::cout << ' ';
            }
            std::cout << path[idx].second;
        }
    }
    std::cout << '\n';
    return EXIT_SUCCESS;
}

# DSU code and explanation for SIT221

Start with **DSU_Complete.cpp**. This is one complete C++17 program with one `main` function. It contains the basic example, optimised DSU, Kruskal, demonstrations and 17 automated test groups. No input files or third-party libraries are needed. Do not paste the old demonstration program into it; that would create a second `main`.

**Verification status:** no C++ compiler was available in the preparation environment, so this file has not been compiled or executed there. Expected outputs are labelled accordingly. Run the tests on your computer before recording. The package supplies tests; it does not claim that they have already passed in C++.

## Files

| File | Purpose |
|---|---|
| `DSU_Complete.cpp` | Complete commented implementation, demos and tests |
| `build_and_test.ps1` | Optional Windows build-and-test helper |
| `EXPECTED_DEMO_OUTPUT.txt` | Expected arrays, connectivity results and MST decisions |
| `EXPECTED_TEST_OUTPUT.txt` | Expected names and summary of the 17 test groups |
| `README.md` | Run instructions, explanations and recording sequence |

## Compile and run on Windows

Extract the ZIP first. Open a terminal in the extracted `DSU_Code_Pack` folder. With a configured g++ compiler, run:

```powershell
g++ -std=c++17 -O2 -Wall -Wextra -pedantic DSU_Complete.cpp -o dsu.exe
```

If compilation succeeds, run:

```powershell
.\dsu.exe --tests
.\dsu.exe --all
```

No keyboard input is required. `--tests` should end with:

```text
Tests passed: 17, failed: 0
```

That is the expected result, not a guarantee. If you see `FAIL` or a compiler error, resolve it before recording. A failed test or invalid mode returns a nonzero exit code. With a successful run, `$LASTEXITCODE` is 0.

The optional helper performs compilation and testing together:

```powershell
.\build_and_test.ps1
```

You may provide a compiler outside PATH:

```powershell
.\build_and_test.ps1 -CompilerPath 'C:\path\to\g++.exe'
```

Replace the example path with the real compiler location. If PowerShell blocks scripts, use the manual compile commands above; changing execution policy is unnecessary. If `g++` is not recognised, you need a configured C++ compiler. An editor alone is not a compiler. You can also use an existing C++17 IDE project: add this file as its sole source file with `main`, then build and run. With no arguments the program runs all demonstrations and tests.

On Linux or macOS with a configured g++ or clang++ compiler, use `-o dsu` and run `./dsu --all`. For clang++, replace the compiler command name. The source itself is portable standard C++17.

## Select the demonstration you want

| Command | What it shows |
|---|---|
| `.\dsu.exe --basic` | A long parent chain without optimisations |
| `.\dsu.exe --dsu` | Union by size, compression, connectivity and group sizes |
| `.\dsu.exe --kruskal` | Accepted and skipped edges, selected tree and total weight |
| `.\dsu.exe --tests` | All automated tests |
| `.\dsu.exe --all` | Everything; also the default with no arguments |
| `.\dsu.exe --help` | Available modes |

## 1 BasicDSU

This first class demonstrates the underlying idea without optimisations. `parent_[x]` stores the parent of item `x`. A root points to itself. `find(x)` repeatedly follows parent pointers until it reaches a root. `unite(a,b)` finds the roots and places the second root below the first. It returns false when both items already have the same root.

The demo deliberately calls `unite(1,0)`, `unite(2,1)`, `unite(3,2)` and `unite(4,3)`. The result is:

```text
Index:   0  1  2  3  4
Parent:  1  2  3  4  4
```

Finding 0 follows 0, 1, 2, 3, 4. The call does not change the array. A bad sequence can create a chain with linear height, so a find or union can take O(n) time. This class illustrates why the optimisations are useful.

## 2 Optimised DSU

The main class uses **union by size**, not union by rank. The two approaches are related, but use different metadata. Keep your spoken explanation consistent with the actual code.

### Stored data and initialisation

| Member | Meaning |
|---|---|
| `parent_` | Parent of every item |
| `size_` | Number of items in a group, valid at its current root |
| `groups_` | Current number of disjoint groups |

The constructor rejects a negative size, creates the arrays and uses `std::iota` to initialise parents to `0, 1, ..., n-1`. Every item begins as its own root and has group size one. A size of zero is allowed, but there is then no valid vertex to query.

### find and path compression

```cpp
if (parent_[x] != x)
    parent_[x] = find(parent_[x]);
return parent_[x];
```

1. `check(x)` rejects negative and out-of-range indices before array access.
2. If `x` is a root, return it.
3. Otherwise, recursively find the root of its parent.
4. On return, store that root directly in `parent_[x]`.

For a chain where 3 points to 2 and 2 points to 0, `find(3)` returns 0 and changes `parent_[3]` to 0. It preserves set membership and the group count. Only the tree representation changes. Union by size bounds this implementation's recursive depth logarithmically; this is not a general recommendation to recurse over arbitrarily deep chains.

### unite and union by size

```cpp
a = find(a);
b = find(b);
if (a == b) return false;
if (size_[a] < size_[b]) std::swap(a, b);
parent_[b] = a;
size_[a] += size_[b];
--groups_;
return true;
```

The variables `a` and `b` become roots. Equal roots mean that the union request changes nothing. Otherwise, swapping ensures that `a` is the root of a group at least as large as `b`. We attach `b` below `a`, update the surviving root's size and decrease the number of groups once. Equal sizes keep the first root, which makes our trace reproducible.

Linking roots merges whole groups and preserves parent chains. Attaching an arbitrary non-root can corrupt the partition. Updating the size at a non-root also gives incorrect metadata. The function is named `unite` because `union` is a C++ keyword.

### Other methods

- `connected(a,b)` compares `find(a)` and `find(b)`. Immediate parents may differ even when representatives match.
- `componentSize(x)` finds the root before reading its size.
- `count()` returns the number of groups.
- `parents()` returns a read-only reference for display. It does not call `find`, so displaying the structure cannot accidentally compress it before the explanation.

### Trace used in the PPT

| Operation | Parent array | Groups |
|---|---|---|
| Start | `[0,1,2,3,4,5]` | 6 |
| `unite(0,1)` | `[0,0,2,3,4,5]` | 5 |
| `unite(2,3)` | `[0,0,2,2,4,5]` | 4 |
| `unite(0,2)` | `[0,0,0,2,4,5]` | 3 |
| `find(3)` | `[0,0,0,0,4,5]` | 3 |

The group containing 3 has size four. Items 0 and 5 are still disconnected. After `unite(4,5)` and `unite(3,5)`, all six items are connected. Repeating `unite(0,5)` returns false and keeps the group count at one.

## 3 Kruskal

`Edge` stores `u`, `v` and an integer `weight`. Each edge is undirected. `Result` stores the selected edges, total weight and remaining number of components. The total uses `long long` so sums larger than a 32-bit integer remain representable for the demonstrated inputs.

The algorithm performs these steps:

1. Construct DSU with one component per vertex and validate all endpoints.
2. Sort a copy of the edges by weight. Endpoint tie-breaks make the trace deterministic; they do not change minimum-weight correctness.
3. Call `unite(e.u,e.v)` for each edge.
4. If it returns true, accept the edge and add its weight.
5. If it returns false, skip the edge because its endpoints already share a component.
6. Return the selected forest and component count.

The current accepted edges form a forest. Equal roots mean an accepted path already connects the endpoints, so adding another edge would close a cycle. Kruskal supplies the greedy weight ordering; DSU only supplies connectivity checking. The cut property justifies accepting a lightest edge between the relevant components. Cycle avoidance alone would not guarantee minimum cost without the ordering.

For the five-vertex example, accepted weights are 1, 2, 4 and 5. Weight 3 is skipped because 0 and 2 already connect through 1. The final total is **12**, with **four edges** and **one component**. A disconnected graph produces a minimum spanning forest rather than an MST spanning every vertex. Self-loops are skipped. Negative weights, parallel edges and ties are supported.

The teaching version scans all edges to show every rejection. For a connected graph, you could stop after accepting `V-1` edges. It does not compute shortest paths or model network capacity and redundancy requirements.

## 4 Automated tests

There are **17 named test groups**, including two larger cross-checks:

- Thirty small random connectivity trials, each with fifty merges, compare DSU with independent breadth-first searches. They check connectivity, component sizes, group counts and whether a merge really occurred.
- Sixty small random connected graphs compare Kruskal's total with exhaustive enumeration of every candidate spanning tree. They also check that selected edges span the vertices without cycles. These test graphs contain at most seven edges so exhaustive search stays small.

The other tests cover the basic chain, initial groups, duplicates, self-merges, exact compression, size-based root swapping, invalid inputs, empty and one-vertex graphs, the main MST, a disconnected forest with an isolated vertex, tied weights, negative weights, self-loops, parallel edges and a total of 4,000,000,000.

`expect` throws if a condition is false. The test runner reports each group and counts failures. `expectThrows` confirms that invalid inputs produce the intended exception. Unlike plain `assert`, these checks remain active with ordinary release compilation. The BFS and exhaustive-search helpers exist to test the algorithms; you do not need to teach their entire implementation in the video.

## 5 Complexity

| Part | Time | Storage |
|---|---|---|
| Basic DSU setup | O(n) | O(n) |
| Basic find or union, worst case | O(n) | O(1) extra for iterative find |
| Optimised DSU setup | O(n) | O(n) |
| Optimised operations | O(alpha(n)) amortised each | O(n) overall, plus a logarithmically bounded recursive stack |
| Kruskal in this program | O(V + E log E + E alpha(V)) | O(V + E) |

`alpha` is the inverse Ackermann function. The amortised bound concerns total work across a sequence, not a promise that every operation takes exactly constant time. Union by size alone limits height to O(log n), since an item's depth can increase only when its group at least doubles. The test harness has its own costs, especially exhaustive search; it is not included in the production algorithm bounds.

## 6 What to show in your video

1. Use `--basic` briefly to show why a long chain is inefficient.
2. Open the `DSU` class and explain the three stored values, constructor, `find` and `unite`.
3. Run `--dsu`. Predict the array before `find(3)`, then explain the changed entry.
4. Open `kruskal`, explain sorting and the `unite` return value, then run `--kruskal`.
5. Explain the weight-3 rejection and add the four accepted weights to obtain 12.
6. Run `--tests`. Explain duplicate merges, invalid indices, disconnected graphs and one independent cross-check. Report your actual output.

The optimised DSU and weighted example match the separate PPT. Use this complete file instead of the earlier `DSU_Demonstration.cpp`; it expands the testing and adds a basic comparison without changing the key example. Keep your face visible when switching from slides to the editor and terminal.

## Technical references

- [Princeton Algorithms: Union-Find](https://algs4.cs.princeton.edu/15uf/)
- [Princeton: weighted quick-union with path compression](https://algs4.cs.princeton.edu/15uf/WeightedQuickUnionPathCompressionUF.java.html)
- [Princeton Algorithms: Minimum Spanning Trees](https://algs4.cs.princeton.edu/43mst/)

The program is a teaching implementation of these standard algorithmic ideas. The examples and test harness are supplied for this presentation, not copied solutions to other students' assessment problems.

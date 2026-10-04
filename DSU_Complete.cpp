#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <random>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

class BasicDSU
{
    std::vector<int> parent_;
    int groups_;

    void check(int x) const
    {
        if (x < 0 || x >= static_cast<int>(parent_.size()))
            throw std::out_of_range("Invalid vertex");
    }

public:
    explicit BasicDSU(int n) : groups_(n)
    {
        if (n < 0)
            throw std::invalid_argument("Negative size");
        parent_.resize(n);
        std::iota(parent_.begin(), parent_.end(), 0);
    }
    int find(int x) const
    {
        check(x);
        while (parent_[x] != x)
            x = parent_[x];
        return x;
    }
    bool unite(int a, int b)
    {
        a = find(a);
        b = find(b);
        if (a == b)
            return false;
        parent_[b] = a;
        --groups_;
        return true;
    }
    int count() const { return groups_; }
    const std::vector<int> &parents() const { return parent_; }
};

class DSU
{
    std::vector<int> parent_;
    std::vector<int> size_;
    int groups_;

    void check(int x) const
    {
        if (x < 0 || x >= static_cast<int>(parent_.size()))
            throw std::out_of_range("Invalid vertex");
    }

public:
    explicit DSU(int n) : groups_(n)
    {
        if (n < 0)
            throw std::invalid_argument("Negative size");
        parent_.resize(n);
        size_.assign(n, 1);
        std::iota(parent_.begin(), parent_.end(), 0);
    }

    int find(int x)
    {
        check(x);
        if (parent_[x] != x)
            parent_[x] = find(parent_[x]);
        return parent_[x];
    }

    bool unite(int a, int b)
    {
        check(a);
        check(b);
        a = find(a);
        b = find(b);
        if (a == b)
            return false;
        if (size_[a] < size_[b])
            std::swap(a, b);
        parent_[b] = a;
        size_[a] += size_[b];
        --groups_;
        return true;
    }

    bool connected(int a, int b)
    {
        check(a);
        check(b);
        return find(a) == find(b);
    }
    int componentSize(int x) { return size_[find(x)]; }
    int count() const { return groups_; }

    const std::vector<int> &parents() const { return parent_; }
};

struct Edge
{
    int u, v, weight;
};
struct Result
{
    std::vector<Edge> chosen;
    long long total = 0;
    int components = 0;
};

Result kruskal(int n, std::vector<Edge> edges, bool trace = false)
{
    DSU dsu(n);
    for (const auto &e : edges)
        if (e.u < 0 || e.u >= n || e.v < 0 || e.v >= n)
            throw std::out_of_range("Invalid edge endpoint");

    std::sort(edges.begin(), edges.end(), [](const Edge &a, const Edge &b)
              { return std::tie(a.weight, a.u, a.v) < std::tie(b.weight, b.u, b.v); });
    Result result;
    for (const auto &e : edges)
    {
        const bool accepted = dsu.unite(e.u, e.v);
        if (trace)
            std::cout << e.u << '-' << e.v << " weight " << e.weight
                      << (accepted ? " ACCEPT\n" : " SKIP\n");
        if (accepted)
        {
            result.chosen.push_back(e);
            result.total += e.weight;
        }
    }
    result.components = dsu.count();
    return result;
}

template <class Structure>
void show(const std::string &label, const Structure &d)
{
    std::cout << label << ": [";
    const auto &p = d.parents();
    for (std::size_t i = 0; i < p.size(); ++i)
        std::cout << (i ? ", " : "") << p[i];
    std::cout << "], groups=" << d.count() << '\n';
}

void basicDemo()
{
    std::cout << "BASIC DSU\n";
    BasicDSU d(5);
    show("Start", d);

    d.unite(1, 0);
    d.unite(2, 1);
    d.unite(3, 2);
    d.unite(4, 3);
    show("After chain-building merges", d);
    std::cout << "find(0) = " << d.find(0) << '\n';
    show("After find (no compression)", d);
}

void dsuDemo()
{
    std::cout << "OPTIMISED DSU\n";
    DSU d(6);
    show("Start", d);
    d.unite(0, 1);
    show("unite(0,1)", d);
    d.unite(2, 3);
    show("unite(2,3)", d);
    d.unite(0, 2);
    show("unite(0,2)", d);
    std::cout << "find(3) = " << d.find(3) << '\n';
    show("After compression", d);
    std::cout << "Group size of 3 = " << d.componentSize(3) << '\n';
    std::cout << "0 and 5 connected? " << d.connected(0, 5) << '\n';
    d.unite(4, 5);
    d.unite(3, 5);
    std::cout << "After two more merges: " << d.connected(0, 5)
              << ", groups=" << d.count() << '\n';
    std::cout << "Repeat unite(0,5): " << d.unite(0, 5)
              << ", groups=" << d.count() << '\n';
}

std::vector<Edge> exampleEdges()
{
    return {{0, 1, 1}, {1, 2, 2}, {0, 2, 3}, {2, 3, 4}, {3, 4, 5}, {2, 4, 6}, {1, 4, 7}};
}

void kruskalDemo()
{
    std::cout << "KRUSKAL\n";
    const Result r = kruskal(5, exampleEdges(), true);
    std::cout << "Selected edges:\n";
    for (const auto &e : r.chosen)
        std::cout << e.u << '-' << e.v << " weight " << e.weight << '\n';
    std::cout << "Total weight = " << r.total
              << ", edges = " << r.chosen.size()
              << ", components = " << r.components << '\n';
}

using Graph = std::vector<std::vector<int>>;
bool reachable(const Graph &graph, int start, int goal)
{
    std::vector<bool> seen(graph.size(), false);
    std::queue<int> todo;
    seen[start] = true;
    todo.push(start);
    while (!todo.empty())
    {
        const int u = todo.front();
        todo.pop();
        if (u == goal)
            return true;
        for (int v : graph[u])
            if (!seen[v])
            {
                seen[v] = true;
                todo.push(v);
            }
    }
    return false;
}

void expect(bool condition, const std::string &message)
{
    if (!condition)
        throw std::runtime_error(message);
}
template <class Exception, class Action>
void expectThrows(Action action)
{
    try
    {
        action();
    }
    catch (const Exception &)
    {
        return;
    }
    throw std::runtime_error("Expected exception was not thrown");
}

long long bruteMST(int n, const std::vector<Edge> &edges)
{
    long long best = std::numeric_limits<long long>::max();
    const unsigned limit = 1u << static_cast<unsigned>(edges.size());
    for (unsigned mask = 0; mask < limit; ++mask)
    {
        Graph g(n);
        int count = 0;
        long long total = 0;
        for (std::size_t i = 0; i < edges.size(); ++i)
            if (mask & (1u << i))
            {
                const auto &e = edges[i];
                g[e.u].push_back(e.v);
                g[e.v].push_back(e.u);
                ++count;
                total += e.weight;
            }
        if (count != n - 1)
            continue;
        bool connected = true;
        for (int v = 0; v < n; ++v)
            if (!reachable(g, 0, v))
                connected = false;
        if (connected)
            best = std::min(best, total);
    }
    return best;
}

void runTests()
{
    int passed = 0, failed = 0;
    auto test = [&](const std::string &name, const std::function<void()> &action)
    {
        try
        {
            action();
            ++passed;
            std::cout << "PASS: " << name << '\n';
        }
        catch (const std::exception &e)
        {
            ++failed;
            std::cout << "FAIL: " << name << " - " << e.what() << '\n';
        }
    };
    test("basic chain", []
         {
        BasicDSU d(5);
        for (int i=1; i<5; ++i) d.unite(i, i-1);
        expect(d.parents()==std::vector<int>({1,2,3,4,4}), "chain mismatch");
        expect(d.find(0)==4 && d.count()==1, "wrong basic root or count"); });
    test("initial singleton groups", []
         {
        DSU d(6);
        expect(d.count()==6, "wrong initial count");
        for (int i=0; i<6; ++i)
            expect(d.find(i)==i && d.componentSize(i)==1, "wrong singleton"); });
    test("duplicate and self merges", []
         {
        DSU d(3);
        expect(d.unite(0,1), "first merge rejected");
        expect(!d.unite(1,0) && !d.unite(2,2), "redundant merge accepted");
        expect(d.count()==2 && d.componentSize(1)==2, "metadata changed"); });
    test("exact parent trace and compression", []
         {
        DSU d(6); d.unite(0,1); d.unite(2,3); d.unite(0,2);
        expect(d.parents()==std::vector<int>({0,0,0,2,4,5}), "before find");
        expect(d.find(3)==0, "wrong representative");
        expect(d.parents()==std::vector<int>({0,0,0,0,4,5}), "after find");
        expect(d.count()==3, "find changed group count"); });
    test("size-based swap", []
         {
        DSU d(4); d.unite(0,1); d.unite(2,0);
        expect(d.find(2)==0 && d.componentSize(2)==3, "larger root lost"); });
    test("negative size", []
         {
        expectThrows<std::invalid_argument>([] { DSU d(-1); });
        expectThrows<std::invalid_argument>([] { BasicDSU d(-1); }); });
    test("invalid DSU indices", []
         {
        DSU d(3);
        expectThrows<std::out_of_range>([&] { d.find(-1); });
        expectThrows<std::out_of_range>([&] { d.find(3); });
        expectThrows<std::out_of_range>([&] { d.unite(0,3); });
        expectThrows<std::out_of_range>([&] { d.connected(-1,0); });
        expect(d.count()==3, "invalid operation merged groups"); });
    test("empty and one-vertex graphs", []
         {
        DSU empty(0);
        expect(empty.count()==0, "empty DSU count");
        expectThrows<std::out_of_range>([&] { empty.find(0); });
        const auto a=kruskal(0,{}), b=kruskal(1,{});
        expect(a.components==0 && a.total==0 && a.chosen.empty(), "empty graph");
        expect(b.components==1 && b.total==0 && b.chosen.empty(), "one vertex"); });
    test("main MST weight 12", []
         {
        const auto r=kruskal(5,exampleEdges());
        expect(r.total==12 && r.chosen.size()==4 && r.components==1, "wrong MST"); });
    test("disconnected minimum spanning forest", []
         {
        const auto r=kruskal(5,{{0,1,2},{2,3,3}});
        expect(r.total==5 && r.chosen.size()==2 && r.components==3, "wrong forest"); });
    test("equal-weight alternatives", []
         {
        const auto r=kruskal(3,{{0,1,1},{1,2,1},{0,2,1}});
        expect(r.total==2 && r.chosen.size()==2 && r.components==1, "equal weights"); });
    test("negative weights and self loops", []
         {
        const auto r=kruskal(3,{{0,0,-100},{0,1,-2},{1,2,0},{0,2,5}});
        expect(r.total==-2 && r.chosen.size()==2, "self loop or weight error"); });
    test("parallel edges", []
         {
        const auto r=kruskal(2,{{0,1,8},{0,1,3},{1,0,3}});
        expect(r.total==3 && r.chosen.size()==1, "parallel edge error"); });
    test("invalid graph endpoints", []
         {
        expectThrows<std::out_of_range>([] { kruskal(3,{{0,3,1}}); });
        expectThrows<std::out_of_range>([] { kruskal(3,{{-1,2,1}}); });
        expectThrows<std::invalid_argument>([] { kruskal(-1,{}); }); });
    test("64-bit total", []
         {
        const auto r=kruskal(3,{{0,1,2000000000},{1,2,2000000000}});
        expect(r.total==4000000000LL, "weight total overflow"); });
    test("random connectivity compared with BFS", []
         {
        std::mt19937 random(221);
        for (int trial=0; trial<30; ++trial) {
            const int n=2+static_cast<int>(random()%9);
            DSU d(n); Graph g(n);
            for (int step=0; step<50; ++step) {
                const int a=static_cast<int>(random()%n);
                const int b=static_cast<int>(random()%n);
                const bool previouslyConnected=reachable(g,a,b);
                expect(d.unite(a,b)==!previouslyConnected, "merge return mismatch");
                g[a].push_back(b); g[b].push_back(a);
                int components=0;
                for (int x=0; x<n; ++x) {
                    bool first=true; int members=0;
                    for (int y=0; y<n; ++y) {
                        const bool same=reachable(g,x,y);
                        expect(d.connected(x,y)==same, "connectivity mismatch");
                        if (same) { ++members; if (y<x) first=false; }
                    }
                    if (first) ++components;
                    expect(d.componentSize(x)==members, "component size mismatch");
                }
                expect(d.count()==components, "component count mismatch");
            }
        } });
    test("random MST compared with exhaustive search", []
         {
        std::mt19937 random(1221);
        for (int trial=0; trial<60; ++trial) {
            const int n=2+static_cast<int>(random()%4);
            std::vector<Edge> edges;

            for (int v=1; v<n; ++v)
                edges.push_back({v-1,v,static_cast<int>(random()%11)-5});
            for (int k=0; k<3; ++k)
                edges.push_back({static_cast<int>(random()%n),
                                 static_cast<int>(random()%n),
                                 static_cast<int>(random()%11)-5});
            const auto r=kruskal(n,edges);
            expect(r.total==bruteMST(n,edges), "minimum weight mismatch");
            expect(r.components==1 && static_cast<int>(r.chosen.size())==n-1,
                   "tree size mismatch");
            Graph selected(n);
            for (const auto& e:r.chosen) {
                expect(!reachable(selected,e.u,e.v), "selected cycle");
                selected[e.u].push_back(e.v); selected[e.v].push_back(e.u);
            }
            for (int v=0; v<n; ++v)
                expect(reachable(selected,0,v), "selected edges do not span");
        } });
    std::cout << "Tests passed: " << passed << ", failed: " << failed << '\n';
    if (failed)
        throw std::runtime_error("Test suite failed");
}

int main(int argc, char *argv[])
{
    try
    {
        std::cout << std::boolalpha;
        const std::string mode = argc > 1 ? argv[1] : "--all";
        if (argc > 2)
            throw std::invalid_argument("Use one mode at a time");
        if (mode == "--help")
        {
            std::cout << "Modes: --basic --dsu --kruskal --tests --all\n";
        }
        else if (mode == "--all")
        {
            basicDemo();
            std::cout << '\n';
            dsuDemo();
            std::cout << '\n';
            kruskalDemo();
            std::cout << "\nTESTS\n";
            runTests();
        }
        else if (mode == "--basic")
            basicDemo();
        else if (mode == "--dsu")
            dsuDemo();
        else if (mode == "--kruskal")
            kruskalDemo();
        else if (mode == "--tests")
            runTests();
        else
            throw std::invalid_argument("Unknown mode; use --help");
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "ERROR: " << e.what() << '\n';
        return 1;
    }
}

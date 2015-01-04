#ifndef ASTAR
#define ASTAR

#include <queue>
#include <vector>
#include <functional>

namespace astar {

template <class T>
class SearchNode {
private:
    T state;
    unsigned pathCost;
    unsigned heuristic;
    mutable std::vector<T> cachedSuccessors;
public:
    SearchNode(const T& state, unsigned pathCost, unsigned heuristic) : state(state), pathCost(pathCost), heuristic(heuristic) {}
    SearchNode(T&& state, unsigned pathCost, unsigned heuristic) : state(std::move(state)), pathCost(pathCost), heuristic(heuristic) {}
    SearchNode(const SearchNode<T>& copy) : state(copy.state), pathCost(copy.pathCost), heuristic(copy.heuristic), cachedSuccessors(copy.cachedSuccessors) {}
    SearchNode(SearchNode<T>&& move) : state(std::move(move.state)), pathCost(move.pathCost), heuristic(move.heuristic), cachedSuccessors(std::move(move.cachedSuccessors)) {}
    ~SearchNode() {}

    inline const T& getState() const { return state; }
    inline unsigned getPathCost() const { return pathCost; }
    inline unsigned getHeuristic() const { return heuristic; }
    inline unsigned getFCost() const { return getPathCost() + getHeuristic(); }
    inline const std::vector<T>& getSuccessors() const {
        if(cachedSuccessors.empty()) {
            cachedSuccessors = getState().successors();
        }
        return cachedSuccessors;
    }
};

template <class T>
inline bool nodeComparator(const SearchNode<T>& lhs, const SearchNode<T>& rhs) {
    return lhs.getFCost() < rhs.getFCost();
}

template <class T, class H>
class AStar {
private:
    std::priority_queue<SearchNode<T>, std::vector<SearchNode<T>>, std::function<bool(const SearchNode<T>&,const SearchNode<T>&)>> queue;
    H heuristic;
public:
    AStar(const T& initialState, const H& heuristic) : queue(&nodeComparator<T>), heuristic(heuristic) {
        queue.emplace(initialState, 0, heuristic(initialState));
    }
    AStar(T&& initialState, const H& heuristic) : queue(&nodeComparator<T>), heuristic(heuristic) {
        queue.emplace(std::move(initialState), 0, heuristic(initialState));
    }
    SearchNode<T> step() {
        if(queue.empty()) {
            throw std::runtime_error("There are no more states to search!");
        }
        SearchNode<T> next = queue.top();
        queue.pop();
        for(T successor : next.getSuccessors()) {
            queue.emplace(successor, next.getPathCost() + 1, heuristic(successor));
        }
        return next;
    }
    SearchNode<T> solve() {
        for(;;) {
            SearchNode<T> best = step();
            if(!best || best.getSuccessors().empty() || queue.empty()) {
                return best;
            }
        }
    }
};

}

#endif /* #ifndef ASTAR */

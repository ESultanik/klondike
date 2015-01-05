#ifndef ASTAR
#define ASTAR

#include <queue>
#include <vector>
#include <functional>
#include <unordered_set>

namespace astar {

template <class T>
class SearchNode {
private:
    const T* state;
    unsigned pathCost;
    unsigned heuristic;
    mutable std::vector<T> cachedSuccessors;
public:
    SearchNode() : state(nullptr), pathCost(0), heuristic(0) {}
    SearchNode(const T& state, unsigned pathCost, unsigned heuristic) : state(&state), pathCost(pathCost), heuristic(heuristic) {}
    SearchNode(const SearchNode<T>& copy) : state(copy.state), pathCost(copy.pathCost), heuristic(copy.heuristic), cachedSuccessors(copy.cachedSuccessors) {}
    SearchNode(SearchNode<T>&& move) : state(move.state), pathCost(move.pathCost), heuristic(move.heuristic), cachedSuccessors(std::move(move.cachedSuccessors)) {}
    ~SearchNode() {}

    SearchNode& operator=(const SearchNode<T>& copy) {
        state = copy.state;
        pathCost = copy.pathCost;
        heuristic = copy.heuristic;
        cachedSuccessors = copy.cachedSuccessors;
        return *this;
    }
    SearchNode& operator=(SearchNode<T>&& move) {
        state = move.state;
        pathCost = move.pathCost;
        heuristic = move.heuristic;
        cachedSuccessors = std::move(move.cachedSuccessors);
        return *this;
    }

    inline operator bool() const { return state != nullptr; }

    inline const T& getState() const { return *state; }
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
    return lhs.getFCost() > rhs.getFCost();
}

template <class T, class H>
class AStar {
private:
    typedef std::priority_queue<SearchNode<T>, std::vector<SearchNode<T>>, std::function<bool(const SearchNode<T>&,const SearchNode<T>&)>> QueueType;
    QueueType queue;
    H heuristic;
    std::unordered_set<T> history;
    size_t nodesExpanded;
    unsigned depthLimit;
public:
    typedef decltype(((T*)nullptr)->getLastMove()) MoveType;
    AStar(const T& initialState, const H& heuristic, unsigned depthLimit = 0) : queue(&nodeComparator<T>), heuristic(heuristic), nodesExpanded(0), depthLimit(depthLimit) {
        const T& h = *history.insert(initialState).first;
        queue.emplace(h, 0, heuristic(initialState));
    }
    AStar(T&& initialState, const H& heuristic, unsigned depthLimit = 0) : queue(&nodeComparator<T>), heuristic(heuristic), nodesExpanded(0), depthLimit(depthLimit) {
        const T& h = *history.insert(initialState).first;
        queue.emplace(h, 0, heuristic(initialState));
    }
    SearchNode<T> step() {
        if(queue.empty()) {
            throw std::runtime_error("There are no more states to search!");
        }
        SearchNode<T> next = queue.top();
	if(nodesExpanded++ % 10000 == 0) {
            std::cout << "Searching: " << next.getFCost() << "\t" << queue.size() << std::endl;// << next.getState() << std::endl;
        }
        queue.pop();
        for(const T& successor : next.getSuccessors()) {
            if(history.find(successor) == history.end()) {
                const T& h = *history.insert(successor).first;
                queue.emplace(h, next.getPathCost() + 1, heuristic(successor));
            }
        }
        return next;
    }
    SearchNode<T> solve() {
        for(;;) {
            SearchNode<T> best = step();
            if(best.getSuccessors().empty() || queue.empty()) {
                return best;
            }
        }
    }
    std::pair<MoveType,SearchNode<T>> getBestMove() {
        if(queue.empty()) {
            throw std::runtime_error("There are no more states to search!");
        } else if(queue.size() != 1) {
            throw std::runtime_error("Error: getBestMove() is only expected to be called when there is exactly one search node in the queue!");
        }
        SearchNode<T> currentState = queue.top();
        queue.pop();
        if(currentState.getSuccessors().empty()) {
            throw std::runtime_error("The current state has no legal moves!");
        }
        MoveType* best = nullptr;
        SearchNode<T>* bestResult = nullptr;
        for(const T& successor : currentState.getSuccessors()) {
            const T& h = *history.insert(successor).first;
            queue.emplace(h, currentState.getPathCost() + 1, heuristic(successor));
            for(;;) {
                SearchNode<T> result = step();
                if(result.getSuccessors().empty() || queue.empty()) {
                    if(bestResult == nullptr || bestResult->getFCost() > result.getFCost()) {
                        delete best;
                        delete bestResult;
                        best = new MoveType(successor.getLastMove());
                        bestResult = new SearchNode<T>(std::move(result));
                    }
                    break;
                }
            }
            queue = QueueType(&nodeComparator<T>);
            history.clear();
            history.insert(currentState.getState());
        }
        auto ret = std::make_pair(std::move(*best), std::move(*bestResult));
        delete best;
        delete bestResult;
        return ret;
    }
};

}

#endif /* #ifndef ASTAR */

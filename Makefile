.PHONY : all
all : klondike.dbg klondike

klondike.dbg : klondike.cpp astar.h
	g++ --std=c++11 -Wall -Wextra -g $< -o $@

klondike : klondike.cpp astar.h
	g++ --std=c++11 -Wall -Wextra -DNDEBUG -O3 $< -o $@

.PHONY : clean
clean :
	rm -f klondike klondike.dbg

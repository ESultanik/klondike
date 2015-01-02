.PHONY : all
all : klondike.dbg klondike

klondike.dbg : klondike.cpp
	g++ --std=c++11 -Wall -Wextra -g $< -o $@

klondike : klondike.cpp
	g++ --std=c++11 -Wall -Wextra -O3 $< -o $@

.PHONY : clean
clean :
	rm -f klondike klondike.dbg

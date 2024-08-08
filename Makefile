vim_executable: vim.cpp terminal.cpp state.cpp util.cpp
	g++ *.cpp -o vim_executable -Wall -Wextra -pedantic

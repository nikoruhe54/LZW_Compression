Nikolai Ruhe
03/16/2018
Dr. Duan
Algorithms Class
Project 2

README

To run project 2 part 1:

1) open linux terminal
2) extract this project
3) build the executable by running the following command:

g++ -std=c++11 lzw435.cpp -o lzw435

4) next run the following commands to test part 1:

./lzw435 c [file_to_be_compressed]
./lzw435 e [file_to_be_compressed.lzw]

5) check that the the file was the same after it was expanded back to original

diff [file_to_be_compressed] [file_to_be_compressed2]

To run project 2 part 2:

1) build the executable by running the following command:

g++ -std=c++11 lzw435M.cpp -o lzw435M

2) next run the following commands to test part 2:

./lzw435M c [file_to_be_compressed]
./lzw435M e [file_to_be_compressed.lzwM]

5) check that the the file was the same after it was expanded back to original

diff [file_to_be_compressed] [file_to_be_compressed2M.lzwM]

Thank you!
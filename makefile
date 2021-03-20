all:main


main:main.cpp
	g++ main.cpp -o main -g -Ofast
clean:
	rm main
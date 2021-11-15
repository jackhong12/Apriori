SAN := -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment
CXXFLAG := -g3 -O3
#CXXFLAG += $(SAN)

apriori.exe: apriori.o
	g++ -std=c++11 -o apriori.exe apriori.o $(CXXFLAG)

apriori.o: apriori.cpp
	g++ -std=c++11 -c -o apriori.o apriori.cpp $(CXXFLAG)

vprofiling:
	valgrind --tool=callgrind ./apriori.exe 3 ./input.txt ./output.txt
	callgrind_annotate callgrind.out.* apriori.cpp

clean:
	rm -f apriori.o apriori.exe
	rm -f callgrind.out.*

FLAGS = -g -Wall -Werror -std=c++14
RUNNAME = predictors
.PHONY: predictors clean run

predictors: BranchPrediction.o
	g++ $(FLAGS) BranchPrediction.o -o predictors

BranchPrediction.o: BranchPrediction.cpp
	g++ -c $(FLAGS) BranchPrediction.cpp -o BranchPrediction.o

run: predictors
	./$(RUNNAME)

clean:
	rm *.o $(RUNNAME)

memcheck: predictors
	valgrind --leak-check=yes ./$(RUNNAME)

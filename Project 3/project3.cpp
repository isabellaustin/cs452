//CS452 Project 3: Clone Wars
//Written by Anna Vadella, Noah Baker, Isabell Austin

//Compile: mpicxx -o blah project3.cpp
//Run: mpirun -np 4 blah

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mpi.h"
using namespace std;

//Project Function Declarations
void smergesort(int *, int, int, int *);
void smerge(int *, int *, int, int, int *);

void pmergesort(int *, int, int, int *);
void pmerge(int *, int *, int, int, int *);

int Rank(int *, int, int, int);

//Helper Function Declarations
void printArray(int *, int);
bool isUnique(int *, int, int);

//Variable Declaration (Global)
int my_rank;							//My CPU number for this process (process ID number)
int p; 									//Number of CPUs that we have
int * output; 							//Output array

//Recursive function
void smergesort(int * a, int first, int last, int * output = NULL) {
	if (last - first < 1)
        return;

    int middle = (first + last) / 2;
    smergesort(a, first, middle, output);
    smergesort(a, middle + 1, last, output);

    smerge(&a[first], &a[middle + 1], middle - first, last - (middle + 1), &output[first]);

    for (int i = first; i <= last; i++)
        a[i] = output[i];
}

//Sequential merge function
void smerge(int * a, int * b, int lasta, int lastb, int * output = NULL) {
	int i = 0;
    int j = 0;
    int k = 0;

    while (i <= lasta && j <= lastb) {
        if (a[i] < b[j])
            output[k++] = a[i++];
        else
            output[k++] = b[j++];
    }

    while (i <= lasta)
        output[k++] = a[i++];
    while (j <= lastb)
        output[k++] = b[j++];
}

//Rank = "how many elements are smaller than me"
//Return value = number of items that are less than the value valToFind
int Rank(int * a, int first, int last, int valToFind) {
	//First and last variables indicate the range of the array you're working on
	//Base Case
	if(first == last) {
		if(valToFind < a[first])
			return 0;
		else
			return 1;
	}

	//Divide the array in half
	int middle = (first + last)/2;

	//Check if valToFind is on the left or the right side of the middle of the array
	if(valToFind < a[middle + 1])
		return Rank(a, first, middle, valToFind);
	else
		//return rank(a, middle + 1, last, valToFind);
		return ((last - first + 1)/2) + Rank(a, middle + 1, last, valToFind);
			//(last - first + 1)/2 --> size of the array divided by 2
}

//Parallel merge function
void pmerge(int * a, int * b, int lasta, int lastb, int * output = NULL) {
	//Phase 1: Calculate SRANKA and SRANKB by striping the work of computing ranks among all processors

	//Phase 2: Share all the SRANKA and SRANKB so all processors have the same answers

	//Phase 3: Make sure the answers from phase 2 are correct, test throughly!

	//Phase 4: Put sampled rank elements in correct place in an array that will have the correct solution

	//Phase 5: Figure out how to determine a "shape" according to the algorithm

	//Phase 6: Each process puts their smerged shapes in the right place, share all dhapes among everyone

}

//Helper function to print array
void printArray(int * a, int size) {
	for(int i = 0; i < size; i++) {
		cout << a[i] << " ";
	}
	cout << endl;
}

//Helper function to check if array entries are unique or not (avoiding duplicates)
bool isUnique(int * a, int b, int entry) {
	for(int i = 0; i < b; i++)
		if(a[i] == entry)					//Entry already exists, not unique
			return false;

	a[b] = entry;							//Entry does not already exist, unique
	return true;
}

int main (int argc, char * argv[]) {

	//Variable Declaration (Local)
	int source;							//Rank of the sender
	int dest;							//Rank of destination
	int tag = 0;						//Message number (ID number for a given message in a chain)
	MPI_Status status;					//Return status for receive

	//Start MPI
	MPI_Init(&argc, &argv);

	//Find out my rank
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	//Find out the number of processes
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	//Define parameter a based on a user inputted size
	int arraySize = 0;

	if(my_rank == 0) {
		cout << "Enter the array size: ";
		cin >> arraySize;
		//cout << "The array size you entered is: " << arraySize << endl;   //Tester print to check if user-inputted size is working
	}

	//Fill array with random numbers between 0 and 500, no duplicate entries!
	srand(time(0));
	int * userArray = new int[arraySize];
	int * outputArray = new int[arraySize];

	if(my_rank == 0) {
		for(int i = 0; i < arraySize; i++) {
            int arrayEntry;
            while(isUnique(userArray, i, arrayEntry) != true)
                arrayEntry = 1 + (rand() % 500);
		}

		cout << "Unsorted Array: " << endl;
		printArray(userArray, arraySize);
	}

	//Broadcast unsorted array from process 0 to every other process
	MPI_Bcast(&userArray[0], arraySize, MPI_INT, 0, MPI_COMM_WORLD);

	//Merging process happens here
	smergesort(userArray, 0, arraySize - 1, outputArray);

	if(my_rank == 0) {
		cout << "Sorted Array: " << endl;
		printArray(userArray, arraySize);
	}

	//Deleting dynamically allocated arrays
	delete [] userArray;
	delete [] outputArray;

	//Shut down MPI
	MPI_Finalize();

	return 0;
}
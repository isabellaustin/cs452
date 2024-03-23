//CS452 Project 3: Clone Wars
//Written by Anna Vadella, Noah Baker, Izzy Austin

//Compile: mpicxx -o blah project3.cpp
//Run: mpirun -np 4 blah

#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mpi.h"
using namespace std;

//Project Function Declarations
void smergesort(int *, int, int, int *);
void pmergesort(int *, int, int, int *);
int Rank(int *, int, int, int);						//Needs to be Rank() instead of rank() b/c C++ has preexisting rank struct
void smerge(int *, int *, int, int, int *);
void pmerge(int *, int *, int, int, int *); 

//Helper Function Declarations
void printArray(int *, int);
bool isUnique(int *, int, int);
void striping(int *, int *, int, int, int, int, int, int);

//Variable Declaration (Global)
int my_rank;							//My CPU number for this process (process ID number)
int p; 									//Number of CPUs that we have
int * output; 							//Output array

//Recursive function
void smergesort(int * a, int first, int last, int * output = NULL) 
{
	//Base Case
	if(last - first < 1)
		return;
	
	//Divides the array in half
	int middle = (first + last)/2;
	
	smergesort(a, first, middle, output);						//First half
	smergesort(a, middle + 1, last, output);					//Second half
	
	//Combine both halves together
	smerge(&a[first], &a[middle + 1], middle - first, last - (middle + 1), &output[first]);	

	//Copy results from array a to output
    for (int i = first; i <= last; i++)
        a[i] = output[i];
} 

void pmergesort(int * a, int first, int last, int * output = NULL) 
{
	//Base Case
	if(last - first < 1)
		return;
	
	//Divides the array in half
	int middle = (first + last)/2;
	
	pmergesort(a, first, middle, output);						//First half
	pmergesort(a, middle + 1, last, output);					//Second half
	
	//Combine both halves together
	pmerge(&a[first], &a[middle + 1], middle - first, last - (middle + 1), &output[first]);	

	//Copy results from array a to output
    for (int i = first; i <= last; i++)
        a[i] = output[i];
} 

//Rank = "how many elements are smaller than me"
//Return value = number of items that are less than the value valToFind
int Rank(int * a, int first, int last, int valToFind)
{
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

//Sequential merge function
void smerge(int * a, int * b, int lasta, int lastb, int * output = NULL)
{
	int i = 0; 
    int j = 0;
    int k = 0;
	
    while(i <= lasta && j <= lastb) {
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

//Parallel merge function
//Arrays are already sorted before being used in pmerge
void pmerge(int * a, int * b, int lasta, int lastb, int * output = NULL) 
{
	//Phase 1: Calculate SRANKA and SRANKB by striping the work of computing ranks among all processors
	//Calculating SRANK array size and segment sizes 
	int localStart = my_rank;
	int segmentSize = (lastb - a[0] + 1)/2; 						//should in theory be doing (last - first + 1)/2
	int logSegmentSize = log2(segmentSize);
	int localSize = ceil(((double)segSize/(double)logSegSize));		//ceil() function: returns smallest possible int value which is greater than or equal to given argument 
	
	//Tester print statement to test localSize 
	cout << "This is the value of localSize " << localSize << endl;
	
	//Initialize SRANK arrays
	int * srankA = new int[localSize];
	int * srankB = new int[localSize];
	int * totalsrankA = new int[localSize];
	int * totalsrankB = new int[localSize];
	
	//Should have something here which sets/initializes the array values of srankA and srankB to 0 (helper function?)
	
	//Striping process happens here
	//Might need to modify this potentially, not using array b whatsoever currently 
	striping(srankA, a, lasta + 1, lastb, first, localStart, localSize, logSegmentSize);
	striping(srankB, a, first, lasta, lasta + 1, localStart, localSize, logSegmentSize);
	
	//Phase 2: Share all the SRANKA and SRANKB so all processors have the same answers (sharing all SRANK arrays)
	MPI_Allreduce(&srankA[0], &totalsrankA[0], localSize, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(&srankB[0], &totalsrankB[0], localSize, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	
	//Tester print statement to see if all ranks/processors reduced correctly
	cout << my_rank << " finished reducing." << endl;
	
	//Phase 3: Make sure the answers from phase 2 are correct, test throughly!
	
	//Phase 4: Put sampled rank elements in correct place in an array that will have the correct solution
	int size = lastb + 1; 
	int * WIN = new int[size];
	int * localWIN = new int[size];
	
	//Should have something here which sets/initializes the array values of WIN to 0 (helper function?)
	//Definitely think something else is missing here, not really sure what...
	
	//Phase 5: Figure out how to determine a "shape" according to the algorithm (will smerge here!)
	int * shapesA = new int[2 * localSize + 1];
	int * shapesB = new int[2 * localSize + 1];
	
	//Set "shapes" with segment sizes and ranks - Definitely need to revise/test this like a lot 
	for(int i = 0; i < localSize; i++) {						//ShapesA
		shapesA[i] =  i * logSegmentSize;
		shapesA[i + localSize] = totalsrankA[i];
	} 
	
	for(int i = 0; i < localSize; i++) {						//ShapesB
		shapesB[i] =  i * logSegmentSize;
		shapesB[i + localSize] = totalsrankB[i];
	}
	
	shapesA[2 * localSize + 1] = (lastb - a[0] + 1)/2; 
	shapesB[2 * localSize + 1] = (lastb - a[0] + 1)/2; 
	
	//Use smerge to sort ranks 
	smerge(shapesA, 0, localSize - 1, localSize, 2 * localSize);
	smerge(shapesB, 0, localSize - 1, localSize, 2 * localSize);
	
	//Use smerge to determine a "shape" - Definitely need to revise/test this like a lot 
	for(int i = localStart; i < (2 * localSize); i += p) 
		smerge(a, &a[first] + subproblemA[i], &a[first] + subproblemA[i + 1] - 1, (lasta + 1) + subproblemB[i], (lasta + 1) + subproblemB[i + 1] - 1, localWin, lasta + 1, lastb);
	
	//Phase 6: Each process puts their smerged shapes in the right place, share all shapes among everyone 
	MPI_Allreduce(&localWin[0], &win[0], size, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	
	//Copy the values from the WIN array to the output array - Can use a helper function or do the below???
	 for (int i = 0; i < size; i++) {
        WIN[i] = output[i];
    }
	
	//Testing out whether the output array has the correct values or not
	if(my_rank == 0) {
		cout << "Output pmerge array: ";
		printArray(output, size);
		cout << endl;
	} 
	
	//Deleting dynamically allocated arrays	
	delete [] srankA;
	delete [] srankB;
	delete [] totalsrankA;
	delete [] totalsrankB;
	delete [] WIN;
	delete [] localWIN;
} 

//Helper function to print array
void printArray(int * a, int size) 
{
	for(int i = 0; i < size; i++) {
		cout << a[i] << " "; 
	}
	cout << endl;
} 

//Helper function to check if array entries are unique or not (avoiding duplicates)
bool isUnique(int * a, int b, int entry) 
{
	for(int i = 0; i < b; i++)
		if(a[i] == entry)					//Entry already exists, not unique
			return false;
		
	a[b] = entry;							//Entry does not already exist, unique 
	return true;
} 

//Helper function for striping
void striping(int * sRank, int * a, int first, int last, int position, int localStart, int localSize, int logSegmentSize)
{
	//Adding p each time because that's the size of the "jump" needed to calculate each processor's sections/segments
    for (int i = localStart; i < localSize; i += p)								//Because p is a global var, should not have a problem
        sRank[i] = Rank(a, first, last, a[position + (i * logSegmentSize)]);
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
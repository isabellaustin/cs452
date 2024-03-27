#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mpi.h"
using namespace std;

// Project Function Declarations
void smergesort(int *, int, int, int *);
void pmergesort(int *, int, int, int *);
int Rank(int *, int, int, int);
void smerge(int *, int *, int, int, int *);
void pmerge(int *, int *, int, int, int *);

// Helper Function Declarations
void printArray(int *, int);
bool isUnique(int *, int, int);

// Variable Declaration (Global)
int my_rank;
int p;
int *output;


void smergesort(int *a, int first, int last, int *output = NULL) {
    if (last - first < 1)
        return;

    int middle = (first + last) / 2;

    smergesort(a, first, middle, output);
    smergesort(a, middle + 1, last, output);

    smerge(&a[first], &a[middle + 1], middle - first, last - (middle + 1), &output[first]);

    for (int i = first; i <= last; i++)
        a[i] = output[i];
}

void pmergesort(int *a, int first, int last, int *output = NULL) {
    if (last - first < 1)
        return;

    int middle = (first + last) / 2;

    pmergesort(a, first, middle, output);
    pmergesort(a, middle + 1, last, output);

    pmerge(&a[first], &a[middle + 1], middle - first, last - (middle + 1), &output[first]);

    for (int i = first; i <= last; i++)
        a[i] = output[i];
}

int Rank(int *a, int first, int last, int valToFind) {
    if (valToFind > a[last - 1]) {
        return last;
    }

    if (first == last) {
        if (valToFind <= a[0])
            return 0;
        else
            return 1;
    }

    int middle = (first + last) / 2;

    if (valToFind < a[middle + 1])
        return Rank(&a[first], first, middle, valToFind);
    else
        return middle + Rank(&a[middle], 0, middle, valToFind);
}

void smerge(int *a, int *b, int lasta, int lastb, int *output = NULL) {
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

void pmerge(int *a, int *b, int lasta, int lastb, int *output = NULL) {
    //Phase 1: Calculate SRANKA and SRANKB by striping the work of computing ranks among all processors
	//Calculating SRANK array size and segment sizes 


	//we need to have base case if size is 1 or 0 or the log or partition is 0	
	int local_start = my_rank; 
	int logHalf = log2((lasta + lastb + 1)/2); //log half for index just A/B
    int logn = log2(lasta+lastb+1); //log n for index of entire array
    int partition = ceil((double)(lastb + 1) / (logn)); //n over log n for the size of arrays

	//cout << "Partition: " << partition << endl;

    //cout << "last a / last b: " << lasta << " / " << lastb  << endl;

	//when its first run the size of the array is 1, then logSegmentsize is now 0 based on how log works
	//then in local size we get math that is divided by zero which is bad but I think for some reason doesnt 
	//immeditely creash cuz of ceil(), so we end up with partition of -2147483648

	//Phase 3: Make sure the answers from phase 2 are correct, test throughly!*/
	
	//THIS IS WHERE ENDPOINT STUFF IS HAPPENING
	int size = lasta + lastb + 1; 
	int * WIN = new int[size];
	int * localWIN = new int[size];
	
	int * endpointsA = new int[(partition)]; // logn + 1
    int * endpointsB = new int[(partition)];
	int * localendpointsA = new int[(partition)];
	int * localendpointsB = new int[(partition)];
	
	for(int i = 0; i < partition; i++)
	{   
        localendpointsA[i] = 0;
        localendpointsB[i] = 0;
		endpointsA[i] = 0;
		endpointsB[i] = 0;
        WIN[i] = 0;
        localWIN[i] = 0;
	}
	
	for (int i = my_rank; i < partition; i += p) {
        int first = 0;    // first will always be 0
        //int last = (lasta + lastb + 1)/2;  ask if this is necesarry or if use lasta/b
        localendpointsA[i] = Rank(b, first, lasta, a[i * logHalf]); //SRANKA
        localendpointsA[i + partition] = i * logHalf;
        localendpointsB[i] = Rank(a, first, lastb, b[i * logHalf]); //SRANKB
        localendpointsB[i + partition] = i * logHalf;
        cout << my_rank << " endpoint calculation" << endl; 
    }
	
	//MPI_AllReduce(void* send_data, void* recv_data, int count, MPI_Datatype datatype, MPI_Op op, MPI_Comm communicator)

    MPI_Allreduce(localendpointsA, endpointsA, partition, MPI_INT, MPI_SUM, MPI_COMM_WORLD); 

    MPI_Allreduce(localendpointsB, endpointsB, partition, MPI_INT, MPI_SUM, MPI_COMM_WORLD); 

    MPI_Allreduce(localWIN, WIN, size, MPI_INT, MPI_SUM, MPI_COMM_WORLD);   
    
	cout << my_rank << " Reduce complete." << endl;	

    // I could be wrong but I dont think this is neccesarry
    // endpointsA[partition] = size/2;
    // endpointsB[partition ] = size/2; 

    cout << "This is a partition: " << partition << endl;

    //test the endpoints
    if(my_rank == 0) {
        cout << "endpointsA: ";
        printArray(endpointsA, partition);
        cout << "endpointsB: ";
        printArray(endpointsB, partition);
        cout << endl;
    }
   // if (my_rank==0) {
   //     cout << "SRANK + ENDPOINTS: " << endl;
   // }
   // testEndpoints(my_rank, endpointsA, endpointsB, srankSize);

    //endpoints must be sorted to define the shapes
    /*smergesort(endpointsA, 0, partition*2, localWIN);
    smergesort(endpointsB, 0, partition*2, localWIN);

    cout << "smergesort of endpoints" << endl;

    //test the sorted endpoints
   // if (my_rank ==0) {
    //    cout << "SORTED ENDPOINTS: " << endl;
   // }
   // testEndpoints(my_rank, endpointsA, endpointsB, srankSize);

    //CREATE THE SHAPES
    //using sequential merge
    for(int i = my_rank; i < partition * 2; i+=p) {
        smerge(&endpointsA[0], &endpointsB[0], endpointsA[i+1], endpointsB[i+1], WIN);
    }
    cout << "smerge of shapes" << endl;


    //collect all shapes into the win array
    MPI_Allreduce(localWIN, WIN, size, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    for (int i = 0; i < size; i++) {
        output[i] = WIN[i];
    }

    cout << "allReduce and win" << endl;*/
	
	//Use smerge to determine a "shape" - Definitely need to revise/test this like a lot 
	cout << "error when smerging second: " <<  my_rank << endl; 
	for(int i = local_start; i < (2 * partition); i += p) 
		// smerge(a, &a[first] + subproblemA[i], &a[first] + subproblemA[i + 1] - 1, (lasta + 1) + subproblemB[i], (lasta + 1) + subproblemB[i + 1] - 1, localWin, lasta + 1, lastb);4
			//subproblem = shapes
		//smerge(shapesA, shapesB, a[first] + shapesA[i + 1] - 1, (lasta + 1) + shapesB[i + 1] - 1, localWIN);
	
 	cout << "error before all reduce : " <<  my_rank << endl; 
	//Phase 6: Each process puts their smerged shapes in the right place, share all shapes among everyone 
	MPI_Allreduce(&localWIN[0], &WIN[0], size, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

	cout << "error when making Win output: " <<  my_rank << endl; 
	//Copy the values from the WIN array to the output array - Can use a helper function or do the below???
	 for (int i = 0; i < size; i++) {
        output[i] = WIN[i];
    }
	
	//Testing out whether the output array has the correct values or not
	if(my_rank == 0) {
		cout << "Output pmerge array: ";
		printArray(output, size);
		cout << endl;
	} 

    cout << "print" << endl;
	
	//Deleting dynamically allocated arrays	
	delete [] WIN;
	delete [] localWIN;
    delete [] endpointsA;
    delete [] endpointsB;
    delete [] localendpointsA;
    delete [] localendpointsB;
}

void printArray(int *a, int size) {
    for (int i = 0; i < size; i++) {
        cout << a[i] << " ";
    }
    cout << endl;
}

bool isUnique(int *a, int b, int entry) {
    for (int i = 0; i < b; i++)
        if (a[i] == entry)
            return false;

    a[b] = entry;
    return true;
}

int main(int argc, char *argv[]) {
    int source;
    int dest;
    int tag = 0;
    MPI_Status status;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    int arraySize = 0;

    if (my_rank == 0) {
        cout << "Enter the array size: ";
        cin >> arraySize;
    }

    MPI_Bcast(&arraySize, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int *userArray = new int[arraySize];
    int *outputArray = new int[arraySize];

    if (my_rank == 0) {
        srand(1121);
        for (int i = 0; i < arraySize; i++) {
            int arrayEntry = 1 + (rand() % 500);
            while (!isUnique(userArray, i, arrayEntry))
                arrayEntry = 1 + (rand() % 500);
        }

        cout << "Unsorted Array:" << endl;
        printArray(userArray, arraySize);

        smergesort(userArray, 0, arraySize - 1, outputArray);
        cout << "Sorted Array" << endl;
        printArray(userArray, arraySize);
    }

    MPI_Bcast(userArray, arraySize, MPI_INT, 0, MPI_COMM_WORLD);
    pmerge(&userArray[0], &userArray[arraySize/2 + 1], arraySize/2, arraySize-1, outputArray); //may make lastb arraySize-1

    if (my_rank == 0) {
        cout << "Sorted Array:" << endl;
        printArray(userArray, arraySize);
    }

    delete[] userArray;
    delete[] outputArray;

    MPI_Finalize();

    return 0;
}

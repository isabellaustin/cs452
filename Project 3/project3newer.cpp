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
    if (last - first <= 32 )
        return;
    else if(last - first < 1){
        return;
        //pmerge(&a[first], &a[(last+1)/2], (last + 1)/2, last, &output[first]);
    }

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

    //partition should be 11 for 64
	int local_start = my_rank; 
    int size = lasta+lastb+1;
    //cout << "Size: " << size << endl;
	int logHalf = log2((size/2)); //log half for index just A/B
    int logn = ceil(log2(size)); //log n for index of entire array
    //cout << "Log of size: " << logn << endl;
    int partition = ceil((double)(size)/(logn)); //n over log n for the size of arrays

	cout << "Partition: " << partition << endl;

    //cout << "last a / last b: " << lasta << " / " << lastb  << endl;

	//Phase 3: Make sure the answers from phase 2 are correct, test throughly!*/
	
	//THIS IS WHERE ENDPOINT STUFF IS HAPPENING
	int * WIN = new int[size];
	int * localWIN = new int[size];
	
	int * endpointsA = new int[(partition * 2)];
    int * endpointsB = new int[(partition * 2)];
	int * srankA = new int[(partition * 2)];
	int * srankB = new int[(partition * 2)];
	
	for(int i = 0; i < partition; i++)
	{   
        //SRANK a and B change this
        srankA[i] = 0;
        srankB[i] = 0;
		endpointsA[i] = 0;
		endpointsB[i] = 0;
        WIN[i] = 0;
        localWIN[i] = 0;
	}
    
    for (int i = my_rank; i < partition; i += p) {
        srankB[i] = Rank(a, 0, lasta, b[i * logn]); //SRANKB
        srankA[i] = Rank(b, 0, lastb, a[i * logn]); //SRANKA
    }


    MPI_Allreduce(srankA, endpointsA, partition, MPI_INT, MPI_SUM, MPI_COMM_WORLD); 
    MPI_Allreduce(srankB, endpointsB, partition, MPI_INT, MPI_SUM, MPI_COMM_WORLD); 
    //MPI_Allreduce(localWIN, WIN, size, MPI_INT, MPI_SUM, MPI_COMM_WORLD);   

    cout << my_rank << " Reduce complete." << endl;	

    //this in the 64 example, endpoints should be 0,6,12,18,24...
    //Gupta says to use smaller example, change base case to reflect this, debug 
	for (int i = 0; i < partition; i++) {
        endpointsA[i + partition] = i * logn;
        endpointsB[i + partition] = i * logn;
    }
    cout << my_rank << " endpoint calculation" << endl; 
 
    // I could be wrong but I dont think this is neccesarry
    // endpointsA[partition] = size/2;
    // endpointsB[partition ] = size/2; 

    //test the endpoints
    if(my_rank == 0) {
        cout << "endpointsA: ";
        printArray(endpointsA, partition);
        cout << "endpointsB: ";
        printArray(endpointsB, partition);
        cout << endl;
    }

    //endpoints must be sorted to define the shapes
    smerge(&endpointsA[0], &endpointsA[logn], logn-1, partition-1, localWIN);
    smerge(&endpointsB[0], &endpointsB[logn], logn-1, partition-1, localWIN);

    if(my_rank == 0) {
        cout << "sorted endpointsA: ";
        printArray(endpointsA, partition);
        cout << "sorted endpointsB: ";
        printArray(endpointsB, partition);
        cout << endl;
    }

    //collect all shapes into the win array
    MPI_Allreduce(localWIN, WIN, size, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    for (int i = 0; i < size; i++) {
        output[i] = WIN[i];
    }
	
	//Testing out whether the output array has the correct values or not
	if(my_rank == 0) {
		cout << "Output pmerge array: ";
		printArray(output, size);
		cout << endl;
	} 

	//Deleting dynamically allocated arrays	
	delete [] WIN;
	delete [] localWIN;
    delete [] endpointsA;
    delete [] endpointsB;
    delete [] srankA;
    delete [] srankB;
}

void printArray(int *a, int size) {
    for (int i = 0; i < size; i++)
        cout << a[i] << " ";
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

    int * userArray = new int[arraySize];
    int * outputArray = new int[arraySize];

    if (my_rank == 0) {
        srand(1121);
        for (int i = 0; i < arraySize; i++) {
            int arrayEntry = 1 + (rand() % 500);
            while (!isUnique(userArray, i, arrayEntry))
                arrayEntry = 1 + (rand() % 500);
        }

        cout << "Unsorted Array:" << endl;
        printArray(userArray, arraySize);

        smergesort(userArray, 0, arraySize/2, outputArray);
        smergesort(userArray, arraySize/2 + 1, arraySize, outputArray);

        cout << "Sorted Array 1st Half and second half" << endl;
        printArray(userArray, arraySize);
    }

    MPI_Bcast(userArray, arraySize, MPI_INT, 0, MPI_COMM_WORLD);
    pmergesort(userArray, 0, arraySize - 1, outputArray);

    if (my_rank == 0) {
        cout << "Sorted Array:" << endl;
        printArray(userArray, arraySize);
    }

    delete[] userArray;
    delete[] outputArray;

    MPI_Finalize();

    return 0;
}

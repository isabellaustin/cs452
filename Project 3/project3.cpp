//CS452 Project 3: Clone Wars
//Written by Anna Vadella, Noah Baker, Izzy Austin

//Compile: mpicxx -o blah project3.cpp
//Run: mpirun -np 4 blah
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <iomanip>
#include <cmath>
#include "mpi.h"

using namespace std;

void mergesort(int *, int, int, int *);
int Rank(int *, int, int, int);
void smerge(int *, int, int, int *, int, int, int, int, int *);
void pmerge(int *, int *, int, int, int *);

void printArray(int *, int);
bool isUnique(int *, int, int);
void quickSort(int *, int, int);
int partition(int a[], int, int);

int my_rank;
int p;

int Rank(int * a, int first, int last, int valToFind) {
    if (valToFind > a[last - 1]) {
		return last;
	}

    if(last == 1) {	
        if(valToFind <= a[0]) {
			return 0;
		}
        else { 
			return 1;
		}
    }

    int middle = last/2;
    if(valToFind <= a[middle - 1])
        return Rank(&a[first], first, middle, valToFind);
    else
        return middle + Rank(&a[middle], 0, middle, valToFind);
}

void smerge(int * a, int firsta, int lasta, int * b, int firstb, int lastb, int firstoutput, int lastoutput, int * output = NULL) 
{
    int i = firsta;
    int j = firstb;
    int k = firstoutput;
 
    while (i <= lasta && j <= lastb)
        if (a[i] < b[j])
            output[k++] = a[i++];
        else
            output[k++] = b[j++];

    while (i <= lasta)
        output[k++] = a[i++];
    while (j <= lastb)
        output[k++] = b[j++];
}

void pmerge(int * a, int * b, int lasta, int lastb, int * output = NULL) {

	int totalArraySize = lasta + lastb + 1;
    int logn = log2(totalArraySize/2); 
    int partition = ceil((double)(totalArraySize/2)/logn);				

    int * localWIN = new int[totalArraySize];
    int * localendpointsA = new int[partition * 2 + 1];
    int * localendpointsB = new int[partition * 2 + 1];

    int * WIN = new int[totalArraySize];
    int * endpointsA = new int [partition * 2 + 1];
    int * endpointsB = new int [partition * 2 + 1];
	
	for(int i = 0; i < partition * 2 + 1; i++) {   
		endpointsA[i] = 0;
		endpointsB[i] = 0;
        localendpointsA[i] = 0;
        localendpointsB[i] = 0;
	}
	
	for(int i = 0; i < totalArraySize; i++) {
		localWIN[i] = 0;
        WIN[i] = 0;
	} 
	
    for (int i = my_rank; i < partition; i += p) { //striping
        int first = 0;
        int last = totalArraySize/2;

        localendpointsA[i] = Rank(a, first, last, b[i * logn]);
        localendpointsA[i + partition] = i * logn;
        localendpointsB[i] = Rank(b, first, last, a[i * logn]);
        localendpointsB[i + partition] = i * logn;
    }

	MPI_Allreduce(localendpointsA, endpointsA, partition * 2, MPI_INT, MPI_SUM, MPI_COMM_WORLD);    
    MPI_Allreduce(localendpointsB, endpointsB, partition * 2, MPI_INT, MPI_SUM, MPI_COMM_WORLD); 
    MPI_Allreduce(localWIN, WIN, totalArraySize, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
	
    endpointsA[partition * 2] = totalArraySize/2;
    endpointsB[partition * 2] = totalArraySize/2; 

    //sort endpoints to define shapes
    quickSort(endpointsA, 0, partition * 2 - 1);
    quickSort(endpointsB, 0, partition * 2 - 1);

    for(int i = my_rank; i < partition * 2; i+= p) //striping
        smerge(a, endpointsA[i], endpointsA[i + 1] - 1, b, endpointsB[i], endpointsB[i + 1] - 1, endpointsA[i] + endpointsB[i], endpointsB[i + 1] + endpointsB[i + 1], localWIN);

    MPI_Allreduce(localWIN, WIN, totalArraySize, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    for (int i = 0; i < totalArraySize; i++) {
        a[i] = WIN[i];
    }

    if (my_rank == 0) {
        cout << "WIN: ";
        printArray(WIN, totalArraySize);
        cout << endl;
    }
}

void mergesort(int * a, int first, int last, int * output = NULL) {
    if (last <= 4)
        quickSort(a, first, last - 1);
    else {
        mergesort(a, first, last/2);
        mergesort(&a[last/2], first, last/2);
        pmerge(a, &a[last/2], first, last - 1, output);
    }
}

void printArray(int * a, int size) {
    for (int i = 0; i < size; i++) {
        cout << a[i] << " ";
    }
    cout << endl;
}

bool isUnique(int * a, int b, int entry) {
    for (int i = 0; i < b; i++)
        if (a[i] == entry)
            return false;

    a[b] = entry;
    return true;
}

int partition(int a[], int first, int last)
{
    int pivot = a[last];
    int i = (first - 1);

    for (int j = first; j <= last - 1; j++) {
        if (a[j] <= pivot) {
            i++;
            int temp = a[i];
            a[i] = a[j];
            a[j] = temp;
        }
    }

    int temp = a[i + 1];
    a[i + 1] = a[last];
    a[last] = temp;

    return (i + 1);
}

void quickSort(int a[], int first, int last) {
    if (first < last)
    {
        int partitionIndex = partition(a, first, last);
        quickSort(a, first, partitionIndex - 1);
        quickSort(a, partitionIndex + 1, last);
    }
}

int main (int argc, char * argv[]) {
    int source;                					//Rank of the sender
    int dest;                					//Rank of destination
    int tag = 0;            					//message number
    char message[100];        					//message itself
    MPI_Status status;        					//return status for receive

    //Start MPI
    MPI_Init(&argc, &argv);     				//sets up processor
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);    //finds the Rank and populates that variable
    MPI_Comm_size(MPI_COMM_WORLD, &p);			//find out the number of processes

    int arraySize = 0;

    if (my_rank == 0) {
        cout << "Enter the array size: ";
        cin >> arraySize;
    }

    MPI_Bcast(&arraySize, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int * userArray = new int[arraySize];
    int * a = &userArray[0];
    int * b = &userArray[arraySize/2];

    if (my_rank == 0) {
        srand(time(NULL));
        for (int i = 0; i < arraySize; i++) {
            int arrayEntry = 1 + (rand() % 500);
            while (!isUnique(userArray, i, arrayEntry))
                arrayEntry = 1 + (rand() % 500);
        }
    }
	
	if (my_rank == 0) {
		cout << "Unsorted Array: " << endl;
		printArray(userArray, arraySize);
	} 
    MPI_Bcast(userArray, arraySize, MPI_INT, 0, MPI_COMM_WORLD);

    mergesort(userArray, 0, arraySize);

	delete [] userArray;
	
    //Shut down MPI
    MPI_Finalize();

    return 0;
}
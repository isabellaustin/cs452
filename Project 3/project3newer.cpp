//CS452 Project 3: Clone Wars
//Written by Anna Vadella, Noah Baker, Izzy Austin

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

void pmergesort(int *a, int first, int last, int *output = NULL) 
{
    if (last - first <= 32)
    {
        return;
    }
    int middle = (first + last) / 2;

    pmergesort(a, first, middle, output);
    pmergesort(a, middle + 1, last, output);

    pmerge(&a[first], &a[middle + 1], middle - first, last - (middle + 1), &output[first]);

    for (int i = first; i <= last; i++)
        a[i] = output[i];
}

int Rank(int *a, int first, int last, int valToFind) {

    if (valToFind > a[last]) {
        return last;
    }

    if (first == last) {
        if (valToFind <= a[first]) {
            return 0;
        }
        else { return 1; }
    }  

    int middle = (first + last) / 2;

    if (valToFind < a[middle+1]){
        return Rank(a, first, middle, valToFind);
    }
    else{
        return Rank(a, middle+1, last, valToFind);
    }
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
    int totarraySize = lasta + lastb + 1;
    //cout << "Size: " << totarraySize << endl;
    int logn = ceil(log2(totarraySize/2)); //log n for index of entire array
    //cout << "Log of size: " << logn << endl;
    int numRank = ceil((double)(totarraySize/2)/(logn)); // logn*p
	//cout << "numRank: " << numRank << endl;


	int * WIN = new int[totarraySize+1];
	int * endpointsA = new int[(numRank * 2)+ 2];
    int * endpointsB = new int[(numRank * 2)+ 2];
    int * finalendpointsA = new int[totarraySize];
    int * finalendpointsB = new int[totarraySize];
    int * localWIN = new int[totarraySize+1];
	int * srankA = new int[(numRank * 2)+ 2];
	int * srankB = new int[(numRank * 2)+ 2];


	for(int i = 0; i <= (numRank * 2)+ 2 ; i++)
	{   
        srankA[i] = 0;
        srankB[i] = 0;
		endpointsA[i] = 0;
		endpointsB[i] = 0;
        finalendpointsA[i] = 0;
        finalendpointsB[i] = 0;
	}
	
	for(int i = 0; i < totarraySize+1; i++)
	{
		localWIN[i] = 0;
        WIN[i] = 0;
	} 

    for (int i = 0; i < numRank; i++) {
        srankA[i] = i * logn;
        srankB[i] = i * logn;
    }
    srankA[numRank] = totarraySize/2;
    srankB[numRank] = totarraySize/2;
    
    for (int i = my_rank; i < numRank; i += p) {
        srankA[i + (numRank + 1)] = Rank(b, 0, lastb, a[i * logn]); //SRANKB
        srankB[i + (numRank + 1)] = Rank(a, 0, lasta, b[i * logn]); //SRANKA
    }
    srankA[(numRank * 2) + 1] = Rank(b, 0, totarraySize/2, a[totarraySize/2]);
    srankB[(numRank * 2) + 1] = Rank(a, 0, totarraySize/2, b[totarraySize/2]);


    MPI_Allreduce(srankA, endpointsA, (numRank * 2) + 2, MPI_INT, MPI_MAX, MPI_COMM_WORLD); 
    MPI_Allreduce(srankB, endpointsB, (numRank * 2) + 2, MPI_INT, MPI_MAX, MPI_COMM_WORLD); 

    /*if(my_rank == 0) {
        cout << "endpointsA: ";
        printArray(endpointsA, (numRank * 2) + 2);
        cout << "endpointsB: ";
        printArray(endpointsB, (numRank * 2) + 2);
        cout << endl;
    }*/

    smerge(&endpointsA[0], &endpointsA[numRank+1], numRank, numRank, finalendpointsA);
    smerge(&endpointsB[0], &endpointsB[numRank+1], numRank, numRank, finalendpointsB);

    /*if(my_rank == 0) {
        cout << "sorted new endpointsA: ";
        printArray(finalendpointsA, (numRank * 2) + 2);
        cout << "sorted new endpointsB: ";
        printArray(finalendpointsB, (numRank * 2) + 2);
        cout << endl;
    }*/

    int distanceA = 0;
    int distanceB = 0;
    int totaldistance = 0;
    
    for (int i = my_rank; i < (numRank * 2) + 2; i += p) {
        distanceB = finalendpointsA[i + 1] - finalendpointsA[i];
        distanceA = finalendpointsB[i + 1] - finalendpointsB[i];
        totaldistance = finalendpointsA[i] + finalendpointsB[i];
        smerge(&a[finalendpointsB[i]], &b[finalendpointsA[i]], distanceA, distanceB, &localWIN[totaldistance]);    
    }

    /*if(my_rank == 0)
    {
        cout << my_rank << endl;
        cout<< "New values added to array" << endl;
        printArray(localWIN, totarraySize+1); 
    }
    else if(my_rank == 1)
    {
        cout << my_rank << endl;
        cout<< "New values added to array" << endl;
        printArray(localWIN, totarraySize+1); 
    }
    else if(my_rank == 2)
    {
        cout << my_rank << endl;
        cout<< "New values added to array" << endl;
        printArray(localWIN, totarraySize+1); 
    }
    else if(my_rank == 3)
    {
        cout << my_rank << endl;
        cout<< "New values added to array" << endl;
        printArray(localWIN, totarraySize+1); 
    }*/
	  
	MPI_Allreduce(&localWIN[0], &WIN[0], totarraySize+1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

	if(my_rank == 0) {
		cout << "Output pmerge array after all reduce: ";
		printArray(WIN, totarraySize+1);
		cout << endl;
	}
	
	//Deleting dynamically allocated arrays	
	delete [] WIN;
    delete [] endpointsA;
    delete [] endpointsB;
    delete [] srankA;
    delete [] srankB;
    delete [] finalendpointsA;
    delete [] finalendpointsB;
    delete [] localWIN;
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

    int * userArray = new int[arraySize];
    int * outputArray = new int[arraySize];

    if (my_rank == 0) {
        srand(time(0));
        for (int i = 0; i < arraySize; i++) {
            int arrayEntry =  1 + (rand() % 500); //had a plus 1
            while (!isUnique(userArray, i, arrayEntry))
                arrayEntry =  1 + (rand() % 500); //had a plus 1
        }

        cout << "Unsorted Array:" << endl;
        printArray(userArray, arraySize);

        smergesort(userArray, 0, (arraySize/2)-1, outputArray);
        smergesort(userArray, arraySize/2, arraySize-1, outputArray);
    }

    MPI_Bcast(userArray, arraySize, MPI_INT, 0, MPI_COMM_WORLD);
    pmergesort(userArray, 0, arraySize - 1, outputArray);

    delete[] userArray;
    delete[] outputArray;

    MPI_Finalize();

    return 0;
}

    //striping here get the sizes of the first 2, calculates distance, than determines the smerge stuff
    //based on that information, instead of basing this on permission
    //Gupta doesnt like partition. I think shapearraySize is also bad. 
    // Kill the shapes array, because it is clearly dumb.
    //shapes require 4 points to work with
    //smerge(&endpointsA[0], &endpointsA[partition], partition-1, (partition * 2) - 1, &shapes[0]);
    //smerge(&endpointsB[0], &endpointsB[partition], partition-1, (partition * 2) - 1, &shapes[partition*2]); 
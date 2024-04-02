//CS452 Project 3: Clone Wars
//Written by Anna Vadella, Noah Baker, Izzy Austin


//add the 

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
    {
        return;
    }
    else if(last - first < 1)
    {
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
    if (valToFind > a[last]) {
        return last;
    }

    if (first == last) {
        if (valToFind <= a[0])
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }  

    int middle = (first + last) / 2;

    if (valToFind < a[middle+1]){
        return Rank(a, first, middle, valToFind);
    }
    else{
        //used to be 0 and middle instead of middle and last
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
    //Phase 1: Calculate SRANKA and SRANKB by striping the work of computing ranks among all processors
	//Calculating SRANK array size and segment sizes 

	//we need to have base case if size is 1 or 0 or the log or partition is 0	
    //partition should be 11 for 64


    //Recalculate for one indexing 
	int local_start = my_rank; 
    int totarraySize = lasta + lastb + 1;
    //cout << "Size: " << totarraySize << endl;
    int logn = ceil(log2(totarraySize/2)); //log n for index of entire array
    //cout << "Log of size: " << logn << endl;
    int partition = ceil((double)(totarraySize/2)/(logn)); //n over log n for the size of arrays
    int shapearraySize = (partition * 2) * 2;
	//cout << "Partition: " << partition << endl;

    //cout << "LastA/B:   " <<  lasta << "/" << lastb << endl;

	//Phase 3: Make sure the answers from phase 2 are correct, test throughly!*/
	
	//Make the size of srank partition * 2 so you dont have to do extra shit 
	int * WIN = new int[totarraySize];
	int * endpointsA = new int[(partition * 2)+ 2];
    int * endpointsB = new int[(partition * 2)+ 2];
    int * finalendpointsA = new int[totarraySize];
    int * finalendpointsB = new int[totarraySize];
    int * localWIN = new int[totarraySize];
	int * srankA = new int[(partition * 2)+ 2];
	int * srankB = new int[(partition * 2)+ 2];


    //I think some of these are not correct so double check
	for(int i = 0; i <= (partition * 2)+ 2 ; i++)
	{   
        srankA[i] = 0;
        srankB[i] = 0;
		endpointsA[i] = 0;
		endpointsB[i] = 0;
        finalendpointsA[i] = 0;
        finalendpointsB[i] = 0;
	}
	
	for(int i = 0; i < totarraySize; i++)
	{
		localWIN[i] = 0;
        WIN[i] = 0;
	} 

    //add all of the logn values, not rank just yet
    for (int i = 0; i < partition; i++) {
        srankA[i] = i * logn;
        srankB[i] = i * logn;
    }
    srankA[partition] = totarraySize/2;
    srankB[partition] = totarraySize/2;
    
    for (int i = my_rank; i < partition; i += p) {
        srankA[i + (partition + 1)] = Rank(b, 0, lastb, a[i * logn]); //SRANKB
        srankB[i + (partition + 1)] = Rank(a, 0, lasta, b[i * logn]); //SRANKA
    }
    srankA[(partition * 2) + 1] = Rank(b, 0, totarraySize/2, a[totarraySize/2]);
    srankB[(partition * 2) + 1] = Rank(a, 0, totarraySize/2, b[totarraySize/2]);


    MPI_Allreduce(srankA, endpointsA, (partition * 2) + 2, MPI_INT, MPI_MAX, MPI_COMM_WORLD); 
    MPI_Allreduce(srankB, endpointsB, (partition * 2) + 2, MPI_INT, MPI_MAX, MPI_COMM_WORLD); 
	//MPI_MAX is a "slightly more accurate way to define what we are trying to do"

    //cout << my_rank << " endpoint calculation" << endl; 
    //test the endpoints
    if(my_rank == 0) {
        cout << "endpointsA: ";
        printArray(endpointsA, (partition * 2) + 2);
        cout << "endpointsB: ";
        printArray(endpointsB, (partition * 2) + 2);
        cout << endl;
    }

    smerge(&endpointsA[0], &endpointsA[partition+1], partition, partition, finalendpointsA);
    smerge(&endpointsB[0], &endpointsB[partition+1], partition, partition, finalendpointsB);

    if(my_rank == 0) {
        cout << "sorted new endpointsA: ";
        printArray(finalendpointsA, (partition * 2) + 2);
        cout << "sorted new endpointsB: ";
        printArray(finalendpointsB, (partition * 2) + 2);
        cout << endl;
    }

    int distanceA = 0;
    int distanceB = 0;
    int totalDistance = 0;

    for(int i = my_rank; i < (partition * 2) + 2; i+=p)
    {
        distanceA = finalendpointsA[i + 1] - finalendpointsA[i];
        cout << "distanceA: " << distanceA << ": " << finalendpointsA[i + 1] << ":" << finalendpointsA[i] << endl;
        distanceB = finalendpointsB[i + 1] - finalendpointsB[i];
        cout << "distanceB: " << distanceB << ": " << finalendpointsB[i + 1] << ":" << finalendpointsB[i] << endl;
		

        //in order to make sure that every sorted number is in the right spot for MPI_Reduce to combine everything together into a sorted array
        //should we make changed to smerge so that i, j, and k are parameters, and everything falls into the right spot
        //can we make a special shape smerge that does this, or am i over complicating things.
        smerge(&a[finalendpointsA[i]], &b[finalendpointsB[i]], distanceB, distanceA, &localWIN[totalDistance]);

        //this is some dumb stuff because zero indexing is the worst this will change
        if(distanceA == 0 && distanceB == 0)
        {
            totalDistance += 2;
        }

        totalDistance = totalDistance + distanceA + distanceB;
        cout <<  "total distance: " << totalDistance << endl;
        cout<< "New values added to array" << endl;
        printArray(localWIN, totarraySize-totalDistance); 
    }

	/*for(int i = my_rank; i < (partition * 2); i+= p)
    {
		cout << "my rank: " << my_rank << endl;
		
		distanceA = finalendpointsA[i + 1] - finalendpointsA[i];
        cout << "distanceA: " << distanceA << ": " << finalendpointsA[i + 1] << ":" << finalendpointsA[i] << endl;
        distanceB = finalendpointsB[i + 1] - finalendpointsB[i];
        cout << "distanceB: " << distanceB << ": " << finalendpointsB[i + 1] << ":" << finalendpointsB[i] << endl;
		
        smerge(&a[finalendpointsA[i]], &b[finalendpointsB[i]], distanceB, distanceA, &localWIN[totalDistance]);
        totalDistance = totalDistance + distanceA + distanceB;
        cout << totalDistance << endl;
        cout<< "New values added to array" << endl;
        printArray(localWIN, totalDistance); 
		
		//Might be in wrong spot for local win, off by one when creating shapes 
		//Gupta says just change it back to striping and trace it as a stripe
		//Print out the local endpoint error for each processor
    }*/

	  
	MPI_Allreduce(localWIN, WIN, totarraySize, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

	if(my_rank == 0) {
		cout << "Output pmerge array after all reduce: ";
		printArray(WIN, totarraySize);
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
        srand(1121);
        for (int i = 0; i < arraySize; i++) {
            int arrayEntry = 1 + (rand() % 500);
            while (!isUnique(userArray, i, arrayEntry))
                arrayEntry = 1 + (rand() % 500);
        }

        cout << "Unsorted Array:" << endl;
        printArray(userArray, arraySize);

        smergesort(userArray, 0, (arraySize/2)-1, outputArray);
        smergesort(userArray, arraySize/2, arraySize-1, outputArray);

        cout << "Sorted Array 1st Half" << endl;
        for(int i = 0; i < arraySize/2; i++) {
		    cout << userArray[i] << " "; 
	    }
	    cout << endl;

        cout << "Sorted Array 2st Half" << endl;
        for(int i = (arraySize/2); i < arraySize; i++) {
		    cout << userArray[i] << " "; 
	    }
	    cout << endl;
    }

    MPI_Bcast(userArray, arraySize, MPI_INT, 0, MPI_COMM_WORLD);
    pmergesort(userArray, 0, arraySize - 1, outputArray);

    /*if (my_rank == 0) {
        cout << "Sorted Array:" << endl;
        printArray(userArray, arraySize);
    }*/

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

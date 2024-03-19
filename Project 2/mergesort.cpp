#include <iostream>
#include <string.h>
using namespace std; 

void merge(int * a, int * b, int lasta, int lastb, int * output = NULL) 
{
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

void mergesort(int * a, int first, int last, int * output = NULL) 
{	
	if (last - first < 1)
        return;

    int partition = (first + last) / 2;

    mergesort(a, first, partition, output);
    mergesort(a, partition + 1, last, output);
    merge(&a[first], &a[partition + 1], partition - first, last - (partition + 1), &output[first]);

    //copy output to a
    for (int i = first; i <= last; i++)
        a[i] = output[i];
} 

int main()
{
	srand(time(0));
	
	int n;
	cout << "Enter the size of the array: ";
	cin >> n;	
	
	int * inputArray = new int[n];
	int * outputArray = new int[n];
	
	// fill in array with random numbers
	for(int i = 0; i < n; i++) {
		inputArray[i] = 1 + (rand() % 1000000); 
	}
	
	cout << "Unsorted Array: " << endl;
    for(int x = 0; x < n; x++) {
		cout << inputArray[x] << " "; 
	}
	cout << '\n' << endl;
	
	mergesort(inputArray, 0, n - 1, outputArray);
	
	cout << "Sorted Array: " << endl;
    for(int x = 0; x < n; x++) {
		cout << outputArray[x] << " "; 
	}
    cout << endl;
	
	delete [] inputArray;
	delete [] outputArray;
	
	return 0;
} 
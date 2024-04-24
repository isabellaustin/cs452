//CS452 Project 4: COVINS-20
//Written by Izzy Austin, Anna Vadella, and Noah Baker

//Compile: g++ treeMain.cpp
//Run: ./a.out

#include <stdio.h>
#include <stdlib.h>
#include "Tree.h"
using namespace std;

int main() 
{
    srand(1251);

    int rootValue = 1000;
    int insLeaves [30]; //insert
    int delLeaves [30]; //delete

    for(int i = 0; i < 30; i++) {
        insLeaves[i] = 1 + rand() % 999;		//Node values range from 1 to 999
        // if(i % 2 == 0)
        //     delLeaves[i] = insLeaves[i];
        // else
        //     delLeaves[i] = 1+rand()%999;
    }

	Node* root = new Node(rootValue);
    cout << "Root: " << rootValue << endl;
	Tree tree(root);
    cout << endl;
	
    tree.Print();
    
    //how many are we suppose to ins/del? random?
    for(int i = 0; i < 15; i++) {
        cout << "Node inserted: " << insLeaves[i] << endl;
        tree.Insert(insLeaves[i]);
        cout << endl;
        
        tree.Print();
        cout << endl;
    }

    for(int i = 0; i < 10; i++) {
        cout << "Node deleted: " << insLeaves[i] << endl;
        tree.Delete(delLeaves[i]);
        cout << endl;

        tree.Print();
        cout << endl;
    }

    delete root;

	return 0;
}
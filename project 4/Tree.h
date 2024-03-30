#include <iostream>
#include "Node.h"
using namespace std;

class Tree {
    Node * root;
    void print(Node * start);
    
    public:
        Tree(); // constructor
        Node * search(int valToFind);
        bool insert (int valToAdd); 
        bool delete (int valtokill);
        void print();
}
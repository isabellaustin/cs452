#include <iostream>
using namespace std;

class Node {
    Node * parent;
    int value [6];
    Node * child[6];

    public:
        Node (int val); // constructor 
        bool numChildren();
        void absorb(Nodw * newChild);
        void discard(Node * removeChild);        
}
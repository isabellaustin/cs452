//CS452 Project 4: Node Class
//Written by Izzy Austin, Anna Vadella, and Noah Baker

#include <iostream>
using namespace std;

//From Project Description
/*class Node {
    Node * parent;
    int currentVal [6];
    Node * child[6];

    public:
        Node (int val); // constructor 
        bool numChildren();
        void absorb(Nodw * newChild);
        void discard(Node * removeChild);        
}*/

class Node
{
    public:
        Node * parent;
        Node * child[6];
        int value[6];
        int childCount;
        int currentVal;
    
        Node() { 			//Default Constructor
            parent = NULL;
            for (int x = 0; x < 6; x++)
            {
                child[x] = NULL;
                value[x] = -1;
            }
            childCount = 0;
            currentVal = -1;
        }

        Node(int val) {
            parent = NULL;
            for (int x = 0; x < 6; x++)
            {
                child[x] = NULL;
                value[x] = -1;
            }

            childCount = 0;
            currentVal = val;
        }

        Node * getLeft() {
            if (leftExists())
                return child[0];
            else
                return NULL;
        }

        Node * getMiddle() {
            if (middleExists())
                return child[1];
            else
                return NULL;
        }

        Node * getRight() {
            if (middleExists())
                return child[2];
            else if (rightExists())
                return child[1];
            else
                return NULL;
        }

        Node * getParent() {
            return parent;
        }

        int getValue(){
            if (isLeaf()) 
                return currentVal;
            else {
                if (childCount == 3)
                    return value[2];
                else if (childCount == 2)
                    return value[1];
                else //1 child - "left"
                    return value[0];
            }
        }

        void setParent(Node * node) {
            parent = node;
        }

        void setChildValues() {
            for (int i = 0; i < 6; i++)
            {
                if (i < childCount)
                    value[i] = child[i]->getValue();
                else
                    value[i] = -1;
            }
        }

		//Helper function to check if a node is a leaf
        bool isLeaf() {
            if (currentVal == -1)
                return false;
            else
                return true;
        }

		//Helper function to check if left child exists
        bool leftExists() {
            if (isLeaf())
                return false;
            else
                return true;
        }

		//Helper function to check if middle child exists
        bool middleExists() {
            if (rightExists() && childCount > 2)
                return true;
            else
                return false;
        }

		//Helper function to check if right child exists
        bool rightExists() {
            if (leftExists() && childCount > 1)
                return true;
            else
                return false;
        }

		//Function for adding a chuld 
        void addChild(Node * node) {
            child[childCount++] = node;

            currentVal = -1;
            node->setParent(this);

            if(childCount == 2)
                sort(child[0], child[1]);
            if(childCount == 3)
                sort3(child[0], child[1], child[2]);
            if(childCount == 4)
                sort4(child[0], child[1], child[2], child[3]);

            setChildValues();
        }

		//Function for removing a child
        void removeChild(int pos) {
            //Removing the child by copying the children to the right
            childCount--;
            for (int x = pos; x < childCount; ++x)
            {
                child[pos] = child[pos + 1];
                value[pos] = value[pos + 1];
            }
            //Removing the last child (extra copy of last child)
            child[childCount] = NULL;
            value[childCount] = -1;
            return;
        }

		//Helper function to adjust Subtree
        void adjustSubtree() {
            int myOldValue = getValue();
            setChildValues();
            int myNewValue = getValue();
            if (myOldValue != myNewValue && parent != NULL)
                parent->adjustSubtree();
        }

        //Helper function for shifting nodes
        void shiftUp(int oldChild, int newChild) {
            if (oldChild < newChild)
            {
                child[oldChild]->removeChild(child[oldChild]->childCount - 1);
                child[newChild]->addChild(child[oldChild]);
            }
            else
            {
                child[oldChild]->removeChild(0);
                child[newChild]->addChild(child[oldChild]);
            }
        }

		//Helper function for sorting between 2 nodes
        void sort(Node * a, Node * b) {
            if (a->currentVal > b->currentVal)
            {
                Node * t = a;
                a = b;
                b = t;
            }
        }

		//Helper function for sorting between 3 nodes 
        void sort3(Node * a, Node * b, Node * c) {
            sort(a, b);
            sort(a, c);
            sort(b, c);
        }

		//Helper function for sorting between 4 nodes
        void sort4(Node * a, Node * b, Node * c, Node * d) {
            sort(a, b);
            sort(a, c);
            sort(a, d);
            sort(b, c);
            sort(b, d);
            sort(c, d);
        }
};
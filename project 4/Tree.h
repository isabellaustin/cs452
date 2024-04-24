//CS452 Project 4: Tree Class
//Written by Izzy Austin, Anna Vadella, and Noah Baker

#include <iostream>
#include "Node.h"
using namespace std;

//From Project Description
/*class Tree {
    Node * root;
    void print(Node * start);
    
    public:
        Tree(); // constructor
        Node * search(int valToFind);
        bool insert (int valToAdd); 
        bool delete (int valtokill);
        void print();
}*/

class Tree
{
    private:
        Node * root;

        //Helper function for Insert - Absorb node a into node b
        void absorb(Node * a, Node * b) {
            if (b == root) { 					//Base case
                Node * newRoot = new Node();
                newRoot->addChild(a);			//a will be the left child of newRoot
                newRoot->addChild(b);			//b will be the right child of newRoot

                newRoot->setChildValues();
                root = newRoot;
                return;
            }
            else  { 
                Node * p = b->getParent();		//Let p be the parent of node b
                p->addChild(a);
                if (p->parent != NULL && p->childCount <= 3) //"Easy insertion"
                    p->parent->adjustSubtree();
                else 
                {
                    Node * pLeft = new Node();
                    int leftChildren = p->childCount/2;
                    int rightChildren = p->childCount - leftChildren;

                    for (int i = 0; i < leftChildren; i++)
                    {
                        pLeft->addChild(p->child[i]);
                        p->child[i] = p->child[i + leftChildren];
                    }

                    if (leftChildren < rightChildren)
                        p->child[leftChildren] = p->child[p->childCount - 1];

                    for (int x = rightChildren; x < p->childCount; ++x)
                        p->child[x] = NULL;

                    p->childCount = rightChildren;
                    p->setChildValues();
                    absorb(pLeft, p);
                }
            }
        }

        //Helper function for Delete - discard a from the tree
        void discard(Node * a) {
            Node * p = a->getParent(); 		//Let p be the parent of a

            //Remove the connection between p and a by searching for node a in the parent
            for (int i = 0; i < p->childCount; i++)
                if (p->child[i] == a)
                    p->removeChild(i);

            delete a;
			
            bool recurse; 				//Bool to check whether or not recursion is necessary

            //Condition for if p has two children after deleting --> Parent could remove child
            if (p->childCount >= 2){
                return;
            }
            else if (p->getParent() == NULL)    //Deleted the root value
            {
                if (!p->child[0]->isLeaf())     //Set root to be the left child
                {
                    root = p->child[0];
                    delete p;
                }
                return;
            }

            //Find the total number of children among all siblings of parent p (including p), will range between 3 and 7
            Node * totchildren = p->getParent();
            int totalChildren = 0;
            int grandChild;

            //Calculate the total number of grandchildren
            for (int x = 0; x < totchildren->childCount; x++)
            {
                totalChildren += totchildren->child[x]->childCount;
                if (totchildren->child[x]->childCount == 1)
                    grandChild = x;
            }

            if (totalChildren >= 4)				//In this case, no recursion is needed!
            {
                if (totchildren->childCount == 2) {
                    if (totchildren->child[0]->childCount == 3)
                        totchildren->shiftUp(0, 1);
                    else
                        totchildren->shiftUp(1, 0);
				} 
                else if (totalChildren == 5) { 
                    if (grandChild == 2)
                        totchildren->shiftUp(grandChild, 1);
                    else
                        totchildren->shiftUp(grandChild, grandChild + 1);
				} 
                else { 
                    if (grandChild == 0)
                    {
                        totchildren->shiftUp(1, 0);
                        if (totchildren->child[1]->childCount == 1)
                            totchildren->shiftUp(2, 1);
                    }
                    else if (grandChild == 2)
                    {
                        totchildren->shiftUp(1, 2);
                        if (totchildren->child[1]->childCount == 1)
                            totchildren->shiftUp(0, 1);
                    }
                    else
                        if (totchildren->child[0]->childCount > totchildren->child[2]->childCount)
                            totchildren->shiftUp(0, 1);
                        else
                            totchildren->shiftUp(1, 1);
				}         
            }
            else //Allocate b to the sibling of p
            {
                if (grandChild == 0)
                    totchildren->shiftUp(0, 1);
                else
                    totchildren->shiftUp(1, 0);
                discard(p);
                recurse = false;
            }

            if (recurse)
                totchildren->adjustSubtree();
        }

    public: 
        Tree() { 			//Default constructor
            root = NULL;
        }

        Tree(Node * r) {
            root = r;
        }

        Node * Search(Node *root, int valToFind) {
            if (root->isLeaf())		//Base case for leaf, root has not children
                return root;

            if (root->getLeft()->getValue() >= valToFind) {		//Looking at leftmost element of node
                return Search(root->getLeft(), valToFind);		//Recursively checking left
			} 
            else if (root->middleExists() && root->getMiddle()->getValue() >= valToFind) {		//Have to check that this condition exists!
                return Search(root->getMiddle(), valToFind);	//Recursively checking middle
			} 
            else
                return Search(root->getRight(), valToFind);		//Recursively checking right
        }

        void Insert(int valToAdd) {
            Node * a = new Node(valToAdd);
            Node * b = Search(root, valToAdd);
            if (a == b) 
                return;
            else 
                absorb(a, b);
        }

        void Delete(int valToRemove) {
            Node * a = Search(root, valToRemove);			//Searching tree for the value to be removed 

            if (a->getValue() == valToRemove)
                discard(a);
        }

        void Print() {
            print(root);
        }

        void print(Node * r) {
            
            if(r == NULL) //No Tree
                cout << "The tree is empty." << endl;
            else {
                Node * parent = r;
                if(parent->childCount < 2)
                    cout << parent->getValue() << " has 1 child" << endl;
                else if(parent->childCount == 2) {
                    cout << parent->getValue() << " has 2 children: " << parent->getLeft()->getValue() << " " << parent->getRight()->getValue() << endl;
                    print(parent->getLeft());
                    print(parent->getRight());
                }
                else {
                    cout << parent->getValue() << " has 3 children: " << parent->getLeft()->getValue() << " " << parent->getMiddle()->getValue() << " " << parent->getRight()->getValue() << endl;
                    print(parent->getLeft());
                    print(parent->getMiddle());
                    print(parent->getRight());
                }
            }
        }
};
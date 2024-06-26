#ifndef _AVL_TREE_H_
#define _AVL_TREE_H_

typedef int Type;

typedef struct AVLTreeNode{
    Type key;
    int height;
    struct AVLTreeNode *left;
    struct AVLTreeNode *right; 
}Node, *AVLTree;


int avltree_height(AVLTree tree);


void preorder_avltree(AVLTree tree);

void inorder_avltree(AVLTree tree);

void postorder_avltree(AVLTree tree);

void print_avltree(AVLTree tree, Type key, int direction);


Node* avltree_search(AVLTree x, Type key);

Node* iterative_avltree_search(AVLTree x, Type key);


Node* avltree_minimum(AVLTree tree);

Node* avltree_maximum(AVLTree tree);


Node* avltree_insert(AVLTree tree, Type key);


Node* avltree_delete(AVLTree tree, Type key);


void destroy_avltree(AVLTree tree);


#endif
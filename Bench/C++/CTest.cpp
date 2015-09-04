// Exercise work on a binary tree writin in C++

#include <iostream>
#include "Node.h"

int main(void) {
  GC_init();

  //Create binary tree
  Node* root = Node::build_tree_of_size(1);

  //Do work on binary tree
  root->do_work();

  return 0;
}



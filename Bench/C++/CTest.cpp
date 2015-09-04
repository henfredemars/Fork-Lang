// Exercise work on a binary tree writin in C++

#include <iostream>
#include <cstring>
#include <stdlib.h>
#include "Node.h"

int main(void) {
  //Allow BDW subsystem to initialize
  GC_init();

  //Create binary tree
  Node* root = Node::build_tree_of_size(1000);
  root->populate();

  //Do work on binary tree
  std::cout << "Began working..." << std::endl;
  root->do_work();

  //Print Completion Message
  char* tcmp_buf = (char*)GC_malloc(200);
  std::string tcmsg = "Tasks have been completed.\n";
  memcpy(tcmp_buf,tcmsg.c_str(),tcmsg.length());
  snprintf(tcmp_buf+tcmsg.length(),200-tcmsg.length(),"Tree had size: %d\n",root->size());
  std::cout << tcmp_buf;

  return 0;
}



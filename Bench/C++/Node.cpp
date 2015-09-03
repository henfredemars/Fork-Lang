// Node definition and implementation for C++ binary tree benchmark program
//   Be careful not to compile with optimizations or do_work could get optimized away

#include <random>
#include <unistd.h>
#include "gc.h"
#include "gc_cpp.h"
#include "util.cpp"

#define NULL 0
#define WORK_COST_MS 100

class Node : public gc {
  private:
    static std::uniform_real_distribution<double> generator(0, 1);
    static std::mt19937_64 rng;
    double value;
    Node* leftchild;
    Node* rightchild;
  public:
    Node(void);
    void populate(void);
    void do_work(void);
    static Node* build_tree_of_size(int size);
    void insert_into_tree(Node* new_node);
};


// Class constructor
Node::Node(void) {
  this->value = 0;
  this->leftchild = NULL;
  this->rightchild = NULL;
}

// Insert a new node into a random location in the tree
void Node::insert_into_tree(Node* new_node) {
  if (this->leftchild) {
    if (this->rightchild) { //Both children exist
      if (this->generator(rng) < 0.5) {
        this->leftchild->insert_into_tree(new_node);
      } else {
        this->rightchild->insert_into_tree(new_node);
      }
    } else { //Left but no right, choose right
      this->rightchild = new_node;
    }
  } else if (this->rightchild) { //Right but no left
    this->leftchild = new_node;
  } else { //No children, choose randomly
    if (this->generator(rng) < 0.5) {
      this->leftchild = new_node;
    } else {
      this->rightchild = new_node;
    }
  }
}

// Populate the tree from here down with random values
void Node::populate(void) {
  this->value = this->generator(rng);
  (this->leftchild) ? (this->leftchild->populate()) : (null_function());
  (this->rightchild) ? (this->rightchild->populate()) : (null_function());
}

// Do computationally expensive work at each node
void Node::do_work(void) {
  usleep(WORK_COST_MS);
  (this->leftchild) ? (this->leftchild->do_work()) : (null_function());
  (this->rightchild) ? (this->rightchild->do_work()) : (null_function());
}

// Build tree to contain 'size' Nodes
Node* build_tree_of_size(int size) {
  if (size < 1) return NULL;
  int actual_size = 1;
  Node* root = new Node();
  while (actual_size < size) {
    root->insert_into_tree(new Node());
    actual_size++;
  }
  return root;
}



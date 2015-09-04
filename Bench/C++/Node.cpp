// Node implementation file

#include "Node.h"

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
      if (Node::generator->operator()(*rng) < 0.5) {
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
    if (Node::generator->operator()(*rng) < 0.5) {
      this->leftchild = new_node;
    } else {
      this->rightchild = new_node;
    }
  }
}

// Populate the tree from here down with random values
void Node::populate(void) {
  this->value = Node::generator->operator()(*rng);
  (this->leftchild) ? (this->leftchild->populate()) : (null_function());
  (this->rightchild) ? (this->rightchild->populate()) : (null_function());
}

// Compute the size of the tree
int Node::size(void) const {
  int count = 1;
  if (this->leftchild) {
    count += this->leftchild->size();
  }
  if (this->rightchild) {
    count += this->rightchild->size();
  }
  return count;
}

// Do computationally expensive work at each node
void Node::do_work(void) {
  for (int i = 0; i < WORK_COST; i++) {
    this->value = pow(this->value,M_PI);
  }
  (this->leftchild) ? (this->leftchild->do_work()) : (null_function());
  (this->rightchild) ? (this->rightchild->do_work()) : (null_function());
}

// Build tree evenly to contain 'size' Nodes
Node* Node::build_tree_of_size(int size) {
  if (size < 1) return NULL;
  int actual_size = 1;
  Node* root = new Node();
  while (actual_size < size) {
    root->insert_into_tree(new Node());
    actual_size++;
  }
  return root;
}

// Initialize static members
std::uniform_real_distribution<double>* Node::generator = new std::uniform_real_distribution<double>();
std::mt19937_64* Node::rng = new std::mt19937_64();


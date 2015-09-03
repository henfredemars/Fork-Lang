// Node definition and implementation for C++ binary tree benchmark program
//   Be careful not to compile with optimizations or do_work could get optimized away

#include <random>
#include <unistd.h>
#include "gc.h"
#include "gc_cpp.h"
#include "util.cpp"

#define NULL 0
#define WORK_COST 100

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
};


// Class constructor
Node::Node(void) {
  this->value = 0;
  this->leftchild = NULL;
  this->rightchild = NULL;
}

// Populate the tree with random values
void Node::populate(void) {
  this->value = this->generator(rng);
  (this->leftchild) ? (this->leftchild->populate()) : (null_function())
  (this->rightchild) ? (this->rightchild->populate()) : (null_function())
}

// Do computationally expensive work
void Node::do_work(void) {
  usleep(100);
}

// Node definition and implementation for C++ binary tree benchmark program

#ifndef NODE_H
#define NODE_H

// Attempt to detect more memory errors
#define GC_DEBUG

#include <random>
#include <math.h>
#include "../../gc/include/gc.h"
#include "../../gc/include/gc_cpp.h"
#include "util.h"

#define WORK_COST 100000

class Node : public gc {
  private:
    double value;
    Node* leftchild;
    Node* rightchild;
  public:
    Node(void);
    void populate(void);
    void do_work(void);
    int size(void) const;
    static Node* build_tree_of_size(int size);
    void insert_into_tree(Node* new_node);
    static std::uniform_real_distribution<double>* generator;
    static std::mt19937_64* rng;
};

#endif /* Node.h */

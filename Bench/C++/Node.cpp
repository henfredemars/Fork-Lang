// Node definition and implementation for C++ binary tree benchmark program

#include <random>

class Node {
  private:
    static std::uniform_real_distribution<double> generator(0, 1);
    static std::mt19937_64 rng;
    double value;
    Node* leftchild;
    Node* rightchild;

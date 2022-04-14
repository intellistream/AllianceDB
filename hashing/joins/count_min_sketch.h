
#ifndef COUNT_MIN_SKETCH_H
#define COUNT_MIN_SKETCH_H


// define some constants
# define LONG_PRIME 4294967311l
# define MIN(a,b)  (a < b ? a : b)

#include "../utils/types.h" /* tuple_t */

/** CountMinSketch class definition here **/
struct CountMinSketch {
  // width, depth 
  unsigned int w,d;
  
  // eps (for error), 0.01 < eps < 1
  // the smaller the better
  float eps;
  
  // gamma (probability for accuracy), 0 < gamma < 1
  // the bigger the better
  float gamma;
  
  // aj, bj \in Z_p
  // both elements of fild Z_p used in generation of hash
  // function
  unsigned int aj, bj;

  // total count so far
  unsigned int total; 

  // array of arrays of counters
  int **C;

  // array of hash values for a particular item 
  // contains two element arrays {aj,bj}
  int **hashes;

  // generate "new" aj,bj
  void genajbj(int **hashes, int i);

public:
  // constructor
  CountMinSketch(float eps, float gamma);
  void cons();
  CountMinSketch();
  CountMinSketch(CountMinSketch &c);
  CountMinSketch(const CountMinSketch &c);
//  CountMinSketch& operator=(CountMinSketch &c);
  CountMinSketch& operator=(const CountMinSketch &c);
  // update item (int) by count c
  void update(intkey_t item, int c);
  // update item (string) by count c
//  void update(const char *item, int c);

  // estimate count of item i and return count
  unsigned int estimate(intkey_t item);
//  unsigned int estimate(const char *item);

  // return total count
  unsigned int totalcount();

  // generates a hash value for a string
  // same as djb2 hash function
//  unsigned int hashstr(const char *str);

  // destructor
  ~CountMinSketch();
};


#endif //COUNT_MIN_SKETCH_H 
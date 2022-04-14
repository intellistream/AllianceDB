# include <iostream>
# include <cmath>
# include <cstdlib>
# include <ctime>
# include <limits>
# include "count_min_sketch.h"
using namespace std;

/**
   Class definition for CountMinSketch.
   public operations:
   // overloaded updates
   void update(int item, int c);
   void update(char *item, int c);
   // overloaded estimates
   unsigned int estimate(int item);
   unsigned int estimate(char *item);
**/


// CountMinSketch constructor
// ep -> error 0.01 < ep < 1 (the smaller the better)
// gamma -> probability for error (the smaller the better) 0 < gamm < 1
CountMinSketch::CountMinSketch(float ep, float gamm) {
  if (!(0.009 <= ep && ep < 1)) {
    cout << "eps must be in this range: [0.01, 1)" << endl;
    exit(EXIT_FAILURE);
  } else if (!(0 < gamm && gamm < 1)) {
    cout << "gamma must be in this range: (0,1)" << endl;
    exit(EXIT_FAILURE);
  }
  eps = ep;
  gamma = gamm;
  w = ceil(exp(1)/eps);
  d = ceil(log(1/gamma));
  total = 0;
  // initialize counter array of arrays, C
  C = new int *[d];
  unsigned int i, j;
  for (i = 0; i < d; i++) {
    C[i] = new int[w];
    for (j = 0; j < w; j++) {
      C[i][j] = 0;
    }
  }
  // initialize d pairwise independent hashes
  srand(time(NULL));
  hashes = new int* [d];
  for (i = 0; i < d; i++) {
    hashes[i] = new int[2];
    genajbj(hashes, i);
  }

//  cout <<"Pointer Place  "<< C <<" "<<hashes <<'\n';
//  fflush(stdout);
}

void CountMinSketch::cons() {
  float ep = 0.1, gamm = 0.1;
  if (!(0.00009 <= ep && ep < 1)) {
    cout << "eps must be in this range: [0.01, 1)" << endl;
    exit(EXIT_FAILURE);
  } else if (!(0 < gamm && gamm < 1)) {
    cout << "gamma must be in this range: (0,1)" << endl;
    exit(EXIT_FAILURE);
  }
  eps = ep;
  gamma = gamm;
  w = ceil(exp(1)/eps);
  d = ceil(log(1/gamma));
  total = 0;
  // initialize counter array of arrays, C
  C = new int *[d];
  unsigned int i, j;
  for (i = 0; i < d; i++) {
    C[i] = new int[w];
    for (j = 0; j < w; j++) {
      C[i][j] = 0;
    }
  }
  // initialize d pairwise independent hashes
  srand(time(NULL));
  hashes = new int* [d];
  for (i = 0; i < d; i++) {
    hashes[i] = new int[2];
    genajbj(hashes, i);
  }

//  cout <<"Pointer Place  "<< C <<" "<<hashes <<'\n';
//  fflush(stdout);
}

CountMinSketch::CountMinSketch() {
//  cout << "\n\nI'm alive!\n\n";
  // float ep = 0.01, gamm = 0.01;
  float ep = 0.1, gamm = 0.1;
//  cout << "\n\nI'm alive!  "<< ep << "  "<< gamm <<" \n\n";
//  fflush(stdout);
  if (!(0.009 <= ep && ep < 1)) {
    cout << "eps must be in this range: [0.01, 1)" << endl;
    exit(EXIT_FAILURE);
  } else if (!(0 < gamm && gamm < 1)) {
    cout << "gamma must be in this range: (0,1)" << endl;
    exit(EXIT_FAILURE);
  }
  eps = ep;
  gamma = gamm;
  w = ceil(exp(1)/eps);
  d = ceil(log(1/gamma));
  total = 0;
  // initialize counter array of arrays, C
  C = new int *[d];
  unsigned int i, j;
  for (i = 0; i < d; i++) {
    C[i] = new int[w];
    for (j = 0; j < w; j++) {
      C[i][j] = 0;
    }
  }
  // initialize d pairwise independent hashes
  srand(time(NULL));
  hashes = new int* [d];
  for (i = 0; i < d; i++) {
    hashes[i] = new int[2];
    genajbj(hashes, i);
  }
//  cout <<"Pointer Place  "<< C <<" "<<hashes <<'\n';
//  fflush(stdout);
}


CountMinSketch::CountMinSketch(CountMinSketch &c)
{
//  cout <<"copy swapped \n";
//  fflush(stdout);
  C = c.C;
  hashes = c.hashes;
  d = c.d;
  w = c.w;
  eps = c.eps;
  gamma = c.gamma;
  total = c.total;
  c.C = NULL;
  c.hashes = NULL;
}

CountMinSketch::CountMinSketch(const CountMinSketch &c)
{
//  cout <<"copy swapped const\n";
//  fflush(stdout);
  C = c.C;
  hashes = c.hashes;
  d = c.d;
  w = c.w;
  eps = c.eps;
  gamma = c.gamma;
  total = c.total;
  const_cast<CountMinSketch &>(c).C = NULL;
  const_cast<CountMinSketch &>(c).hashes = NULL;
}
/*
CountMinSketch& CountMinSketch::operator=(CountMinSketch &c)
{
  cout <<"swapped \n";
  fflush(stdout);
  C = c.C;
  hashes = c.hashes;
  d = c.d;
  w = c.w;
  eps = c.eps;
  gamma = c.gamma;
  total = c.total;
  c.C = NULL;
  c.hashes = NULL;
}
*/
CountMinSketch& CountMinSketch::operator=(const CountMinSketch &c)
{
//  cout <<"swapped const\n" <<" its place: "<<&c << "\n";
//  fflush(stdout);
  C = c.C;
  hashes = c.hashes;
//  cout <<"Copyed Pointers  "<< C <<" "<<hashes <<'\n';
//  fflush(stdout);
  d = c.d;
  w = c.w;
  eps = c.eps;
  gamma = c.gamma;
  total = c.total;
//  const_cast<CountMinSketch &>(c).C = NULL;
//  const_cast<CountMinSketch &>(c).hashes = NULL;
}


// CountMinSkectch destructor
CountMinSketch::~CountMinSketch() {
  // free array of counters, C
  unsigned int i;
  if (C != NULL)
  {
    for (i = 0; i < d; i++) {
      delete[] C[i];
    }
    delete[] C;
  }
  
  // free array of hash values
  if (hashes != NULL)
  {
    for (i = 0; i < d; i++) {
      delete[] hashes[i];
    }
    delete[] hashes;
  }
}

// CountMinSketch totalcount returns the
// total count of all items in the sketch
unsigned int CountMinSketch::totalcount() {
  return total;
}

// countMinSketch update item count (int)
void CountMinSketch::update(intkey_t item, int c) {
//  cout <<"Used Pointer Place  "<< C <<" "<<hashes <<'\n';
//  fflush(stdout);
  total = total + c;
  unsigned int hashval = 0;
  
  for (unsigned int j = 0; j < d; j++) {
    hashval = ((long long)hashes[j][0]*item+hashes[j][1])%LONG_PRIME%w;
    hashval %= w;
    C[j][hashval] = C[j][hashval] + c;
  }
}
/*
// countMinSketch update item count (string)
void CountMinSketch::update(const char *str, int c) {
  int hashval = hashstr(str);
  update(hashval, c);
}
*/
// CountMinSketch estimate item count (int)
unsigned int CountMinSketch::estimate(intkey_t item) {
  int minval = numeric_limits<int>::max();
  unsigned int hashval = 0;
  for (unsigned int j = 0; j < d; j++) {
    hashval = ((long long)hashes[j][0]*item+hashes[j][1])%LONG_PRIME%w;
    hashval %= w;
    minval = MIN(minval, C[j][hashval]);
  }
  return minval;
}
/*
// CountMinSketch estimate item count (string)
unsigned int CountMinSketch::estimate(const char *str) {
  int hashval = hashstr(str);
  return estimate(hashval);
}
*/
// generates aj,bj from field Z_p for use in hashing
void CountMinSketch::genajbj(int** hashes, int i) {
  hashes[i][0] = int(float(rand())*float(LONG_PRIME)/float(RAND_MAX) + 1);
  hashes[i][1] = int(float(rand())*float(LONG_PRIME)/float(RAND_MAX) + 1);
}

// generates a hash value for a sting
// same as djb2 hash function
/*
unsigned int CountMinSketch::hashstr(const char *str) {
  unsigned long hash = 5381;
  int c;
  while (c = *str++) {
    hash = ((hash << 5) + hash) + c; // hash * 33 + c 
  }
  return hash;
}
*/
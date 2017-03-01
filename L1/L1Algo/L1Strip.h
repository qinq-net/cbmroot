#ifndef _L1Strip_h_
#define _L1Strip_h_

#include "../CbmL1Def.h"
#include "omp.h"

struct L1Branch;

struct L1Strip{
//  L1Strip():f(0),n(0){}
//  L1Strip(fscal _f, unsigned short int _n = 0):f(_f),n(_n){}
  
   L1Strip():f(0){
   }
  L1Strip(fscal _f, unsigned short int _n = 0):f(_f){ };

  operator fscal() const { return f;}
//   operator fvec() { return fscal(f);}

// private:
  fscal f;
 
 // unsigned short int n; // number of event
};

struct L1StripSelectTracks{
  
  L1StripSelectTracks(){   
  //  Candidates.resize(70);
   // Candidates2.resize(70);
    omp_init_lock(&Occupied); 
    Cand = -1;
  }
  
  ~L1StripSelectTracks()
  {
    omp_destroy_lock(&Occupied);
  }
  
//  L1Vector <L1Branch*> Candidates;
//  L1Vector <int> Candidates2;
  int Cand;
  omp_lock_t Occupied;
};

typedef unsigned /*short*/ int TStripI; // strip index type

#endif

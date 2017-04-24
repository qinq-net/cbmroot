#ifndef _L1StsHit_h_
#define _L1StsHit_h_

#include "L1Strip.h"

                   
//struct L1Branch;
typedef unsigned /*short*/ int THitI; // strip index type
typedef unsigned short int TZPosI;    // strip z-coor index type


class L1StsHit
{
  public:
  TStripI f, b; // front and back strip indices
  TZPosI iz; // index of z coor. in L1Algo::vStsZPos
  
#ifdef USE_EVENT_NUMBER   
    int n;
#endif 
    
    float t_reco;
    float t_er;

   // int ista;

  
  L1StsHit():f(0),b(0),iz(0), 
  
#ifdef USE_EVENT_NUMBER  
  n(0),
#endif  
  
  
  t_reco(0.f){

  }
  L1StsHit(L1StsHit &h,  /*short*/ int sh, /*short*/ int shB):f(h.f + sh),b(h.b + shB),iz(0), 
  
#ifdef USE_EVENT_NUMBER     
  n(0),
#endif   
  
t_reco(0.f) {}

};

#endif

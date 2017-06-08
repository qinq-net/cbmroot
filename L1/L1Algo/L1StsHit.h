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
    int n;
//    std::vector<L1Branch*> CandidatesB;
//    std::vector<L1Branch*> CandidatesF;
// short int CandF;
// short int CandB;
    bool used;
    float t_reco;
    float t_er;
    float t_mc;
    int ista;
    float time1, time2;
   
 // unsigned short int n; // number of event
  
  L1StsHit() 
    :f(0),b(0),iz(0), n(0),used(false),t_reco(0.f),t_er(0.),t_mc(0.),ista(-1), time1(0.), time2(0.) {
//   CandidatesB.resize(0, 0);
//   CandidatesF.resize(0, 0);
  }
  L1StsHit(L1StsHit &h,  /*short*/ int sh, /*short*/ int shB)
    :f(h.f + sh),b(h.b + shB),iz(0), n(0),used(false),t_reco(0.f),t_er(0.),t_mc(0.),ista(-1),time1(0.), time2(0.) {}

};

#endif

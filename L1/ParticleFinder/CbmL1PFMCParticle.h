/*
 *=====================================================
 *
 *  CBM Level 1 Reconstruction
 *
 *  Authors: M.Zyzak
 *
 *  e-mail :
 *
 *=====================================================
 *
 *  Finds Particles: Lambdas, K0
 *
 */

#ifndef _CbmL1PFMCParticle_h_
#define _CbmL1PFMCParticle_h_

#include "TObject.h"
#include <vector>

class CbmL1MCTrack;

struct McVector
{
  std::vector< CbmL1MCTrack* >  mcTracks;
};

struct McTrackFind
{
  CbmL1MCTrack* mcTrack;
  int vNumber;
};



#endif


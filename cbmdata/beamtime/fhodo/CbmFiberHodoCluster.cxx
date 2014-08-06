/**
 * \file CbmFiberHodoCluster.cxx
 * \author Florian Uhlig <f.uhlig@gsi.de>
 * \brief Data Container for TRD clusters.
 */
#include "CbmFiberHodoCluster.h"

CbmFiberHodoCluster::CbmFiberHodoCluster()
  : CbmCluster()
  , fMean(0.)
  , fMeanError(0)
{
}

CbmFiberHodoCluster::~CbmFiberHodoCluster(){
}

ClassImp(CbmFiberHodoCluster)

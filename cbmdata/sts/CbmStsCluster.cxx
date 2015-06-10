/**
 ** \file CbmStsCluster.cxx
 ** \author V.Friese <v.friese@gsi.de>
 ** \since 28.08.06
 **
 ** Updated 25/06/2008 by R. Karabowicz.
 ** Updated 04/03/2014 by A. Lebedev <andrey.lebedev@gsi.de>
 ** Updated 10/06/2014 by V.Friese <v.friese@gsi.de>
 **/
#include "CbmStsCluster.h"


// --- Constructor
CbmStsCluster::CbmStsCluster()
    : CbmCluster()
    , fCharge(0.)
    , fChannelMean(0.)
    , fChannelMeanSq(0.)
    , fTime(0.)
    , fIndex(-1)
{
}


// --- Destructor
CbmStsCluster::~CbmStsCluster()
{
}


ClassImp(CbmStsCluster)

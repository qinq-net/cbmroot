/**
 * \file CbmTrdCluster.cxx
 * \author Florian Uhlig <f.uhlig@gsi.de>
 * \brief Data Container for TRD clusters.
 */
#include "CbmTrdCluster.h"

CbmTrdCluster::CbmTrdCluster()
  : CbmCluster(),
    fNCols(0),
    fNRows(0)
{
}

CbmTrdCluster::~CbmTrdCluster(){
}

ClassImp(CbmTrdCluster)

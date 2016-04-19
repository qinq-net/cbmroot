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
CbmTrdCluster::CbmTrdCluster(const std::vector<Int_t>& indices, Int_t address)
  : CbmCluster(indices,address),
    fNCols(0),
    fNRows(0)
{
}
CbmTrdCluster::~CbmTrdCluster(){
}

ClassImp(CbmTrdCluster)

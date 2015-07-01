// ------------------------------------------------------
// -----              CbmD0Tools class file         -----
// -----          Created 27.05.2015 by P.Sitzmann  -----
// ------------------------------------------------------
#include "CbmD0Tools.h"

#include "TObject.h"


// Includes from KF
#include "KFPVertex.h"
#include "KFPTrack.h"

// Includes from Cbm
#include "CbmVertex.h"
#include "CbmTrack.h"

// Includes from Fair
#include "FairLogger.h"


// -------------------------------------------------------------------------
CbmD0Tools::CbmD0Tools()
{
;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
CbmD0Tools::~CbmD0Tools()
{
;
}
// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
KFPVertex* CbmD0Tools::CbmVertexToKFPVertex(const CbmVertex& vertex)
{
KFPVertex* kfpVertex = new KFPVertex();

kfpVertex->SetXYZ(vertex.GetX(), vertex.GetY(), vertex.GetZ()); // position
kfpVertex->SetCovarianceMatrix(vertex.GetCovariance(0,0),vertex.GetCovariance(1,0),vertex.GetCovariance(1,1),vertex.GetCovariance(2,0),vertex.GetCovariance(2,1),vertex.GetCovariance(2,2)); // upper right half of covarinace matrix
kfpVertex->SetNContributors(vertex.GetNTracks()); // nummber of participans
kfpVertex->SetChi2(vertex.GetChi2());
kfpVertex->SetNDF(vertex.GetNDF()); //degrees of freedom

return kfpVertex;
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
KFPTrack*  CbmD0Tools::CbmTrackToKFPTrack(const CbmTrack& track)
{
;
}
// -------------------------------------------------------------------------


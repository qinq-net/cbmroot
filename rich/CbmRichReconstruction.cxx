/**
 * \file CbmRichReconstruction.cxx
 *
 * \author Semen Lebedev
 * \date 2012
 **/

#include "CbmRichReconstruction.h"
#include "CbmRichRing.h"

#include "CbmRichProjectionProducerAnalytical.h"
#include "CbmRichProjectionProducerTGeo.h"
#include "prototype/CbmRichProtProjectionProducer.h"

#include "CbmRichTrackExtrapolationBase.h"
#include "CbmRichTrackExtrapolationIdeal.h"
#include "CbmRichTrackExtrapolationMirrorIdeal.h"
#include "CbmRichTrackExtrapolationKF.h"
#include "CbmRichTrackExtrapolationLittrack.h"

#include "CbmRichRingFinderHough.h"
#include "CbmRichRingFinderIdeal.h"
#include "CbmL1RichENNRingFinder.h"
//#include "CbmL1RichENNRingFinderParallel.h"
#include "prototype/CbmRichProtRingFinderHough.h"

#include "CbmRichRingFitterCircle.h"
#include "CbmRichRingFitterTAU.h"
#include "CbmRichRingFitterCOP.h"
#include "CbmRichRingFitterRobustCOP.h"
#include "CbmRichRingFitterEllipseTau.h"
#include "CbmRichRingFitterEllipseMinuit.h"

#include "CbmRichRingTrackAssignClosestD.h"

#include "FairHit.h"
#include "CbmGlobalTrack.h"
#include "FairRootManager.h"
#include "FairLogger.h"
#include "CbmRichConverter.h"
#include "TClonesArray.h"

#include <iostream>

using std::cout;
using std::endl;

CbmRichReconstruction::CbmRichReconstruction()
: FairTask("CbmRichReconstruction"),
fRichHits(NULL),
fRichRings(NULL),
fRichProjections(NULL),
fRichTrackParamZ(NULL),
fGlobalTracks(NULL),

fRingFinder(NULL),
fRingFitter(NULL),
fTrackExtrapolation(NULL),
fProjectionProducer(NULL),
fRingTrackAssign(NULL),

fRunExtrapolation(true),
fRunProjection(true),
fRunFinder(true),
fRunFitter(true),
fRunTrackAssign(true),

fExtrapolationName("littrack"),
fProjectionName("analytical"),
fFinderName("hough"),
fFitterName("ellipse_tau"),
fTrackAssignName("closest_distance"),

fZTrackExtrapolation(260.)
{
    
}

CbmRichReconstruction::~CbmRichReconstruction()
{
    if (NULL != fRingFinder) delete fRingFinder;
    if (NULL != fRingFitter) delete fRingFitter;
    if (NULL != fTrackExtrapolation) delete fTrackExtrapolation;
    if (NULL != fProjectionProducer) delete fProjectionProducer;
}

InitStatus CbmRichReconstruction::Init()
{
    FairRootManager* ioman = FairRootManager::Instance();
    if (NULL == ioman) { Fatal("CbmRichReconstruction::Init","RootManager not instantised!"); }
    
    if (fRunExtrapolation && fRunProjection) {
        fRichTrackParamZ = new TClonesArray("FairTrackParam",100);
        ioman->Register("RichTrackParamZ", "RICH", fRichTrackParamZ, IsOutputBranchPersistent("RichTrackParamZ"));
        
        fGlobalTracks = (TClonesArray*) ioman->GetObject("GlobalTrack");
        if ( NULL == fGlobalTracks) { Fatal("CbmRichReconstruction::Init", "No GlobalTrack array!");}
        
        fRichProjections = new TClonesArray("FairTrackParam");
        ioman->Register("RichProjection", "RICH", fRichProjections, IsOutputBranchPersistent("RichProjection"));
    }

    fRichHits = (TClonesArray*) ioman->GetObject("RichHit");
    if ( NULL == fRichHits) { Fatal("CbmRichReconstruction::Init","No RichHit array!"); }

    fRichRings = new TClonesArray("CbmRichRing", 100);
    ioman->Register("RichRing", "RICH", fRichRings, IsOutputBranchPersistent("RichRing"));
    
    if (fRunExtrapolation) InitExtrapolation();
    if (fRunProjection) InitProjection();
    if (fRunFinder) InitFinder();
    if (fRunFitter) InitFitter();
    if (fRunTrackAssign) InitTrackAssign();
    
    return kSUCCESS;
}

void CbmRichReconstruction::Exec(
                                 Option_t* /*opt*/)
{
    LOG(INFO) << "CbmRichReconstruction Exec" << FairLogger::endl;
    if (fRunExtrapolation) RunExtrapolation();
    if (fRunProjection) RunProjection();
    if (fRunFinder) RunFinder();
    if (fRunFitter) RunFitter();
    if (fRunTrackAssign) RunTrackAssign();
}

void CbmRichReconstruction::InitExtrapolation()
{
    if (fExtrapolationName == "ideal"){
        fTrackExtrapolation = new CbmRichTrackExtrapolationIdeal();
    } else if (fExtrapolationName == "mirror_ideal"){
        fTrackExtrapolation = new CbmRichTrackExtrapolationMirrorIdeal();
    } else if (fExtrapolationName == "kf" || fExtrapolationName == "KF"){
        fTrackExtrapolation = new CbmRichTrackExtrapolationKF();
    } else if (fExtrapolationName == "lit" || fExtrapolationName == "littrack"){
        fTrackExtrapolation = new CbmRichTrackExtrapolationLittrack();
    } else {
        LOG(FATAL) << fExtrapolationName << " is not correct name for extrapolation algorithm." << FairLogger::endl;
    }
    fTrackExtrapolation->Init();
}

void CbmRichReconstruction::InitProjection()
{
    if (fProjectionName == "analytical") {
        fProjectionProducer = new CbmRichProjectionProducerAnalytical();
    } else if (fProjectionName == "TGeo" || fProjectionName == "tgeo") {
        fProjectionProducer = new CbmRichProjectionProducerTGeo();
    } else {
        LOG(FATAL) << fFinderName << " is not correct name for projection producer algorithm." << FairLogger::endl;
    }
    fProjectionProducer->Init();
}

void CbmRichReconstruction::InitFinder()
{
    if (fFinderName == "hough"){
        fRingFinder = new CbmRichRingFinderHough();
    } else if (fFinderName == "ideal"){
        fRingFinder = new CbmRichRingFinderIdeal();
    } else if (fFinderName == "enn"){
        fRingFinder = new CbmL1RichENNRingFinder(0);
    } else if ((fFinderName == "enn_parallel")){
        // fRingFinder = new CbmL1RichENNRingFinderParallel(0);
    } else if (fFinderName == "hough_prototype") {
        fRingFinder = new CbmRichProtRingFinderHough();
    } else {
        LOG(FATAL) << fFinderName << " is not correct name for ring finder algorithm." << FairLogger::endl;
    }
    
    fRingFinder->Init();
}

void CbmRichReconstruction::InitFitter()
{
    if (fFitterName == "circle_cop"){
        fRingFitter = new CbmRichRingFitterCOP();
    } else if (fFitterName == "circle_simple") {
        fRingFitter = new CbmRichRingFitterCircle();
    } else if (fFitterName == "circle_tau") {
        fRingFitter = new CbmRichRingFitterTAU();
    } else if (fFitterName == "circle_robust_cop") {
        fRingFitter = new CbmRichRingFitterRobustCOP();
    } else if (fFitterName == "ellipse_tau") {
        fRingFitter = new CbmRichRingFitterEllipseTau();
    } else if (fFitterName == "ellipse_minuit") {
        fRingFitter = new CbmRichRingFitterEllipseMinuit();
    } else {
        LOG(FATAL) << fFitterName << " is not correct name for ring fitter algorithm." << FairLogger::endl;
    }
    CbmRichConverter::Init();
}

void CbmRichReconstruction::InitTrackAssign()
{
    if (fTrackAssignName == "closest_distance"){
        fRingTrackAssign = new CbmRichRingTrackAssignClosestD();
    } else {
        LOG(FATAL) << fTrackAssignName << " is not correct name for ring-track assignment algorithm." << FairLogger::endl;
    }
    fRingTrackAssign->Init();
}

void CbmRichReconstruction::RunExtrapolation()
{
    if (fRichTrackParamZ == NULL) LOG(INFO) << "fRichTrackParamZ == NULL" << FairLogger::endl;
    fRichTrackParamZ->Delete();
    fTrackExtrapolation->DoExtrapolation(fGlobalTracks, fRichTrackParamZ, fZTrackExtrapolation);
}

void CbmRichReconstruction::RunProjection()
{
    fProjectionProducer->DoProjection(fRichProjections);
}

void CbmRichReconstruction::RunFinder()
{
    fRichRings->Delete();
    fRingFinder->DoFind(fRichHits, fRichProjections, fRichRings);
}

void CbmRichReconstruction::RunFitter()
{
    int nofRings = fRichRings->GetEntriesFast();
    for (int iRing = 0; iRing < nofRings; iRing++) {
        CbmRichRing* ring = (CbmRichRing*)fRichRings->At(iRing);
        if (NULL == ring) continue;
        CbmRichRingLight ringL;
        
        CbmRichConverter::CopyHitsToRingLight(ring, &ringL);
        fRingFitter->DoFit(&ringL);
        CbmRichConverter::CopyParamsToRing(&ringL, ring);
    }
}

void CbmRichReconstruction::RunTrackAssign()
{
    fRingTrackAssign->DoAssign(fRichRings, fRichProjections);
}

void CbmRichReconstruction::Finish()
{
    
}

ClassImp(CbmRichReconstruction)

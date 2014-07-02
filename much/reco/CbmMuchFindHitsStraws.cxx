/**
 * \file CbmMuchFindHitsStraws.cxx
 **/

#include "CbmMuchFindHitsStraws.h"
#include "CbmMuchDigitizeStraws.h"
#include "CbmMuchStrawDigi.h"
#include "CbmMuchDigiMatch.h"
#include "CbmMuchAddress.h"
#include "CbmMuchLayerSide.h"
#include "CbmMuchModule.h"
#include "CbmMuchCluster.h"
#include "CbmMuchPoint.h"
#include "CbmMuchStation.h"
#include "CbmMuchPixelHit.h"
#include "CbmMuchStrawHit.h"
#include "FairRun.h"
#include "FairRootManager.h"
#include "FairGeoRotation.h"
#include "TRandom.h"
#include "TVector3.h"
#include "TClonesArray.h"

#include <cassert>

// -------------------------------------------------------------------------
CbmMuchFindHitsStraws::CbmMuchFindHitsStraws(const char* digiFileName)
    : FairTask("CbmMuchFindHitsStraws")
    , fGeoScheme(CbmMuchGeoScheme::Instance())
    , fDigiFile(digiFileName)
    , fPoints(NULL)
    , fDigis(NULL)
    , fDigiMatches(NULL)
    , fClusters(NULL)
    , fHits(NULL)
    , fEffic(0)
    , fMerge(0)
    , fMirror(0)
    , fBinary(1)
    , fDimens(1)
    , fPhis()
{
    SetPhis(0., 10, -10.);
}

// -------------------------------------------------------------------------
CbmMuchFindHitsStraws::~CbmMuchFindHitsStraws()
{
    /*
     if ( fHits ) {
        fHits->Delete();
        delete fHits;
     }
    */
}

// -------------------------------------------------------------------------
InitStatus CbmMuchFindHitsStraws::Init()
{

    // Get Digitizer task to obtain info dimension
    CbmMuchDigitizeStraws* digStr = (CbmMuchDigitizeStraws*)FairRun::Instance()->GetTask("CbmMuchDigitizeStraws");
    fDimens = digStr->GetDimens();

    // Get input array
    FairRootManager* ioman = FairRootManager::Instance();
    if (!ioman)
        Fatal("Init", "No FairRootManager");
    if (fDimens == 1)
    {
        fDigis = (TClonesArray*)ioman->GetObject("MuchStrawDigi");
        fDigiMatches = (TClonesArray*)ioman->GetObject("MuchStrawDigiMatch");
    }
    else
    {
        fPoints = (TClonesArray*)ioman->GetObject("MuchPoint");
        fDigis = (TClonesArray*)ioman->GetObject("MuchDigi");
        fDigiMatches = (TClonesArray*)ioman->GetObject("MuchDigiMatch");
        fClusters = (TClonesArray*)ioman->GetObject("MuchCluster");
    }

    // Initialize GeoScheme
    TFile* oldfile = gFile;
    TFile* file = new TFile(fDigiFile);
    TObjArray* stations = (TObjArray*)file->Get("stations");
    file->Close();
    file->Delete();
    gFile = oldfile;
    fGeoScheme->Init(stations);

    // Register hit array
    fHits = new TClonesArray("CbmMuchStrawHit", 1000);
    if (fDimens == 1)
        ioman->Register("MuchStrawHit", "Straw hit in MUCH", fHits, kTRUE);
    else
        ioman->Register("MuchStrawHit", "Straw hit in MUCH", fHits, kFALSE);
    //} else fHits = (TClonesArray*) ioman->GetObject("MuchPixelHit");

    return kSUCCESS;
}

// -------------------------------------------------------------------------
void CbmMuchFindHitsStraws::Exec(Option_t* opt)
{

  if (fHits) fHits->Delete();

    static Int_t first = 1;
    static Double_t radIn[6];
    Double_t diam[6] = { 0.62, 0.62, 0.62, 0.62, 0.62, 0.62 }; // tube diameters
    Double_t sigmaX = 0.02, sigmaY = 2.0;                      // 200um, 2cm
    const Double_t sigmaBin = diam[0] / TMath::Sqrt(12.);

    Double_t phi[3] = { fPhis[0] * TMath::DegToRad(), fPhis[1] * TMath::DegToRad(), fPhis[2] * TMath::DegToRad() }; // rotation angles of views (doublets)

    if (first)
    {
        // Some initialization - should go somewhere else
        cout << " Processing straws ... " << endl;
        first = 0;
        // Get inner radia of stations
        Int_t nSt = fGeoScheme->GetNStations();
        for (Int_t i = 0; i < nSt; ++i)
        {
            CbmMuchStation* st = fGeoScheme->GetStation(i);
            radIn[i] = st->GetRmin();
            CbmMuchModule* mod = fGeoScheme->GetModule(i, 0, 0, 0);
            cout << i << " " << radIn[i] << " " << mod->GetSize().X() << " " << mod->GetSize().Y() << " " << mod->GetSize().Z() << " " << mod->GetDetectorType()
                 << endl;
            if (mod->GetDetectorType() == 2)
                assert(TMath::Abs(mod->GetSize().Z() - diam[i]) < 0.1); // tube diameter
        }
    }

    Int_t iarray[5] = { 0 }, nDigis = fDigis->GetEntriesFast(), nHits = fHits->GetEntriesFast();
    TVector3 pos, dpos;
    Double_t xyz[3] = { 0 }, array[5] = { 0 };
    for (Int_t idig = 0; idig < nDigis; ++idig)
    {
        CbmMuchStrawDigi* digi = (CbmMuchStrawDigi*)fDigis->UncheckedAt(idig);
        Int_t detId = digi->GetAddress();
        CbmMuchModule* module = fGeoScheme->GetModuleByDetId(detId);
        if (fDimens == 1)
        {
            xyz[0] = digi->GetX();
            xyz[1] = digi->GetY();
            xyz[2] = digi->GetZ();
        }
        else
        {
            if (module->GetDetectorType() != 2)
                continue; // skip GEMs

            //Int_t stationId = CbmMuchAddress::GetStationIndex(detId);
            //if (stationId < 4) continue;

            CbmMuchDigiMatch* digiM = (CbmMuchDigiMatch*)fDigiMatches->UncheckedAt(idig);
            CbmMuchPoint* point = (CbmMuchPoint*)fPoints->UncheckedAt(digiM->GetLink(0).GetIndex());
            xyz[0] = (point->GetXIn() + point->GetXOut()) / 2.;
            xyz[1] = (point->GetYIn() + point->GetYOut()) / 2.;
            xyz[2] = (point->GetZIn() + point->GetZOut()) / 2.;
        }
        // cout << detId << " " << module << " " << module->GetDetectorType() << endl;

        UInt_t address = CbmMuchAddress::GetElementAddress(digi->GetAddress(), kMuchModule);
        Int_t station3 = CbmMuchAddress::GetStationIndex(address);
        Int_t layer = CbmMuchAddress::GetLayerIndex(address);
        Int_t side = CbmMuchAddress::GetLayerSideIndex(address);
        Int_t rot = layer % 3;
        Double_t cosPhi = TMath::Cos(phi[rot]);
        Double_t sinPhi = TMath::Sin(phi[rot]);
        Double_t plocX = xyz[0] * cosPhi + xyz[1] * sinPhi;
        Double_t plocY = -xyz[0] * sinPhi + xyz[1] * cosPhi;
        //    std::cout << "phi=" << phi[rot] << " x=" << xyz[0] << " y=" << xyz[1] << " z=" << xyz[2] << " plocX=" << plocX << " plocY=" << plocY << std::endl;
        Double_t xloc = plocX;
        // cout << " Local: " << ploc.getX() << " " << ploc.getY() << " " << ploc.getZ() << endl;
        if (side)
            xloc += diam[station3] / 2.; // half-tube shift
        Int_t itube = (Int_t)(xloc / diam[station3]), iSegment;
        if (xloc < 0)
            itube--;
        Double_t xwire = (itube + 0.5) * diam[station3]; // wire position
        // cout << itube << " " << layer << " " << side << " " << xloc << " " << xwire << endl;
        Double_t times[3] = { 0 };

        /* No segmentation outside beam hole region
        if (TMath::Abs(plocX) < radIn[station3]) {
        if (plocY > 0) iSegment = 1;
        else iSegment = -1;
        } else iSegment = 0;
        */
        if (plocY > 0)
            iSegment = 1;
        else
            iSegment = -1;

        // Global coordinates in rotated coordinate system
        Double_t errU = gRandom->Gaus(0, sigmaX);
        // Double_t wXY = TMath::Sin(phi[rot]);
        // cout << station3 << " " << layer << " " << " " << wXY << endl;

        pos.SetXYZ(xyz[0], xyz[1], xyz[2]);
        dpos.SetXYZ(sigmaX, sigmaY, 0.);

        iarray[0] = station3;
        iarray[1] = itube;
        iarray[2] = iSegment;
        array[0] = xwire - diam[station3] / 2. * side; // Xwire
        array[1] = xloc - xwire;                       // drift distance
        array[2] = array[1] + errU;                    // drift distance with error
        Double_t locX = plocX + errU, locY = plocY;

        CbmMuchStrawHit* hit = NULL;
        if (fDimens == 1)
        {
            // 1 - dim. hits
            // CbmMuchStrawHit(detectorId,u,phi,z,du,dphi,dz,refId,planeId);
            // cout << " Local: " << ploc.getX()+errU << " " << ploc.getY() << " " << ploc.getZ() << endl;
            hit = new ((*fHits)[nHits++]) CbmMuchStrawHit(address,
                                                          // plocX + errU, TMath::ASin(wXY), pos[2], sigmaX, 0, 0,
                                                          locX,
                                                          phi[rot],
                                                          pos[2],
                                                          sigmaX,
                                                          0,
                                                          0,
                                                          idig,
                                                          fGeoScheme->GetLayerSideNr(address));
            hit->SetX(pos[0]);
            hit->SetY(pos[1]);
            hit->SetTube(itube);
            hit->SetSegment(iSegment);
            if (fBinary)
            {
                hit->SetU(array[0]);
                hit->SetDu(sigmaBin);
            }
        }
        else
        {
            // 2 - dim. hits
            // CbmMuchPixelHit(Int_t address,const TVector3& pos,const TVector3& err,Double_t dxy,Int_t refId,Int_t planeId);
            // CbmMuchPixelHit(address,x,y,z,dx,dy,0,dxy,iCluster,planeId,t,dt);
            // CbmMuchPixelHit *hit = new ((*fHits)[nHits++]) CbmMuchPixelHit(address, pos, dpos, dxy,
            //                       idig, fGeoScheme->GetLayerSideNr(address));
            Double_t errV = gRandom->Gaus(0, sigmaY);
            locY += errV;
            Double_t sigX = sigmaX;
            if (fBinary)
            {
                locX = array[0];
                sigX = sigmaBin;
            }
            Double_t globX = locX * cosPhi - locY * sinPhi;
            Double_t globY = locX * sinPhi + locY * cosPhi;
            Double_t dx, dy, dxy;
            ComputeErrors(phi[rot], cosPhi, sinPhi, sigX, sigmaY, dx, dy, dxy);
            hit = new ((*fHits)[nHits++]) CbmMuchStrawHit(address,
                                                          // plocX + errU, TMath::ASin(wXY), pos[2], sigmaX, 0, 0,
                                                          globX,
                                                          globY,
                                                          pos[2],
                                                          dx,
                                                          dy,
                                                          dxy,
                                                          idig,
                                                          fGeoScheme->GetLayerSideNr(address));
            // CbmMuchPixelHit *hit = new ((*fHits)[nHits++])
            // CbmMuchPixelHit(address, globX, globY, pos.Z(), dx, dy, 0.0, dxy, idig,
            //		  fGeoScheme->GetLayerSideNr(address), 0.0, 0.0);
        }
        hit->SetDouble(3, array);
        hit->SetInt(3, iarray);
        hit->SetFlag(0);
    }

    // Apply inefficiency (currently inside tube walls)
    if (fEffic)
        Effic(diam);
    // Merge hits inside the same tube
    if (fMerge)
        Merge();
    // Add mirror hits (left/right ambiguity)
    if (fMirror)
        Mirror();
    // Create pixel hits and store them in the pixel container
    if (fDimens == 2)
        StorePixels();

    static Int_t eventNo = 0;
    LOG(INFO) << "CbmMuchFindHitsStraws::Exec eventNo=" << eventNo << " hits=" << fHits->GetEntriesFast() << " digis=" << fDigis->GetEntriesFast()
              << FairLogger::endl;
}

// -------------------------------------------------------------------------
void CbmMuchFindHitsStraws::ComputeErrors(Double_t phi,
                                          Double_t cosPhi,
                                          Double_t sinPhi,
                                          Double_t sigX,
                                          Double_t sigmaY,
                                          Double_t& dx,
                                          Double_t& dy,
                                          Double_t& dxy)
{
    // Compute errors in global frame for 2-dimensional hits

    Double_t tanPhi = TMath::Tan(phi);
    // Take phi2 = phi + 90
    Double_t sin2 = cosPhi, cos2 = -sinPhi;
    Double_t r11 = sin2 / (cosPhi * sin2 - sinPhi * cos2);
    Double_t r12 = tanPhi / (tanPhi * cos2 - sin2);
    Double_t r21 = cos2 / (sinPhi * cos2 - cosPhi * sin2);
    Double_t r22 = 1. / (sin2 - tanPhi * cos2);
    dx = r11 * r11 * sigX * sigX + r12 * r12 * sigmaY * sigmaY;
    dy = r21 * r21 * sigX * sigX + r22 * r22 * sigmaY * sigmaY;
    dxy = r11 * r21 * sigX * sigX + r12 * r22 * sigmaY * sigmaY;
    dx = TMath::Sqrt(dx);
    dy = TMath::Sqrt(dy);
    //dxy = TMath::Sign(TMath::Sqrt(TMath::Abs(dxy)), dxy);
    //cout << phi << " " << dx << " " << dy << " " << dxy << endl;
}

// ---------   Private method Effic   -------------------------------
void CbmMuchFindHitsStraws::Effic(Double_t* diam)
{
    // Apply straw inefficiency (currently inside tube walls)

    Int_t nHits = fHits->GetEntriesFast();
    for (Int_t ihit = 0; ihit < nHits; ++ihit)
    {
        CbmMuchStrawHit* hit = (CbmMuchStrawHit*)fHits->UncheckedAt(ihit);

        // Apply inefficiency
        Double_t drift = hit->GetDouble()[1];
        Int_t station = hit->GetInt()[0];
        if (TMath::Abs(drift) < diam[station] / 2 - 0.01)
            continue; // outside tube wall
        fHits->RemoveAt(ihit);
    }
    fHits->Compress();
    LOG(INFO) << "CbmMuchFindHitsStraws::Effic hits=" << nHits << " corrected hits=" << fHits->GetEntriesFast() << FairLogger::endl;
}

// ---------   Private method Merge   -----------------------------
void CbmMuchFindHitsStraws::Merge()
{
    // Merge hits inside the same tube
    Int_t nHits = fHits->GetEntriesFast();
    for (Int_t ihit = 0; ihit < nHits; ++ihit)
    {
        CbmMuchStrawHit* hit = (CbmMuchStrawHit*)fHits->UncheckedAt(ihit);
        if (hit == 0x0)
            continue;
        Int_t tube = hit->GetInt()[1];
        Int_t segm = hit->GetInt()[2];
        Int_t addr = hit->GetAddress();
        Double_t drift = TMath::Abs(hit->GetDouble()[1]);
        CbmMuchDigiMatch* digiM = (CbmMuchDigiMatch*)fDigiMatches->UncheckedAt(hit->GetRefId());
        // CbmMuchDigiMatch *digiM = (CbmMuchDigiMatch*) fDigiMatches->UncheckedAt(ihit);

        for (Int_t jhit = ihit + 1; jhit < nHits; ++jhit)
        {
            CbmMuchStrawHit* hit1 = (CbmMuchStrawHit*)fHits->UncheckedAt(jhit);
            if (hit1 == 0x0)
                continue;
            Int_t tube1 = hit1->GetInt()[1];
            Int_t segm1 = hit1->GetInt()[2];
            Int_t addr1 = hit1->GetAddress();

            if (tube1 != tube)
                continue; // different tubes
            if (addr1 != addr)
                continue; // different layers
            if (segm1 != segm)
                continue; // upper and lower tubes

            Double_t drift1 = TMath::Abs(hit1->GetDouble()[1]);
            CbmMuchDigiMatch* digiM1 = (CbmMuchDigiMatch*)fDigiMatches->UncheckedAt(hit1->GetRefId());
            // CbmMuchDigiMatch *digiM1 = (CbmMuchDigiMatch*) fDigiMatches->UncheckedAt(jhit);
            // cout << jhit << " " << hit1->GetRefId() << endl;
            if (drift < drift1)
            {
                fHits->RemoveAt(jhit);
                Int_t nPoints = digiM1->GetNofLinks();
                for (Int_t j = 0; j < nPoints; ++j)
                {
                    hit->SetFlag(hit->GetFlag() + (1 << 1));  // increase overlap multiplicity
                    digiM->AddLink(digiM1->GetMatchedLink()); // add point
                }
            }
            else
            {
                fHits->RemoveAt(ihit);
                Int_t nPoints = digiM->GetNofLinks();
                for (Int_t j = 0; j < nPoints; ++j)
                {
                    hit1->SetFlag(hit1->GetFlag() + (1 << 1));
                    digiM1->AddLink(digiM->GetMatchedLink());
                }
                break;
            }
        }
    }
    fHits->Compress();
    LOG(INFO) << "CbmMuchFindHitsStraws::Merge hits=" << nHits << " merged hits=" << fHits->GetEntriesFast() << FairLogger::endl;
}

// -------------------------------------------------------------------------
void CbmMuchFindHitsStraws::Mirror()
{
    // Add mirror hits (left/right ambiguity)

    Double_t phi[3] = { fPhis[0] * TMath::DegToRad(), fPhis[1] * TMath::DegToRad(), fPhis[2] * TMath::DegToRad() }; // rotation angles of views (doublets)
    Int_t nHits0 = fHits->GetEntriesFast();
    Int_t nHits = nHits0;
    for (Int_t ihit = 0; ihit < nHits0; ++ihit)
    {
        CbmMuchStrawHit* hit = (CbmMuchStrawHit*)fHits->UncheckedAt(ihit);
        CbmMuchStrawHit* hitM = new ((*fHits)[nHits++]) CbmMuchStrawHit(*hit);
        // Add mirror hit
        // Double_t xwire = fDimens == 1 ? hit->GetDouble()[0] : hit2->GetDouble()[0];
        Double_t drift = hit->GetDouble()[2];
        if (fDimens == 1)
        {
            // 1-D hits
            hitM->SetU(hit->GetU() - 2 * drift);
            // cout << hit->GetMatch() << " " << hitM->GetMatch() << endl;
            // hitM->SetMatch(hit->GetMatch());
        }
        else
        {
            // 2-D hits
            // UInt_t address =  CbmMuchAddress::GetElementAddress(hit->GetAddress(),kMuchModule);
            UInt_t address = hit->GetAddress();
            Int_t layer = CbmMuchAddress::GetLayerIndex(address);
            Int_t rot = layer % 3;
            Double_t cosPhi = TMath::Cos(phi[rot]);
            Double_t sinPhi = TMath::Sin(phi[rot]);
            Double_t dx = drift * cosPhi, dy = drift * sinPhi;
            hitM->SetU(hit->GetU() - 2 * dx);
            hitM->SetPhi(hit->GetPhi() - 2 * dy);
            // cout << phi[rot] << " " << xwire << " " << hit2->GetX() << " " << drift << " " << hitM->GetX() << endl;
            // cout << hit2->GetY() << " " << drift << " " << hitM->GetY() << endl;
        }
        hitM->SetFlag(hitM->GetFlag() + 1); // flag mirror hit
    }
}

// -------------------------------------------------------------------------
void CbmMuchFindHitsStraws::StorePixels()
{
    // Create pixel hits and store them in the pixel container

    Int_t nofStrawHits = fHits->GetEntriesFast();
    if (nofStrawHits == 0)
        return;

    TClonesArray* pixHits = (TClonesArray*)FairRootManager::Instance()->GetObject("MuchPixelHit");
    Int_t nofPixelHits = pixHits->GetEntriesFast();
    Int_t nofClusters = fClusters->GetEntriesFast();

    for (Int_t ihit = 0; ihit < nofStrawHits; ++ihit)
    {
        CbmMuchStrawHit* strawHit = (CbmMuchStrawHit*)fHits->UncheckedAt(ihit);

        // Create cluster
        CbmMuchCluster* cluster = new ((*fClusters)[nofClusters]) CbmMuchCluster();
        cluster->AddDigi(strawHit->GetRefId());
        cluster->SetAddress(strawHit->GetAddress()); // correct or not - ???

        // Create pixel hit
        CbmMuchPixelHit* pixelHit = new ((*pixHits)[nofPixelHits]) CbmMuchPixelHit(strawHit->GetAddress(),
                                                                                   strawHit->GetU(),
                                                                                   strawHit->GetPhi(),
                                                                                   strawHit->GetZ(),
                                                                                   strawHit->GetDu(),
                                                                                   strawHit->GetDphi(),
                                                                                   0.0,
                                                                                   strawHit->GetDz(),
                                                                                   nofClusters,
                                                                                   fGeoScheme->GetLayerSideNr(strawHit->GetAddress()),
                                                                                   0.0,
                                                                                   0.0);
        pixelHit->SetFlag(strawHit->GetFlag());
        // CbmMuchPixelHit(address, globX, globY, pos.Z(), dx, dy, 0.0, dxy, idig,
        //		  fGeoScheme->GetLayerSideNr(address), 0.0, 0.0);

        nofClusters++;
        nofPixelHits++;
    }

    fHits->Delete();
}

ClassImp(CbmMuchFindHitsStraws)

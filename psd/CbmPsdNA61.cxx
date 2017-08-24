// -------------------------------------------------------------------------
// -----                       CbmPsdv1 source file                   -----
// -----                  Created 04/02/05  by Alla Maevskaya              -----
// -------------------------------------------------------------------------

#include "CbmPsdNA61.h"

#include "CbmPsdPoint.h"
#include "CbmStack.h"
#include "TClonesArray.h"
#include "TGeoManager.h"
#include "TParticle.h"
#include "TVirtualMC.h"
#include "TGeoBBox.h"

#include <iostream>
#include <fstream>

using std::cout;
using std::endl;
using std::ofstream;

// -----   Default constructor   -------------------------------------------
CbmPsdNA61::CbmPsdNA61() 
: CbmPsd(),
  fTrackID(0),
  fVolumeID(0),
  fModuleID(0),
  fPos(),
  fMom(),
  fTime(0.),
  fLength(0.),
  fEloss(0.),
  fPosIndex(0),
  fPsdCollection(new TClonesArray("CbmPsdPoint")),
  fXshift(0.),
  fZposition(0.),
  fRotYAngle(0.),
  fGeoFile("")
{
}
// -------------------------------------------------------------------------



// -----   Standard constructor   ------------------------------------------
CbmPsdNA61::CbmPsdNA61(const char* name, Bool_t active)
: CbmPsd(name, active),
  fTrackID(0),
  fVolumeID(0),
  fModuleID(0),
  fPos(),
  fMom(),
  fTime(0.),
  fLength(0.),
  fEloss(0.),
  fPosIndex(0),
  fPsdCollection(new TClonesArray("CbmPsdPoint")),
  fXshift(0.),
  fZposition(0.),
  fRotYAngle(0.),
  fGeoFile("")
{
}

// -------------------------------------------------------------------------



// -----   Destructor   ----------------------------------------------------
CbmPsdNA61::~CbmPsdNA61() {
 if (fPsdCollection) {
   fPsdCollection->Delete(); 
   delete fPsdCollection;
 }
}
// -------------------------------------------------------------------------



// -----   Public method ProcessHits  --------------------------------------
Bool_t  CbmPsdNA61::ProcessHits(FairVolume* /*vol*/)
{
  //  cout<<" CbmPsdNA61::ProcessHits in "<<vol->GetName()<<endl;
  if (TMath::Abs(gMC->TrackCharge()) <= 0) return kFALSE;

  Int_t pdg;
  // Set parameters at entrance of volume. Reset ELoss.
  
  if ( gMC->IsTrackEntering() ) 
  {
    ResetParameters();
    fEloss   = 0.;
    fTrackID = gMC->GetStack()->GetCurrentTrackNumber();
    fTime    = gMC->TrackTime() * 1.0e09;
    fLength  = gMC->TrackLength();
    gMC->TrackPosition(fPos);
    gMC->TrackMomentum(fMom);
    Int_t copyNo;
    gMC->CurrentVolID(copyNo);
    //fVolumeID=copyNo;
    Int_t iCell, iNumm ;
    gMC->CurrentVolOffID(2, iCell);  // SELIM: change 1 -> 2
    fModuleID=iCell;//module
    gMC->CurrentVolOffID(1, iNumm);  // SELIM: change 2 -> 1
    fVolumeID=iNumm;//scint
    pdg = gMC->TrackPid();
  }
  
  if (gMC->IsTrackInside() ) 
  {
    // Sum energy loss for all steps in the active volume
    fEloss += gMC->Edep();
    
    if( gMC->IsTrackStop() ||
	gMC->IsTrackDisappeared()   ) 
    {
      fEloss += gMC->Edep();
      
      if (pdg == 1000791970)
      // if (strstr(fDebug,"hit"))
      Info("ProcessHits track inside","track %d pdg %d volume %d %s, x=(%.1f,%.1f,%.1f), p=(%.2e,%.2e,%.2e), time %f fLength %f dE=%.3e",
	     fTrackID, pdg, fVolumeID, gMC->CurrentVolName(),
	     fPos.X(),fPos.Y(),fPos.Z(),fMom.Px(),fMom.Py(),fMom.Pz(),fTime, fLength, fEloss);
      
      // Create CbmPsdPoint
      CbmPsdPoint *fPoint =  AddHit(fTrackID, fVolumeID, TVector3(fPos.X(),  fPos.Y(),  fPos.Z()),
				    TVector3(fMom.Px(), fMom.Py(), fMom.Pz()),fTime, fLength,fEloss);
      fPoint->SetModuleID(fModuleID);
      fPoint->SetTrackID(fTrackID);
    }
  }
  
  if ( gMC->IsTrackExiting() /*   ||
       gMC->IsTrackStop()       ||
       gMC->IsTrackDisappeared() */  ) 
  {
     // Sum energy loss for all steps in the active volume
    fEloss += gMC->Edep();    
   
    if (pdg == 1000791970)
      //    if (strstr(fDebug,"hit"))
    Info("ProcessHits","track %d pdg %d volume %d %s, x=(%.1f,%.1f,%.1f), p=(%.2e,%.2e,%.2e), time %f fLength %f dE=%.3e",
	   fTrackID, pdg, fVolumeID, gMC->CurrentVolName(),
	   fPos.X(),fPos.Y(),fPos.Z(),fMom.Px(),fMom.Py(),fMom.Pz(),fTime, fLength, fEloss);
    
    // Create CbmPsdPoint
    CbmPsdPoint *fPoint =  AddHit(fTrackID, fVolumeID, TVector3(fPos.X(),  fPos.Y(),  fPos.Z()),
				  TVector3(fMom.Px(), fMom.Py(), fMom.Pz()),fTime, fLength,fEloss);
    fPoint->SetModuleID(fModuleID);
    fPoint->SetTrackID(fTrackID);
  }
  
  ((CbmStack*)gMC->GetStack())->AddPoint(kPsd, fTrackID);
 
  //  ResetParameters();
    
  return kTRUE;
}
// -------------------------------------------------------------------------



// -----   Public method CreateMaterial   ----------------------------------
void CbmPsdNA61::CreateMaterial() {
    
    // Create materials, media and volume
    //-----------List of Materials and Mixtures--------------

    TGeoMaterial *matIRON = new TGeoMaterial("IRON",55.85,26,7.87);
    matIRON->SetUniqueID(  10);
    TGeoMaterial *matLEAD = new TGeoMaterial("LEAD",207.19,82,11.35);
    matLEAD->SetUniqueID(  13);
    TGeoMaterial *matAIR = new TGeoMaterial("AIR",14.61,7.3,0.1205000E-02);
    matAIR->SetUniqueID(  15);
    TGeoMaterial *matVAC = new TGeoMaterial("VACUUM",0,0,0);
    matVAC->SetUniqueID(  16);
    TGeoMaterial *matSIL = new TGeoMaterial("SILICON",28.09,14,2.33);
    matSIL->SetUniqueID(  20);
    TGeoMaterial *matHE = new TGeoMaterial("HE_GAS",4,2,0.1780000E-03);
    matHE->SetUniqueID(  24);
    TGeoMaterial *matAl = new TGeoMaterial("Al", 26.98,13,2.7);
    
    TGeoMixture *matPLASTIC = new TGeoMixture("PLASTIC",2,   1.03200    );
    matPLASTIC->SetUniqueID(  25);
    matPLASTIC->DefineElement(0,12.01,6,0.9225687);
    matPLASTIC->DefineElement(1,1.008,1,0.7743125E-01);
    
    TGeoMixture *matTYVEC = new TGeoMixture("TYVEC",2,  0.930000    );
    matTYVEC->SetUniqueID(  38);
    matTYVEC->DefineElement(0,12.011,6,0.8562772);
    matTYVEC->DefineElement(1,1.008,1,0.1437228);    
    
}
// -----   Public method ConstructGeometry   -------------------------------
void CbmPsdNA61::CreateModule (TGeoVolume *module, Float_t half_modul_size_x,  Float_t half_modul_size_y, Int_t n_sec, 
                               bool isHole, Int_t hole_size, Int_t hole_pos, 
                               bool is45mod)

{
    
    CreateMaterial();
    //-----------List of Tracking Media--------------

    TGeoMedium *medAIR = new TGeoMedium("AIR",      1,15,0,1,0.19,0.25,-1,-1,0.1000000E-02,-1);
    TGeoMedium *medVAC = new TGeoMedium("VACUUM",   2,16,0,1,0.19,0.25,-1,-1,0.1000000E-02,-1);
    TGeoMedium *medIRON = new TGeoMedium("IRON",     9,10,0,1,0.19,1,-1,-1,0.1,-1);
    TGeoMedium *medSCINT = new TGeoMedium("PLASTIC",11,25,1,0,0,1,-1,-1,0.1000000E-02,-1);
    TGeoMedium *medLEAD = new TGeoMedium("LEAD",   24,13,0,0,0,1,-1,-1,0.1000000E-02,-1);
    TGeoMedium *medFIBER = new TGeoMedium("PLASTIC",27,25,0,1,0.19,1,-1,-1,0.1000000E-02,-1);
    TGeoMedium *medTYVEC = new TGeoMedium("TYVEC",  32,38,0,0,0,1,-1,-1,0.1000000E-02,-1);

    TGeoMaterial *matAl = new TGeoMaterial("Al", 26.98,13,2.7);    
    TGeoMedium *medAl = new TGeoMedium("Al", 3, matAl);

    Float_t *buf = 0;

    Float_t tyvec_thick = 0.02;
    Float_t iron_thick_x = 0.1;
    Float_t iron_thick_y = 0.15;
    Float_t half_scint_size_z = 0.2;
    Float_t half_lead_thick = 0.8;
    Float_t iron_front_back_half_thick = 1.0;
    if (is45mod) iron_front_back_half_thick = 0.75/2;

    Float_t half_scint_size_x = half_modul_size_x - iron_thick_x - tyvec_thick;
    Float_t half_scint_size_y = half_modul_size_y - iron_thick_y - tyvec_thick;    
    Float_t half_modul_size_z = (half_scint_size_z + half_lead_thick + tyvec_thick)*n_sec - half_lead_thick + iron_front_back_half_thick*2; //  62.4; //cm
    Float_t sec_thick = 2*(half_lead_thick + half_scint_size_z + tyvec_thick);
    
    
    TGeoVolume *iron_front_back = gGeoManager->MakeBox("iron_front_back",medIRON, half_scint_size_x + tyvec_thick,
                                                                                    half_scint_size_y + tyvec_thick, 
                                                                                    iron_front_back_half_thick   );
        
    TGeoVolume *lead_layer = gGeoManager->MakeBox("lead_layer",medLEAD, half_scint_size_x + tyvec_thick, 
                                                                        half_scint_size_y + tyvec_thick, 
                                                                        half_lead_thick);

    TGeoVolume *tyvec = gGeoManager->MakeBox("tyvec", medTYVEC, half_scint_size_x + tyvec_thick, 
                                                                half_scint_size_y + tyvec_thick, 
                                                                half_scint_size_z + tyvec_thick);

    TGeoVolume *scint = gGeoManager->MakeBox("scint",medSCINT, half_scint_size_x, 
                                                                    half_scint_size_y, 
                                                                    half_scint_size_z);

    AddSensitiveVolume(scint);   //NOTE

    //HOLES (TUBS) in different volumes 
    if (isHole){

        Float_t half_hole_size = hole_size/2;
        Float_t hole_size_scint = half_hole_size - iron_thick_x - tyvec_thick;
        Float_t hole_size_tyvec = half_hole_size - iron_thick_x;
        
        Float_t phi_1 = 90.0*hole_pos;
        Float_t phi_2 = 90.0*(hole_pos+1);
        
                
        TGeoVolume *iron_front_back_hole = gGeoManager -> MakeTubs("iron_front_back_hole", medAIR, 0., hole_size_tyvec,  iron_front_back_half_thick,   phi_1, phi_2); //in Fe 
        TGeoVolume *lead_layer_hole      = gGeoManager -> MakeTubs("lead_layer_hole",      medAIR, 0., hole_size_tyvec,  half_lead_thick,              phi_1, phi_2); //in Pb
        TGeoVolume *tyvec_hole           = gGeoManager -> MakeTubs("tyvec_hole",           medAIR, 0., hole_size_tyvec,  tyvec_thick,                  phi_1, phi_2); //in Tyvec
        TGeoVolume *scint_hole           = gGeoManager -> MakeTubs("scint_hole",           medAIR, 0., hole_size_scint,  half_scint_size_z,            phi_1, phi_2); //in Scin.

        iron_front_back_hole -> SetLineColor(kRed+3);
        lead_layer_hole      -> SetLineColor(kRed+3);
        tyvec_hole           -> SetLineColor(kRed+3);
        scint_hole           -> SetLineColor(kRed+3);
  
        Int_t sx, sy;
        switch (hole_pos)
        {
            case 0:  sx = -1;    sy = -1;   break;
            case 1:  sx =  1;    sy = -1;   break;
            case 2:  sx =  1;    sy =  1;   break;
            case 3:  sx = -1;    sy =  1;   break;
        }
        
        
        // module  with hole
        iron_front_back -> AddNode(iron_front_back_hole, 1, new TGeoTranslation(sx*(half_scint_size_x+tyvec_thick), sy*(half_scint_size_y+tyvec_thick), 0.));
        
        tyvec  -> AddNode(tyvec_hole, 1, new TGeoTranslation(sx*(half_scint_size_x+tyvec_thick),  sy*(half_scint_size_y+tyvec_thick), -0.21));
        tyvec  -> AddNode(tyvec_hole, 2, new TGeoTranslation(sx*(half_scint_size_x - tyvec_thick),  sy*(half_scint_size_y+tyvec_thick), 0.21));
        
        scint->AddNode(scint_hole, 1, new TGeoTranslation(sx*half_scint_size_x, sy*half_scint_size_y, 0.));
        
        lead_layer->AddNode(lead_layer_hole, 1, new TGeoTranslation(sx*(half_scint_size_x+tyvec_thick), sy*(half_scint_size_y+tyvec_thick), 0.));

        //HOLES (BOX)   
        TGeoVolume *iron_hole_y = gGeoManager->MakeBox("iron_hole_y",medVAC, iron_thick_x/2,                  half_hole_size/2, 62.4); //vertical hole for VMHA
        TGeoVolume *iron_hole_x = gGeoManager->MakeBox("iron_hole_x",medVAC, half_hole_size/2-iron_thick_x/2, iron_thick_y/2,   62.4); //horizontal hole for VMHA

        //Horizontal and vertical holes in module
        module->AddNode(iron_hole_y,1,new TGeoTranslation( sx*(half_modul_size_x-iron_thick_x/2),                  sy*(half_modul_size_x-half_hole_size/2), 0.));
        module->AddNode(iron_hole_x,1,new TGeoTranslation( sx*(half_modul_size_y-half_hole_size/2-iron_thick_x/2), sy*(half_modul_size_y-iron_thick_y/2),   0.));

        //HOLES (BOX)   
        TGeoVolume *tyvec_hole_y = gGeoManager->MakeBox("tyvec_hole_y",medVAC, 0.01, (half_hole_size/2-iron_thick_y/2), 0.2); //vertical hole for Tyvec
        TGeoVolume *tyvec_hole_x = gGeoManager->MakeBox("tyvec_hole_x",medVAC, (half_hole_size/2-iron_thick_x/2-tyvec_thick/2), 0.01, 0.2); //horizontal hole for Tyvec

        Float_t tyvec_hole_pos_1 = half_modul_size_x-iron_thick_x/2-tyvec_thick/2;
        Float_t tyvec_hole_pos_2 = half_modul_size_y-iron_thick_y/2-tyvec_thick/2;

        //Horizontal and vertical holes in Tyvec (parts A,B,C,D) 
        tyvec->AddNode(tyvec_hole_y,1,new TGeoTranslation(sx*tyvec_hole_pos_1,               sy*(tyvec_hole_pos_2+half_hole_size/2), 0.));
        tyvec->AddNode(tyvec_hole_x,1,new TGeoTranslation(sx*(tyvec_hole_pos_2+tyvec_thick/2+half_hole_size/2), sy*tyvec_hole_pos_1, 0.));
     
        iron_front_back->SetLineColor(kGreen);
        lead_layer->SetLineColor(kGreen);
        tyvec->SetLineColor(kGreen);    
        scint->SetLineColor(kGreen);
        
    }
    else {
        Float_t scale = half_modul_size_x/10.;
        TGeoVolume *fiber_in_iron = gGeoManager->MakeBox("fiber_in_iron",medFIBER, 0.1, 8.6*scale, 1);
        TGeoVolume *fiber_in_lead = gGeoManager->MakeBox("fiber_in_lead",medFIBER, 0.1, 8.6*scale, 0.8);
        TGeoVolume *VRYL = gGeoManager->MakeBox("VRYL",medFIBER, 0.01, 8.6*scale, 0.22);
        TGeoVolume *VRAL = gGeoManager->MakeBox("VRAL",medFIBER, 0.09, 8.6*scale, 0.01);
        TGeoVolume *VRSL = gGeoManager->MakeBox("VRSL",medFIBER, 0.09, 8.6*scale, 0.2);    
    
        iron_front_back -> AddNode( fiber_in_iron, 1, new TGeoTranslation(9.8*scale, 0.75, 0) );        
        tyvec -> AddNode( VRYL, 1, new TGeoTranslation(9.89*scale, 0.75, 0) );
        tyvec -> AddNode( VRAL, 1, new TGeoTranslation(9.79*scale, 0.75, -0.21) );
        tyvec -> AddNode( VRAL, 2, new TGeoTranslation(9.79*scale, 0.75, 0.21) );
        scint -> AddNode( VRSL, 1, new TGeoTranslation(9.79*scale, 0.75, 0.) );
        lead_layer -> AddNode( fiber_in_lead, 1, new TGeoTranslation(9.8*scale, 0.75, 0.) );
    }
    
    if (!is45mod){
        module -> AddNode(iron_front_back, 1, new TGeoTranslation(0, 0, -(half_modul_size_z-iron_front_back_half_thick)));
        module -> AddNode(iron_front_back, 2, new TGeoTranslation(0, 0, half_modul_size_z-iron_front_back_half_thick));
    }
    else
    {
        TGeoVolume *al_front = gGeoManager->MakeBox("al_front",medAl, half_scint_size_x + tyvec_thick,
                                                                      half_scint_size_y + tyvec_thick, 
                                                                      0.5   );
        TGeoVolume *al_back = gGeoManager->MakeBox("al_back",medAl, half_scint_size_x + tyvec_thick,
                                                                    half_scint_size_y + tyvec_thick, 
                                                                    0.25   );
        module -> AddNode(al_front, 1, new TGeoTranslation(0, 0, -(half_modul_size_z-0.5)));
        module -> AddNode(al_back, 2, new TGeoTranslation(0, 0, half_modul_size_z-0.75));
        
        
        
    }
    
    tyvec->AddNode(scint, 1, gGeoIdentity);
    
    Float_t TEMP = -half_modul_size_z + 2*iron_front_back_half_thick + half_scint_size_z + tyvec_thick;
    if (is45mod) TEMP += 0.25;
    
    for (Int_t ivol=1; ivol<=n_sec; ivol++)
    {
        Float_t zvtyl = TEMP + sec_thick*(ivol-1); 
        module -> AddNode(tyvec, ivol, new TGeoTranslation(0, 0, zvtyl));
        fNbOfSensitiveVol++;
    }    
    TEMP += half_scint_size_z + tyvec_thick + half_lead_thick;
    for (Int_t ivol=1; ivol<n_sec; ivol++)
    {
        Float_t zvpbl = TEMP + sec_thick*(ivol-1); 
        module->AddNode(lead_layer, ivol+1, new TGeoTranslation(0, 0, zvpbl));
    }    
        
}




void CbmPsdNA61::ConstructGeometry() {
  // Create materials, media and volume
  
    Float_t xPSD = fXshift;
    Float_t zPSD = fZposition;
    Float_t rotPSD = fRotYAngle; //in deg.//marina

    Float_t *buf = 0;
    Float_t zdcSize[3]   = {  70.,   70.,  62.4 }; // SELIM: fZposition = 1 scintillator, not PSD center
    zPSD += zdcSize[2];
    
    TGeoRotation *rot = new TGeoRotation();
    rot->RotateY(rotPSD);    

//     Float_t hole_size = 6.0;      
    
    CreateMaterial();
    TGeoMaterial *matAl = new TGeoMaterial("Al", 26.98,13,2.7);    
    TGeoMedium *medAl = new TGeoMedium("Al", 3, matAl);

    //-----------List of Tracking Media--------------
    TGeoMedium *medAIR = new TGeoMedium("AIR",      1,15,0,1,0.19,0.25,-1,-1,0.1000000E-02,-1);
    TGeoMedium *medIRON = new TGeoMedium("IRON",     9,10,0,1,0.19,1,-1,-1,0.1,-1);
    
    Float_t half_modul_size_x = 10.; //cm
    Float_t half_modul_size_y = 10.; //cm
    Float_t half_modul_size_z = 62.4; //cm
    
    TGeoVolume *PSD = gGeoManager->MakeBox("PSD", medAIR, 70, 70, half_modul_size_z+20);

    TGeoVolume *large_module = gGeoManager->MakeBox("large_module",medIRON, half_modul_size_x,   half_modul_size_y,   half_modul_size_z);
    CreateModule (large_module);

    TGeoVolume *small_module = gGeoManager->MakeBox("small_module",medIRON,  5.0,   5.0,   half_modul_size_z);
    CreateModule (small_module, 5., 5.);

    TGeoVolume *module_45 = gGeoManager->MakeBox("module_45",medIRON,  5.0,   5.0,   (25.98-1.6)/2);
    CreateModule (module_45, 5., 5., 12, false, 0, 0, true);

    
    gGeoManager->GetVolume("cave")->AddNode(PSD, 1, new TGeoCombiTrans(xPSD, 0., zPSD, rot));//marina
   
    //NA61 geometry
    Float_t xi[100], yi[100];
    Int_t iMod=0, iModNoHole=0;
    ofstream fxypos(fGeoFile);
//     ofstream fxypos("psd_geo_xy_test.txt");   

    Float_t xCur=-15., yCur=-15.;    
    for(Int_t iy=0; iy<4; iy++) 
    {
        for(Int_t ix=0; ix<4; ix++) 
        {
            iMod++;
            xi[iMod-1] = xCur + xPSD;
            yi[iMod-1] = yCur;  // change: put fxzpos << etc here
            fxypos << xi[iMod-1]<< " " << yi[iMod-1] << endl;
            gGeoManager->Node("small_module", iMod, "PSD", xCur, yCur, 0, 0, kTRUE, buf, 0); 
            xCur += 10.;      
        }
        xCur = -15;
        yCur += 10.;
    }
    
    xCur=-50.; 
    yCur=-50.; 
    for(Int_t iy=0; iy<6; iy++) 
    {
        for(Int_t ix=0; ix<6; ix++) 
        {
            bool isModule = ( TMath::Abs(xCur) > 15 || TMath::Abs(yCur) > 15 ) && ( TMath::Abs(yCur*xCur) < 2500 );
            if (isModule)
            {
                iMod++;
                xi[iMod-1] = xCur + xPSD;
                yi[iMod-1] = yCur;  // change: put fxzpos << etc here
                fxypos << xi[iMod-1]<< " " << yi[iMod-1] << endl;
                gGeoManager->Node("large_module", iMod, "PSD", xCur, yCur, 0, 0, kTRUE, buf, 0); 
                cout << xi[iMod-1] << " " << yi[iMod-1] << endl;

            }
            xCur += 20.;      
        }
        xCur = -50;
        yCur += 20.;
    
    }
    gGeoManager->Node("module_45", iMod+1, "PSD", 0, 0, -(25.98-1.6)/2-0.5-half_modul_size_z, 0, kTRUE, buf, 0); 
    TGeoVolume *al_support = gGeoManager->MakeBox("al_support",medAl,  7.5, 1.25, 13. );
    gGeoManager->Node("al_support", iMod+2, "PSD", 0, -5.-1.25/2, -13-0.5-half_modul_size_z, 0, kTRUE, buf, 0); 
    
    fxypos << 0.0 << " " << 0.0 << endl;

    cout <<"fNbOfSensitiveVol " <<fNbOfSensitiveVol <<endl;
    fxypos.close();

}
// -------------------------------------------------------------------------

ClassImp(CbmPsdNA61)

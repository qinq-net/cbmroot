/******************************************************************************
 ** Creation of STS geometry in ROOT format (TGeo).
 **
 ** @file create_stsgeo_v17a.C
 ** @author Volker Friese <v.friese@gsi.de>
 ** @since 15 June 2012
 ** @date 09.05.2014
 ** @author Tomas Balog <T.Balog@gsi.de>
 **
 ** v16c: like v16b, but senors of ladders beampipe next to beampipe
 **       shifted closer to the pipe, like in the CAD model
 ** v16b: like v16a, but yellow sensors removed
 ** v16a: derived from v15c (no cones), but with sensor types renamed:
 ** 2 -> 1, 3 -> 2, 4 -> 3, 5 -> 4, 1 -> 5
 **
 ** v15c: as v15b without cones
 ** v15b: introduce modified carbon ladders from v13z
 ** v15a: with flipped ladder orientation for stations 0,2,4,6 to match CAD design
 **
 ** TODO:
 **
 ** DONE:
 ** v15b - use carbon macaroni as ladder support
 ** v15b - introduce a small gap between lowest sensor and carbon ladder
 ** v15b - build small cones for the first 2 stations
 ** v15b - within a station the ladders of adjacent units should not touch eachother - set gkLadderGapZ to 10 mm
 ** v15b - for all ladders set an even number of ladder elements 
 ** v15b - z offset of cones to ladders should not be 0.3 by default, but 0.26
 ** v15b - within a station the ladders should be aligned in z, defined either by the unit or the ladder with most sensors
 ** v15b - get rid of cone overlap in stations 7 and 8 - done by adapting rHole size
 **
 ** The geometry hierarachy is:
 **
 ** 1. Sensors  (see function CreateSensors)
 **    The sensors are the active volumes and the lowest geometry level.
 **    They are built as TGeoVolumes, shape box, material silicon.
 **    x size is determined by strip pitch 58 mu and 1024 strips 
 **    plus guard ring of 1.3 mm at each border -> 6.1992 cm.
 **    Sensor type 1 is half of that (3.0792 cm).
 **    y size is determined by strip length (2.2 / 4.2 / 6.3 cm) plus
 **    guard ring of 1.3 mm at top and bottom -> 2.46 / 4.46 / 6.46 cm.
 **    z size is a parameter, to be set by gkSensorThickness.
 **
 ** 2. Sectors  (see function CreateSectors)
 **    Sectors consist of several chained sensors. These are arranged
 **    vertically on top of each other with a gap to be set by
 **    gkChainGapY. Sectors are constructed as TGeoVolumeAssembly.
 **    The sectors are auxiliary volumes used for proper placement
 **    of the sensor(s) in the module. They do not show up in the
 **    final geometry.
 **
 ** 3. Modules (see function ConstructModule)
 **    A module is a readout unit, consisting of one sensor or
 **    a chain of sensors (see sector) and a cable.
 **    The cable extends from the top of the sector vertically to the
 **    top of the halfladder the module is placed in. The cable and module
 **    volume thus depend on the vertical position of the sector in 
 **    the halfladder. The cables consist of silicon with a thickness to be
 **    set by gkCableThickness.
 **    Modules are constructed as TGeoVolume, shape box, medium gStsMedium.
 **    The module construction can be switched off (gkConstructCables)
 **    to reproduce older geometries.
 **
 ** 4. Halfladders (see function ConstructHalfLadder)
 **    A halfladder is a vertical assembly of several modules. The modules
 **    are placed vertically such that their sectors overlap by 
 **    gkSectorOverlapY. They are displaced in z direction to allow for the 
 **    overlap in y by gkSectorGapZ.
 **    The horizontal placement of modules in the halfladder can be choosen
 **    to left aligned or right aligned, which only matters if sensors of
 **    different x size are involved.
 **    Halfladders are constructed as TGeoVolumeAssembly.
 **
 ** 5. Ladders (see function CreateLadders and ConstructLadder)
 **    A ladder is a vertical assembly of two halfladders, and is such the
 **    vertical building block of a station. The second (bottom) half ladder
 **    is rotated upside down. The vertical arrangement is such that the
 **    inner sectors of the two halfladders have the overlap gkSectorOverlapY
 **    (function CreateLadder) or that there is a vertical gap for the beam
 **    hole (function CreateLadderWithGap).
 **    Ladders are constructed as TGeoVolumeAssembly.
 **   
 ** 6. Stations (see function ConstructStation)
 **    A station represents one layer of the STS geometry: one measurement
 **    at (approximately) a given z position. It consist of several ladders
 **    arranged horizontally to cover the acceptance.
 **    The ladders are arranged such that there is a horizontal overlap
 **    between neighbouring ladders (gkLadderOverLapX) and a vertical gap
 **    to allow for this overlap (gkLadderGapZ). Each second ladder is
 **    rotated around its y axis to face away from or into the beam.
 **    Stations are constructed as TGeoVolumes, shape box minus tube (for
 **    the beam hole), material gStsMedium.
 **
 ** 7. STS
 **    The STS is a volume hosting the entire detectors system. It consists
 **    of several stations located at different z positions.
 **    The STS is constructed as TGeoVolume, shape box minus cone (for the
 **    beam pipe), material gStsMedium. The size of the box is computed to
 **    enclose all stations.
 *****************************************************************************/


// Remark: With the proper steering variables, this should exactly reproduce
// the geometry version v11b of A. Kotynia's described in the ASCII format.
// The only exception is a minimal difference in the z position of the 
// sectors/sensors. This is because of ladder types 2 and 4 containing the half
// sensors around the beam hole (stations 1,2 and 3). In v11b, the two ladders
// covering the beam hole cannot be transformed into each other by rotations,
// but only by a reflection. This means they are constructionally different.
// To avoid introducing another two ladder types, the difference in z position
// was accepted.





using std::cout;
using std::endl;
using std::setw;
using std::right;

// Forward declarations
TGeoVolume* ConstructModule(const char* name, Double_t moduleSize,
                            Int_t nLayers);


// ============================================================================
// ======                         Main function                           =====
// ============================================================================

void create_psdgeo()
{

  // -----   Steering variables   ---------------------------------------------
  const char* geoTag = "test";  // Geometry tag

  Double_t psdX       = 11.;    // x position of PSD in cave
  Double_t psdY       = 0.;     // y position of PSD in cave
  Double_t psdZ       = 800.;   // z position of PSD in cave
  Double_t psdRotY    = 0.;     // Rotation of PSD around y axis [degrees]
  Double_t moduleSize = 20.;    // Module size [cm]
  Int_t    nModulesX  = 7;      // Number of modules in a row (x direction)
  Int_t    nModulesY  = 7;      // Number of modules in a row (x direction)
  // --------------------------------------------------------------------------


  // ---- Number of modules per row/column must be uneven ---------------------
  // Otherwise, the central one (to be skipped) is not defined.
  if ( nModulesX % 2 != 1  || nModulesY % 2 != 1 ) {
    cout << "Error: number of modules per row and column must be uneven! "
        << nModulesX << " " << nModulesY << endl;
    return;
  }
  // --------------------------------------------------------------------------


  // -------   Load media from media file   -----------------------------------
  cout << endl << "==> Reading media..." << endl;
  FairGeoLoader*    geoLoad = new FairGeoLoader("TGeo","FairGeoLoader");
  FairGeoInterface* geoFace = geoLoad->getGeoInterface();
  TString geoPath = gSystem->Getenv("VMCWORKDIR");
  TString medFile = geoPath + "/geometry/media.geo";
  geoFace->setMediaFile(medFile);
  geoFace->readMedia();
  // --------------------------------------------------------------------------


  // ------   Create the required media from the media file    ----------------
  FairGeoMedia*   geoMedia = geoFace->getMedia();
  FairGeoBuilder* geoBuild = geoLoad->getGeoBuilder();

  // ---> Air
  FairGeoMedium* mAir      = geoMedia->getMedium("air");
  if ( ! mAir ) Fatal("Main", "FairMedium air not found");
  geoBuild->createMedium(mAir);
  TGeoMedium* air = gGeoManager->GetMedium("air");
  if ( ! air ) Fatal("Main", "Medium air not found");
  cout << "Created medium: air" << endl;

  // ---> Iron
  FairGeoMedium* mIron  = geoMedia->getMedium("iron");
  if ( ! mIron ) Fatal("Main", "FairMedium iron not found");
  geoBuild->createMedium(mIron);
  TGeoMedium* iron = gGeoManager->GetMedium("iron");
  if ( ! iron ) Fatal("Main", "Medium iron not found");
  cout << "Created medium: iron" << endl;

  // ---> Lead
  FairGeoMedium* mLead  = geoMedia->getMedium("lead");
  if ( ! mIron ) Fatal("Main", "FairMedium lead not found");
  geoBuild->createMedium(mLead);
  TGeoMedium* lead = gGeoManager->GetMedium("lead");
  if ( ! lead ) Fatal("Main", "Medium iron not found");
  cout << "Created medium: lead" << endl;

  // ---> Tyvek
  FairGeoMedium* mTyvek  = geoMedia->getMedium("PsdTyvek");
  if ( ! mTyvek ) Fatal("Main", "FairMedium PsdTyvek not found");
  geoBuild->createMedium(mTyvek);
  TGeoMedium* tyvek = gGeoManager->GetMedium("PsdTyvek");
  if ( ! tyvek ) Fatal("Main", "Medium PsdTyvek not found");
  cout << "Created medium: PsdTyvek" << endl;

  // ---> Scintillator
  FairGeoMedium* mScint  = geoMedia->getMedium("PsdScint");
  if ( ! mScint ) Fatal("Main", "FairMedium PsdScint not found");
  geoBuild->createMedium(mScint);
  TGeoMedium* scint = gGeoManager->GetMedium("PsdScint");
  if ( ! scint ) Fatal("Main", "Medium PsdScint not found");
  cout << "Created medium: PsdScint" << endl;

  // ---> Fibres
  FairGeoMedium* mFibre  = geoMedia->getMedium("PsdFibre");
  if ( ! mFibre ) Fatal("Main", "FairMedium PsdFibre not found");
  geoBuild->createMedium(mFibre);
  TGeoMedium* fibre = gGeoManager->GetMedium("PsdFibre");
  if ( ! fibre ) Fatal("Main", "Medium PsdFibre not found");
  cout << "Created medium: PsdFibre" << endl;

  // ---> Medium for PSD volume
  TGeoMedium* psdMedium = air;
  // --------------------------------------------------------------------------


  // --------------   Create geometry and top volume  -------------------------
  cout << endl << "==> Set top volume..." << endl;
  gGeoManager->SetName("PSDgeom");
  TGeoVolume* top = new TGeoVolumeAssembly("TOP");
  gGeoManager->SetTopVolume(top);
  // --------------------------------------------------------------------------
  
  
  // -----   Create the PSD modules   -----------------------------------------
  cout << endl;
  TGeoVolume* module2060 = ConstructModule("module2060", 20., 60);
  // --------------------------------------------------------------------------


  // ---------------   Create PSD volume   ------------------------------------
  cout << endl;
  cout << "===> Creating PSD...." << endl;

  // --- Determine size of PSD box (half-lengths)
  Double_t psdSizeX = 0.5 * nModulesX * moduleSize;
  Double_t psdSizeY = 0.5 * nModulesY * moduleSize;
  TGeoBBox* shape = dynamic_cast<TGeoBBox*>(module2060->GetShape());
  Double_t psdSizeZ = shape->GetDZ();

  TString psdName = "psd_";
  psdName += geoTag;
  TGeoVolume* psd = gGeoManager->MakeBox(psdName, psdMedium, psdSizeX,
                                         psdSizeY, psdSizeZ);
  cout << "Module array is " << nModulesX << " x " << nModulesY
       << endl;
  cout << "PSD size is " << 2. * psdSizeX << " cm x " << 2. * psdSizeY
       << " cm x " << 2. * psdSizeZ << " cm" << endl;
  // --------------------------------------------------------------------------


  // -----   Place modules into the system volume   ---------------------------
  Double_t xModCurrent = -0.5 * ( nModulesX + 1 ) * moduleSize;

  Int_t nModules = 0;
  for (Int_t iModX = 0; iModX < nModulesX; iModX++) {
    xModCurrent += moduleSize;
    Double_t yModCurrent = -0.5 * ( nModulesY + 1 ) * moduleSize;
    for (Int_t iModY = 0; iModY < nModulesY; iModY++) {
      yModCurrent += moduleSize;

      // Skip edge modules
      if ( ( iModX == 0 || iModX == nModulesX - 1 ) &&
           ( iModY == 0 || iModY == nModulesY - 1 ) ) continue;

      // Skip centre module (only for uneven number of modules)
      if ( iModX == (nModulesX - 1) / 2 && iModY == (nModulesY - 1) / 2 )
        continue;

      // Node number and name (convention)
      Int_t iNode = 100 * iModX + iModY;

      // Position of module inside PSD
      TGeoTranslation* trans = new TGeoTranslation(xModCurrent, yModCurrent, 0.);

      psd->AddNode(module2060, iNode, trans);
      cout << "Adding module " << setw(5) << right << iNode << " at x = "
           << setw(6) << right << xModCurrent << " cm , y = "
           << setw(6) << right << yModCurrent << " cm" << endl;
      nModules++;

    } //# modules in y direction
  } //# modules in x direction
  cout << "PSD contains " << nModules << " modules." << endl;
  // --------------------------------------------------------------------------



  // -----   Place PSD in top node (cave) -------------------------------------
  TGeoRotation* psdRot = new TGeoRotation();
  psdRot->RotateY(psdRotY);
  TGeoCombiTrans* psdTrans = new TGeoCombiTrans(psdX, psdY, psdZ, psdRot);
  top->AddNode(psd, 0, psdTrans);
  cout << endl << "==> PSD position in cave: " << endl;
  psdTrans->Print();
  // --------------------------------------------------------------------------


  // -----   Close and check geometry   ---------------------------------------
  cout << endl << "==> Closing geometry..." << endl;
  gGeoManager->CloseGeometry();
  gGeoManager->CheckGeometryFull();
  cout << endl;
  gGeoManager->CheckOverlaps(0.0001, "s");
  // --------------------------------------------------------------------------


  // -----   Write PSD volume and placement matrix to geometry file   ---------
  cout << endl;
  TString geoFileName = "psd_";
  geoFileName = geoFileName + geoTag + ".geo.root";
  psd->Export(geoFileName);
  TFile* geoFile = new TFile(geoFileName, "UPDATE");
  psdTrans->Write();
  cout << endl;
  cout << "==> Geometry " << psd->GetName() << " written to "
       << geoFileName << endl;
  geoFile->Close();
  // --------------------------------------------------------------------------


  // -----   Write entire TGeoManager to file   -------------------------------
  TString geoManFileName = "psd_";
  geoManFileName = geoManFileName + geoTag + ".geoman.root";
  TFile* geoManFile = new TFile(geoManFileName, "RECREATE");
  gGeoManager->Write();
  geoManFile->Close();
  cout << "==> TGeoManager " << gGeoManager->GetName() << " written to "
       << geoManFileName << endl << endl << endl;
  // --------------------------------------------------------------------------


  // ----   Display geometry   ------------------------------------------------
  gGeoManager->SetVisLevel(5);  // Scintillator level
  top->Draw("ogl");
  // --------------------------------------------------------------------------
  
}
//  End of main function
/** ======================================================================= **/






/** ======================================================================= **/
TGeoVolume* ConstructModule(const char* name, Double_t sizeXY, Int_t nLayers) {

  // The module consists of nLayers of scintillators covered with Tyvek.
  // Between each two scintillators, there is a lead layer (total nLayers -1).
  // At the top, there is a gap to stretch the light fibres. This is modelled
  // by a volume made of the same material as the scintillator (but inactive).
  // The arrangement is surrounded by iron. At the front and at the back,
  // there is a thick iron layer (the back one acting as first absorber)
  // for constructional reasons.

  cout << "===> Creating Module " << name << ", size " << sizeXY
       << " cm with " << nLayers << " layers...." << endl;

  // -----   Constructional parameters   ----------------------------
  Double_t leadD     =  1.60;  // Thickness of lead layer
  Double_t scintD    =  0.40;  // Thickness of scintillator layer
  Double_t tyvekD    =  0.02;  // Thickness of Tyvek wrapper around scintillator
  Double_t boxDx     =  0.15;  // Thickness of iron box lateral
  Double_t boxDy     =  0.05;  // Thickness of iron box top/bottom
  Double_t boxDz     =  2.00;  // Thickness of iron box front/back
  Double_t chanDy    =  0.20;  // Height of fibre channel
  Double_t chanDistL =  0.50;  // Distance of channel from left edge of lead
  Double_t chanDistR =  2.00;  // Distance of channel from right edge of lead
  // ------------------------------------------------------------------------


  // -----   Get required media   -------------------------------------------
  TGeoMedium* medAir = gGeoManager->GetMedium("air");  // PSD
  if ( ! medAir ) Fatal("ConstructModule", "Medium air not found");
  TGeoMedium* medIron = gGeoManager->GetMedium("iron");  // Box
  if ( ! medIron ) Fatal("ConstructModule", "Medium iron not found");
  TGeoMedium* medLead = gGeoManager->GetMedium("lead");  // Lead layers
  if ( ! medLead ) Fatal("ConstructModule", "Medium lead not found");
  TGeoMedium* medTyvek = gGeoManager->GetMedium("PsdTyvek");  // Tyvek layers
  if ( ! medTyvek ) Fatal("ConstructModule", "Medium PsdTyvek not found");
  TGeoMedium* medScint = gGeoManager->GetMedium("PsdScint");  // Scintillator
  if ( ! medScint ) Fatal("ConstructModule", "Medium PsdScint not found");
  TGeoMedium* medFibre = gGeoManager->GetMedium("PsdFibre");  // Fibres
  if ( ! medFibre ) Fatal("ConstructModule", "Medium PsdFibre not found");
  medAir->Print();
  medIron->Print();
  medLead->Print();
  medTyvek->Print();
  medScint->Print();
  medFibre->Print();
 // ------------------------------------------------------------------------



  // -----   Create module volume   -----------------------------------------
  // Module length: nLayers of scintillators, nLayers - 1 of lead
  // plus the iron box front and back.
  // Material is iron.
  Double_t moduleLength = 2. * boxDz
                        + nLayers * ( scintD + 2. * tyvekD )
                        + ( nLayers - 1 ) * leadD;
  TGeoVolume* module = gGeoManager->MakeBox(name, medIron, 0.5 * sizeXY,
                                            0.5 * sizeXY, 0.5 * moduleLength);
  module->SetLineColor(kRed);
  module->Print();
  cout << endl;
  // ------------------------------------------------------------------------


  // -----   Lead filler   --------------------------------------------------
  // The lead filler represents all lead absorber layers.
  // The Tyvek/scintillator layers and the fibre channel will be placed
  // inside.
  // Dimensions are half-lengths.
  Double_t leadLx = 0.5 * sizeXY - boxDx;
  Double_t leadLy = 0.5 * sizeXY - boxDy;
  Double_t leadLz = 0.5 * moduleLength - boxDz;
  TGeoVolume* lead = gGeoManager->MakeBox("lead", medLead,
                                          leadLx, leadLy, leadLz);
  lead->SetLineColor(kBlue);
  // ------------------------------------------------------------------------


  // -----   Fibre channel   ------------------------------------------------
  // This volume represents the air gap at the top of the volume hosting the
  // light guide fibres. The material is the same as for the scintillators
  // (but inactive).
  // The (half-)width of the channel is defined by the distances from the
  // lead edges.
  Double_t chanLx = leadLx - 0.5 * chanDistL - 0.5 * chanDistR;
  if ( chanLx < 0. ) {
    cout << "Error: lead volume is not large enough to host fibre channel!"
         << endl;
    cout << "Lead width: " << 2. * leadLx << ", distance from left edge "
         << chanDistL << ", distance from right edge " << chanDistR
         << endl;
    return 0;
  }
  Double_t chanLy = 0.5 * chanDy;
  Double_t chanLz = leadLz;
  TGeoVolume* channel = gGeoManager->MakeBox("channel", medFibre,
                                             chanLx, chanLy, chanLz);
  channel->SetLineColor(kMagenta);
  // ------------------------------------------------------------------------


  // ----   Positioning of the fibre channel   ------------------------------
  // The channel extends from 0.5 cm from the left edge of the lead filler
  // to 2 cm from its right edge (seen from the front). It is top-aligned
  // with the lead filler.
  Double_t chanShiftX = 0.5 * ( chanDistL - chanDistR );
  Double_t chanShiftY = leadLy - 0.5 * chanDy;
  // ------------------------------------------------------------------------


  // -----   Tyvek layer   --------------------------------------------------
  // The scintillator will be placed inside this, leaving only the thin
  // Tyvek as a wrapper. Since these layers will be placed in the lead filler,
  // there has to be a cut-out for the fibre channel.
  Double_t tyvekLz = 0.5 * scintD + tyvekD;  // half-length
  TGeoBBox* tyv1 = new TGeoBBox("psdTyv1", leadLx, leadLy, tyvekLz);
  TGeoBBox* tyv2 = new TGeoBBox("psdTyv2", chanLx, chanLy, tyvekLz);
  TGeoTranslation* trans1 = new TGeoTranslation("tPsd1",
                                                chanShiftX, chanShiftY, 0.);
  trans1->RegisterYourself();
  TGeoCompositeShape* tyvekShape = new TGeoCompositeShape("psdTyv1-psdTyv2:tPsd1");
  TGeoVolume* tyvek = new TGeoVolume("tyvek", tyvekShape, medTyvek);
  tyvek->SetLineColor(kGreen);
  // ------------------------------------------------------------------------


  // -----   Scintillator layer   -------------------------------------------
  // The scintillator layer is embedded (as daughter) into the Tyvek layer.
  // It is also a box minus the cut-out for the fibres. The cut-out is
  // slightly smaller than for the Tyvek volume.
  Double_t scintLx = leadLx - tyvekD;
  Double_t scintLy = leadLy - tyvekD;
  Double_t scintLz = 0.5 * scintD;
  TGeoBBox* sci1 = new TGeoBBox("sci1", scintLx, scintLy, scintLz);
  Double_t cutLx = chanLx;
  Double_t cutLy = chanLy - tyvekD;
  Double_t cutLz = scintLz;
  TGeoBBox* sci2 = new TGeoBBox("sci2", cutLx, cutLy, cutLz);
  Double_t cutShiftX = chanShiftX;
  Double_t cutShiftY = scintLy - cutLy;
  TGeoTranslation* trans2 = new TGeoTranslation("tPsd2", cutShiftX, cutShiftY, 0.);
  trans2->RegisterYourself();
  TGeoCompositeShape* scintShape = new TGeoCompositeShape("scintShape", "sci1-sci2:tPsd2");
  TGeoVolume* scint = new TGeoVolume("scint", scintShape, medScint);
  scint->SetLineColor(kBlack);
  // ------------------------------------------------------------------------


  // -----   Place volumes   ------------------------------------------------

  // ---> Scintillator into Tyvek
  tyvek->AddNode(scint, 0);

  // ---> Fibre channel into lead filler
  lead->AddNode(channel, 0, trans1);

  // --->  Tyvek layers into lead
  Double_t zFirst = 0.;
  Double_t zLast  = 0.;
  for (Int_t iLayer = 0; iLayer < nLayers; iLayer++) {
    Double_t zPos = -1. * leadLz + iLayer * leadD + (2. * iLayer + 1.) * tyvekLz;
    if ( iLayer == 0 ) zFirst = zPos;
    if ( iLayer == nLayers - 1) zLast = zPos;
    lead->AddNode(tyvek, iLayer, new TGeoTranslation(0., 0., zPos));
  }
  cout << module->GetName() << ": Positioned " << nLayers << " Tyvek layers; first at z = " << zFirst
      << ", last at z = " << zLast << endl;

  // ---> Lead into module
  module->AddNode(lead, 0);
  // ------------------------------------------------------------------------

  return module;
}
/** ======================================================================= **/


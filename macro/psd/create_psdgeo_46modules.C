/** @file create_psdgeo.C
 ** @author Volker Friese <v.friese@gsi.de>
 ** @date 16 September 2017
 ** @version 1.0
 **
 ** This macro creates a PSD geometry in ROOT format to be used as input
 ** for the transport simulation. It allows to create module stacks of
 ** varying sizes; the numbers of rows and columns must both be uneven.
 ** The centre module as well as the edge modules are omitted.
 ** The module definition is implemented in the function ConstructModule.
 **
 ** The user can adjust the steering variables to the specific needs.
 **/


using std::cout;
using std::endl;
using std::fstream;
using std::right;
using std::setw;

// Forward declarations
TGeoVolume* ConstructModule(const char* name, Double_t moduleSize,
                            Int_t nLayers, fstream* info = 0);

TGeoVolume* ConstructShield(const char* name, Double_t sizeXY, Double_t holesize, Int_t hole_pos, fstream* info);

// ============================================================================
// ======                         Main function                           =====
// ============================================================================

void create_psdgeo_46modules()
{
  // -----   Steering variables   ---------------------------------------------
  const Double_t psdX       = 9.65;     // x position of PSD in cave (front plane center)
  const Double_t psdY       = 0.;     // y position of PSD in cave (front plane center)
  const Double_t psdZ       = 800.;   // z position of PSD in cave (front plane center)
  const Double_t psdRotY    = 0.01321;     // Rotation of PSD around y axis [rad]
  const Double_t holeSize = 0.;      // diagonal size of the square shaped hole 
  
  Double_t moduleSize = 20.;    // Module size [cm]
  const Double_t shieldWidth = 16.;
  
  TString geoTag = Form ("46mod_hole20cm_xshift%4.2fcm", psdX);  // Geometry tag
  // --------------------------------------------------------------------------


  // -------   Open info file   -----------------------------------------------
  TString infoFileName = "psd_";
  infoFileName = infoFileName + geoTag + ".geo.info";
  fstream infoFile;
  infoFile.open(infoFileName.Data(), fstream::out);
  infoFile << "PSD geometry " << geoTag << " created with create_psdgeo_46modules.C"
    << endl << endl;
  infoFile << "PSD front plane center coordinates: (" << psdX << ", " << psdY << ", "
    << psdZ << ") cm" << endl;
  infoFile << "PSD rotation around y axis: " << psdRotY * TMath::RadToDeg () << " degrees"
    << endl;
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

  // ---> Shield Polyethylene
  FairGeoMedium* mShieldPoly  = geoMedia->getMedium("PsdPolyethylene");
  if ( ! mShieldPoly ) Fatal("Main", "FairMedium PsdPolyethylene not found");
  geoBuild->createMedium(mShieldPoly);
  TGeoMedium* ShieldPoly = gGeoManager->GetMedium("PsdPolyethylene");
  if ( ! ShieldPoly ) Fatal("Main", "Medium PsdPolyethylene not found");
  cout << "Created medium: PsdPolyethylene" << endl;


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
  TGeoVolume* module2060 = ConstructModule("module2060", 20., 60, &infoFile);
  TGeoVolume* module_shield = ConstructShield("shield20", moduleSize, 0., 0, &infoFile);
  // --------------------------------------------------------------------------


  // ---------------   Create PSD volume   ------------------------------------
  cout << endl;
  cout << "===> Creating PSD...." << endl;

  // --- Determine size of PSD box (half-lengths)
  Double_t psdSizeX = 180;
  Double_t psdSizeY = 140;
  TGeoBBox* shape = dynamic_cast<TGeoBBox*>(module2060->GetShape());
  Double_t psdSizeZ = shape->GetDZ();

  TString psdName = "psd_";
  psdName += geoTag;
  TGeoVolume* psd = gGeoManager->MakeBox(psdName, psdMedium, psdSizeX,
                                         psdSizeY, psdSizeZ + 0.5*shieldWidth);
  cout << "PSD size is " << 2. * psdSizeX << " cm x " << 2. * psdSizeY
    << " cm x " << 2. * psdSizeZ << " cm" << endl;
  infoFile << endl << "PSD size is " << 2. * psdSizeX << " cm x "
    << 2. * psdSizeY << " cm x " << 2. * psdSizeZ << " cm" << endl;
  // --------------------------------------------------------------------------

  struct PSDStatic_t {
    Double_t posX;
    Double_t posY;
  };

  std::vector<PSDStatic_t> module2060Positions = {
    // inner part
    {        .0,     20.0},
    {      20.0,     20.0},
    {      20.0,       .0},
    {      20.0,    -20.0},
    {        .0,    -20.0},
    {     -20.0,    -20.0},
    {     -20.0,       .0},
    {     -20.0,     20.0},
    // mid-central part
    {        .0,     40.0},
    {      20.0,     40.0},
    {      40.0,     40.0},
    {      40.0,     20.0},
    {      40.0,       .0},
    {      40.0,    -20.0},
    {      40.0,    -40.0},
    {      20.0,    -40.0},
    {        .0,    -40.0},
    {     -20.0,    -40.0},
    {     -40.0,    -40.0},
    {     -40.0,    -20.0},
    {     -40.0,       .0},
    {     -40.0,     20.0},
    {     -40.0,     40.0},
    {     -20.0,     40.0},

    {     -60.0,    -40.0},
    {     -60.0,    -20.0},
    {     -60.0,       .0},
    {     -60.0,     20.0},
    {     -60.0,     40.0},

    {      60.0,    -40.0},
    {      60.0,    -20.0},
    {      60.0,       .0},
    {      60.0,     20.0},
    {      60.0,     40.0},
    // outer-top part
    {        .0,     60.0},
    {     -20.0,     60.0},
    {      20.0,     60.0},
    // outer-bottom part
    {        .0,    -60.0},
    {     -20.0,    -60.0},
    {      20.0,    -60.0},
    // outer-left part
    {     -80.0,       .0},
    {     -80.0,    -20.0},
    {     -80.0,     20.0},
    // outer-right part
    {      80.0,       .0},
    {      80.0,    -20.0},
    {      80.0,     20.0},
  };


  // -----   Place modules into the system volume   ---------------------------
  Double_t xModCurrent;
  Double_t yModCurrent;

  Int_t nModules = 0;
  for (auto mm : module2060Positions) {
    xModCurrent = mm.posX;
    yModCurrent = mm.posY;
    // Node number and name (convention)
    Int_t iNode = nModules;

    // Position of module inside PSD
    TGeoTranslation* trans = new TGeoTranslation(xModCurrent, yModCurrent, -0.5*shieldWidth);
    TGeoTranslation* trans_shield = new TGeoTranslation(xModCurrent, yModCurrent, psdSizeZ);

    psd->AddNode(module2060, iNode, trans);
    psd->AddNode(module_shield, iNode, trans_shield);

    cout << "Adding module " << setw(5) << right << iNode << " at x = "
      << setw(6) << right << xModCurrent << " cm , y = "
      << setw(6) << right << yModCurrent << " cm" << endl;
    nModules++;

  } //# modules

  cout << endl;

  cout << "PSD contains " << nModules << " modules." << endl;
  infoFile << "PSD contains " << nModules << " modules." << endl;
  // --------------------------------------------------------------------------



  // -----   Place PSD in top node (cave) -------------------------------------
  Double_t psdHalfLength = psdSizeZ + 0.5 * shieldWidth;
  Double_t psdVolCenterX = psdX + psdHalfLength * sin (psdRotY);
  Double_t psdVolCenterY = psdY;
  Double_t psdVolCenterZ = psdZ + psdHalfLength * cos (psdRotY);
  infoFile << "PSD volume center coordinates: (" << psdVolCenterX << ", " << psdVolCenterY << ", "
  << psdVolCenterZ << ") cm" << endl;
  TGeoRotation* psdRot = new TGeoRotation();
  psdRot->RotateY(psdRotY * TMath::RadToDeg ());
  TGeoCombiTrans* psdTrans = new TGeoCombiTrans(psdVolCenterX, psdVolCenterY, psdVolCenterZ, psdRot);
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
  cout << "==> Info written to " << infoFileName << endl;
  geoFile->Close();
  infoFile.close();
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

TGeoVolume* ConstructShield(const char* name, Double_t sizeXY, Double_t holesize, Int_t hole_pos, fstream* info) {

    const TString suffix = Form("_shield_%d_%d_%d", int(sizeXY), int(holesize), int(hole_pos));

    const Double_t psd2iron = 2.;
    const Double_t ironD = 4.;
    const Double_t iron2poly = 2.;
    const Double_t polyD = 8.;
    const Double_t shieldLength = psd2iron + ironD + iron2poly + polyD;

    TGeoMedium* medAir = gGeoManager->GetMedium("air");  // PSD
    if ( ! medAir ) Fatal("ConstructModule", "Medium air not found");
    
    TGeoMedium* medIron = gGeoManager->GetMedium("iron");  // Box
    if ( ! medIron ) Fatal("ConstructModule", "Medium iron not found");

    TGeoMedium* medShieldPoly = gGeoManager->GetMedium("PsdPolyethylene");
    if ( ! medShieldPoly ) Fatal("Main", "Medium PsdPolyethylene not found");

    TGeoVolume* shield = gGeoManager->MakeBox(name, medAir, 0.5 * sizeXY, 0.5 * sizeXY, 0.5 * shieldLength);
    TGeoVolume* iron{nullptr};
    TGeoVolume* poly{nullptr};

    if (holesize > 0)
    {
        Int_t sx, sy;
        switch (hole_pos)
        {
            case 0:  sx =  1;    sy =  1;   break;  // defines hole position in x-y plane
            case 1:  sx =  1;    sy = -1;   break;
            case 2:  sx = -1;    sy =  1;   break;
            case 3:  sx = -1;    sy = -1;   break;
        }    
        TGeoCombiTrans* rot45 = new TGeoCombiTrans(Form("rot45%s", suffix.Data()), sx*sizeXY/2., sy*sizeXY/2., 0., new TGeoRotation("rot", 45.,0.,0.));
        rot45->RegisterYourself();
        
        TGeoBBox* iron1 = new TGeoBBox(Form("iron1%s", suffix.Data()), 0.5 * sizeXY, 0.5 * sizeXY, 0.5 * ironD);
        TGeoBBox* iron2 = new TGeoBBox(Form("iron2%s",suffix.Data() ), 0.5 * holesize, 0.5 * holesize, 0.5 * ironD); //hole
        
        TGeoCompositeShape* ironShape = new TGeoCompositeShape(Form("iron1%s-iron2%s:rot45%s", suffix.Data(), suffix.Data(), suffix.Data() ));
        iron = new TGeoVolume( Form("iron%s", suffix.Data()), ironShape, medIron);

        TGeoBBox* poly1 = new TGeoBBox(Form("poly1%s", suffix.Data()), 0.5 * sizeXY, 0.5 * sizeXY, 0.5 * polyD);
        TGeoBBox* poly2 = new TGeoBBox(Form("poly2%s",suffix.Data() ), 0.5 * holesize, 0.5 * holesize, 0.5 * polyD); //hole

        TGeoCompositeShape* polyShape = new TGeoCompositeShape(Form("poly1%s-poly2%s:rot45%s", suffix.Data(), suffix.Data(), suffix.Data() ));
        poly = new TGeoVolume( Form("poly%s", suffix.Data()), polyShape, medShieldPoly);
    }
    else
    {
        iron = gGeoManager->MakeBox("shield_iron", medIron, 0.5 * sizeXY, 0.5 * sizeXY, 0.5 * ironD);
        poly = gGeoManager->MakeBox("shield_poly", medShieldPoly, 0.5 * sizeXY, 0.5 * sizeXY, 0.5 * polyD);
    }

    shield->AddNode(iron, 0, new TGeoTranslation(0., 0., -shieldLength/2. + psd2iron + ironD/2. ));
    shield->AddNode(poly, 0, new TGeoTranslation(0., 0., -shieldLength/2. + psd2iron + ironD + iron2poly + polyD/2. ));
    
    return shield;
}


/** ======================================================================= **/
TGeoVolume* ConstructModule(const char* name, Double_t sizeXY,
                            Int_t nLayers, fstream* info) {

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


  // -----   Info to file   -------------------------------------------------
  if ( info ) {
    (*info) << "Parameters of module " << name << ": " << endl;
    (*info) << "Size: " << sizeXY << " cm x " << sizeXY << " cm" << endl;
    (*info) << "Number of layers: " << nLayers << endl;
    (*info) << "Thickness of lead layers: " << leadD << " cm" << endl;
    (*info) << "Thickness of scintillators: " << scintD << " cm" << endl;
    (*info) << "Thickness of Tyvek wrap: " << tyvekD << " cm" << endl;
    (*info) << "Thickness of iron box: (" << boxDx << " / " << boxDy
      << " / " << boxDz << ") cm" << endl;
    (*info) << "Height of fibre channel: " << chanDy << " cm" << endl;
    (*info) << "Distance of channel from edges: left " << chanDistL
      << " cm, right " << chanDistR << " cm" << endl;
  }
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
  // The channel extends from chanDistL from the left edge of the lead filler
  // to chanDistR from its right edge (seen from the front). It is top-aligned
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


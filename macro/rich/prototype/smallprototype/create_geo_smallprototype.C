#include <iostream>
using namespace std; 



void create_geo_smallprototype()
{

gSystem->Load("libGeom");
//gGeoMan = gGeoManager;// (TGeoManager*)gROOT->FindObject("FAIRGeom");
//new TGeoManager ("Testbox", "Testbox");

TString geoFileName= "/data/cbm/cbmroot/geometry/rich/prototype/Testbox.geo.root";
//TString geoFileName= "/Users/slebedev/Development/cbm/trunk/cbmroot/geometry/rich/prototype/Testbox.geo.root";
FairGeoLoader*    geoLoad = new FairGeoLoader("TGeo","FairGeoLoader");
  FairGeoInterface* geoFace = geoLoad->getGeoInterface();
  TString geoPath = gSystem->Getenv("VMCWORKDIR");
  TString medFile = geoPath + "/geometry/media.geo";
  //TString medFile = geoPath + "/geometry/media_rich_prototype.geo";
  geoFace->setMediaFile(medFile);
  geoFace->readMedia();
  gGeoMan = gGeoManager;

FairGeoMedia*   geoMedia = geoFace->getMedia();
  FairGeoBuilder* geoBuild = geoLoad->getGeoBuilder();

//Media
FairGeoMedium* mCoating      = geoMedia->getMedium("RICHglass");
  if ( ! mCoating) Fatal("Main", "FairMedium RICHglass not found");
  geoBuild->createMedium(mCoating);
  TGeoMedium* medCoating = gGeoMan->GetMedium("RICHglass");
  if ( ! medCoating) Fatal("Main", "Medium RICHglass not found");

FairGeoMedium* mAluminium      = geoMedia->getMedium("aluminium");				
  if ( ! mAluminium ) Fatal("Main", "FairMedium aluminium not found");
  geoBuild->createMedium(mAluminium);
  TGeoMedium* medAl = gGeoMan->GetMedium("aluminium");
  if ( ! medAl ) Fatal("Main", "Medium vacuum not found");

FairGeoMedium* mVacuum      = geoMedia->getMedium("vacuum");				
  if ( ! mVacuum ) Fatal("Main", "FairMedium vacuum not found");
  geoBuild->createMedium(mVacuum);
  TGeoMedium* medVacuum = gGeoMan->GetMedium("vacuum");
  if ( ! medVacuum ) Fatal("Main", "Medium vacuum not found");

FairGeoMedium* mNitrogen      = geoMedia->getMedium("RICHgas_N2_dis");				
  if ( ! mNitrogen ) Fatal("Main", "FairMedium RICHgas_N2_dis not found");
  geoBuild->createMedium(mNitrogen);
  TGeoMedium* medNitrogen = gGeoMan->GetMedium("RICHgas_N2_dis");
  if ( ! medNitrogen ) Fatal("Main", "Medium RICHgas_N2_dis not found");

FairGeoMedium* mPMT      = geoMedia->getMedium("CsI");
  if ( ! mPMT ) Fatal("Main", "FairMedium CsI not found");
  geoBuild->createMedium(mPMT);
  TGeoMedium* medCsI = gGeoMan->GetMedium("CsI");
  if ( ! medCsI ) Fatal("Main", "Medium CsI not found");

FairGeoMedium* mGlass      = geoMedia->getMedium("Rich_NBK7_glass");
  if ( ! mGlass ) Fatal("Main", "FairMedium Rich_NBK7_glass not found");
  geoBuild->createMedium(mGlass);
  TGeoMedium* medGlass = gGeoMan->GetMedium("Rich_NBK7_glass");
  if ( ! medGlass ) Fatal("Main", "Medium Rich_NBK7_glass  not found");

FairGeoMedium* mElectronic      = geoMedia->getMedium("air");
  if ( ! mElectronic) Fatal("Main", "FairMedium air not found");
  geoBuild->createMedium(mElectronic);
  TGeoMedium* medElectronic = gGeoMan->GetMedium("air");
  if ( ! medElectronic) Fatal("Main", "Medium air not found");



//Variable
const Double_t testboxlength=44;		//Testbox
const Double_t testboxwidth=20;
const Double_t testboxheight=30;
const Double_t wallwidth=0.3;


const Double_t pmtsize =4.85;			//PMT
const Double_t pmtpixelsize =0.6;
const Double_t pmtgap =0.1;
const Double_t pmtpmtdis=3.5;

const Double_t eleclength=10;			//Electronic
const Double_t elecwidth=3*pmtsize;
const Double_t elecheight=2*pmtsize;

const Double_t centerthickness =2.44;	//Lense
const Double_t lenseradius =15.51;
const Double_t lensepmtdistance =3.0;
const Double_t lensebeschichtung =0.1;

const Double_t absorberthickness =0.1;
const Double_t absorberradius =1.6;

const Double_t sensplanesize=80;
const Double_t sensplaneboxdis=1;


//Transformations
TGeoRotation *rotBox= new TGeoRotation("Boxrotation", 0., 0., 0.);
TGeoTranslation *trRichCave= new TGeoTranslation(0., 0., 0.);
    
TGeoTranslation *trBox= new TGeoTranslation(0., 0., 0.);			//Gasbox/Box Translation
TGeoTranslation *trsensplane= new TGeoTranslation(0.,0., testboxlength+ sensplaneboxdis);

TGeoTranslation *trPMTup= new TGeoTranslation(0., pmtsize+pmtpmtdis/2, -(lenseradius-centerthickness-lensepmtdistance));		//PMTContainer Translations
TGeoTranslation *trPMTdown= new TGeoTranslation(0., -(pmtsize+pmtpmtdis/2), -(lenseradius-centerthickness-lensepmtdistance));


TGeoTranslation *tr2= new TGeoTranslation( 	-(pmtsize + pmtgap)	,  pmtsize/2 + pmtgap/2	, 0.);		//PMT Translations
TGeoTranslation *tr3= new TGeoTranslation(	0.					,  pmtsize/2 + pmtgap/2	, 0.);
TGeoTranslation *tr4= new TGeoTranslation( 	pmtsize + pmtgap 	,  pmtsize/2 + pmtgap/2	, 0.);
TGeoTranslation *tr5= new TGeoTranslation( 	-(pmtsize + pmtgap)	,-(pmtsize/2 + pmtgap/2), 0.);
TGeoTranslation *tr6= new TGeoTranslation(	0.					,-(pmtsize/2 + pmtgap/2), 0.);
TGeoTranslation *tr7= new TGeoTranslation( 	pmtsize + pmtgap	,-(pmtsize/2 + pmtgap/2), 0.);

//TGeoTranslation *trelec1= new TGeoTranslation(0., pmtsize+0.5 , -(lenseradius-centerthickness-lensepmtdistance)+eleclength/2+0.1); 		//Electronic Transformations
//TGeoTranslation *trelec2= new TGeoTranslation(0., -(pmtsize+0.5) ,-(lenseradius-centerthickness-lensepmtdistance)+eleclength/2+0.1);

TGeoTranslation *trabsorber= new TGeoTranslation(0. ,0., -(lenseradius-centerthickness)+absorberthickness/2);


TGeoTranslation *trLense= new TGeoTranslation(0., 0., 0.);		//Lense Translations
TGeoTranslation *trComp= new TGeoTranslation("trComp", 0.,0., -(lenseradius-centerthickness)/2+0.1);
trComp->RegisterYourself();

//Virtual Topbox
TGeoVolume* top = new TGeoVolumeAssembly("rich");
 gGeoMan->SetTopVolume(top);

//Shapes
    
TGeoVolume *richCave= gGeoMan->MakeBox("RichCave", medNitrogen, 300., 300., 300.);

//TopBox
TGeoVolume *box= gGeoMan->MakeBox("TopBox", medNitrogen, testboxwidth/2, testboxheight/2, testboxlength/2);
//gGeoMan->SetTopVolume(box);

//Gasbox

TGeoVolume *gas= gGeoMan->MakeBox("Gasbox", medNitrogen , testboxwidth/2-wallwidth, testboxheight/2-wallwidth, testboxlength/2-wallwidth);

//PMT Container containing PMT Matrix

TGeoVolume *pmtcontainer= gGeoMan->MakeBox("PMTContainer", medCsI , 3*pmtsize/2, 2*pmtsize/2, 0.1);

//PMT
TGeoVolume *pmt= gGeoMan->MakeBox("PMT", medCsI , pmtsize/2, pmtsize/2, 0.1);

//PMT Pixels

TGeoVolume *pmtpixel= gGeoMan->MakeBox("pmt_pixel", medCsI, pmtpixelsize/2, pmtpixelsize/2, 0.1); 

//Electronic

//TGeoVolume *elec= gGeoMan->MakeBox("elec", medElectronic , elecwidth/2, elecheight/2, eleclength/2);

//Lensecoating parametrized as sphere
TGeoSphere *coating= new TGeoSphere("Coating", lenseradius, lenseradius+lensebeschichtung, 90., 180., 0., 360.);

//Lense parametrized as sphere
TGeoSphere *lense= new TGeoSphere("Lense", 0., lenseradius, 90., 180., 0., 360.);


//Cutout for Lense
TGeoBBox *cutoutlense= new TGeoBBox("CutOutLense", lenseradius+lensebeschichtung, lenseradius+lensebeschichtung, (lenseradius-centerthickness)/2);

//Composite Lense
TGeoCompositeShape *endlense= new TGeoCompositeShape("Endlense", "Lense-CutOutLense:trComp");
TGeoCompositeShape *lensecoating= new TGeoCompositeShape("Endlensecoating", "Coating-CutOutLense:trComp");
TGeoVolume *compendlense= new TGeoVolume("CompEndLense", endlense, medGlass);
TGeoVolume *complensecoating= new TGeoVolume("Complensecoating", lensecoating, medCoating); 

//Absorber
TGeoVolume *absorber= gGeoMan->MakeTube("Absorber", medAl, 0., absorberradius, absorberthickness/2);

// Sensitive plane behind box
TGeoVolume *sensplane= gGeoMan->MakeBox("sens_plane", medCsI, sensplanesize/2 , sensplanesize/2  ,0.1);

complensecoating->SetLineColor(kRed);
compendlense->SetLineColor(kBlue);





//Positioning
top->AddNode(richCave, 1, trRichCave);

richCave->AddNode(box, 1, trBox);
richCave->AddNode(sensplane, 1, trsensplane);

box->AddNode(gas, 1, rotBox);

gas->AddNode(complensecoating, 1, trLense);
gas->AddNode(compendlense, 1, trLense);
gas->AddNode(pmtcontainer, 1, trPMTup);
gas->AddNode(pmtcontainer, 2, trPMTdown);
//gas->AddNode(elec, 1, trelec1);
//gas->AddNode(elec, 2, trelec2);



compendlense->AddNode(absorber, 1, trabsorber);

pmtcontainer->AddNode(pmt, 1, tr2);
pmtcontainer->AddNode(pmt, 2, tr3);
pmtcontainer->AddNode(pmt, 3, tr4);
pmtcontainer->AddNode(pmt, 4, tr5);
pmtcontainer->AddNode(pmt, 5, tr6);
pmtcontainer->AddNode(pmt, 6, tr7);




for(int i=0; i<8; i++)
	{
		for(int j=0; j<8; j++)
		{
			TGeoTranslation *trij = new TGeoTranslation((2*i-7)*pmtpixelsize/2, (2*j-7)*pmtpixelsize/2, 0.);
			pmt->AddNode(pmtpixel, 8*i+j+1, trij);
		}	
	}	



gGeoMan->CheckOverlaps();
gGeoMan->PrintOverlaps();

//Draw
box->SetLineColor(kBlack);
gas->SetLineColor(kBlue);
gGeoMan->SetTopVisible();
box->SetVisibility(false);


top->Draw("ogl");


 TFile* geoFile = new TFile(geoFileName, "RECREATE");
  top->Write();
  cout << endl;
  cout << "Geometry " << top->GetName() << " written to " 
       << geoFileName << endl;
  geoFile->Close();


}

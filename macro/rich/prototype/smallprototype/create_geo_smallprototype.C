#include <iostream>
using namespace std;



void create_geo_smallprototype()
{

gSystem->Load("libGeom");
//gGeoMan = gGeoManager;// (TGeoManager*)gROOT->FindObject("FAIRGeom");
//new TGeoManager ("Testbox", "Testbox");

TString geoFileName= "Testbox.geo.root";

FairGeoLoader*    geoLoad = new FairGeoLoader("TGeo","FairGeoLoader");
  FairGeoInterface* geoFace = geoLoad->getGeoInterface();
  TString geoPath = gSystem->Getenv("VMCWORKDIR");
  TString medFile = geoPath + "/geometry/media.geo";
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





//Variable
const Double_t testboxlength=44;		//Testbox
const Double_t testboxwidth=20;
const Double_t testboxheight=30;

const Double_t pmtsize =4.85;			//PMT
const Double_t pmtpixelsize =pmtsize/8;

const Double_t centerthickness =2.44;	//Lense
const Double_t lenseradius =15.51;
const Double_t lensepmtdistance =3;
const Double_t lensebeschichtung =0.1;


//Transformations
TGeoTranslation *trBox= new TGeoTranslation(0., 0., 0.);			//Gasbox/Box Translation
TGeoTranslation *trPMTup= new TGeoTranslation(0., pmtsize+0.5, -(lenseradius-centerthickness-lensepmtdistance));		//PMTContainer Translations
TGeoTranslation *trPMTdown= new TGeoTranslation(0., -(pmtsize+0.5), -(lenseradius-centerthickness-lensepmtdistance));


TGeoTranslation *tr2= new TGeoTranslation( 	-pmtsize,  pmtsize/2, 0.);		//PMT Translations
TGeoTranslation *tr3= new TGeoTranslation(	0.		,  pmtsize/2, 0.);
TGeoTranslation *tr4= new TGeoTranslation( 	pmtsize ,  pmtsize/2, 0.);
TGeoTranslation *tr5= new TGeoTranslation( 	-pmtsize, -pmtsize/2, 0.);
TGeoTranslation *tr6= new TGeoTranslation(	0.		, -pmtsize/2, 0.);
TGeoTranslation *tr7= new TGeoTranslation( 	pmtsize , -pmtsize/2, 0.);

TGeoTranslation *tr100= new TGeoTranslation(0. ,0., lenseradius+1);


TGeoTranslation *trLense= new TGeoTranslation(0., 0., 0.);		//Lense Translations
TGeoTranslation *trComp= new TGeoTranslation("trComp", 0.,0., -(lenseradius-centerthickness)/2+0.1);
trComp->RegisterYourself();

//Virtual Topbox
TGeoVolume* top = new TGeoVolumeAssembly("top");
 gGeoMan->SetTopVolume(top);

//Shapes

//TopBox
TGeoVolume *box= gGeoMan->MakeBox("TopBox", medNitrogen, testboxwidth/2+50 , testboxheight/2+50, testboxlength/2+50);
//gGeoMan->SetTopVolume(box);

//Gasbox

TGeoVolume *gas= gGeoMan->MakeBox("Gasbox", medNitrogen , testboxwidth/2-1+50, testboxheight/2-1+50, testboxlength/2-1+50);

//PMT Container containing PMT Matrix

TGeoVolume *pmtcontainer= gGeoMan->MakeBox("PMTContainer", medCsI , 3*pmtsize/2, 2*pmtsize/2, 0.5);

//PMT
TGeoVolume *pmt= gGeoMan->MakeBox("PMT", medCsI , pmtsize/2, pmtsize/2, 0.5);

//PMT Pixels

TGeoVolume *pmtpixel= gGeoMan->MakeBox("pmt_pixel", medCsI, pmtpixelsize/2, pmtpixelsize/2, 0.5); 

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


complensecoating->SetLineColor(kRed);
compendlense->SetLineColor(kBlue);


//Positioning
top->AddNode(box, 1, trBox);

box->AddNode(gas, 1, trBox);

gas->AddNode(complensecoating, 1, trLense);
gas->AddNode(compendlense, 1, trLense);
gas->AddNode(pmtcontainer, 1, trPMTup);
gas->AddNode(pmtcontainer, 2, trPMTdown);


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


box->Draw();


 TFile* geoFile = new TFile(geoFileName, "RECREATE");
  top->Write();
  cout << endl;
  cout << "Geometry " << top->GetName() << " written to " 
       << geoFileName << endl;
  geoFile->Close();


}

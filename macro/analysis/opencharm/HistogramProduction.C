
TString caveGeom="";
TString pipeGeom="";
TString magnetGeom="";
TString mvdGeom="";
TString stsGeom="";
TString richGeom="";
TString muchGeom="";
TString shieldGeom="";
TString trdGeom="";
TString tofGeom="";
TString ecalGeom="";
TString platformGeom="";
TString psdGeom="";
Double_t psdZpos=0.;
Double_t psdXpos=0.;

TString mvdTag="";
TString stsTag="";
TString trdTag="";
TString tofTag="";  

TString stsDigi="";
TString trdDigi="";
TString tofDigi="";

TString mvdMatBudget="";
TString stsMatBudget="";

TString  fieldMap="";
Double_t fieldZ=0.;
Double_t fieldScale=0.;
Int_t    fieldSymType=0;


TString input;
TString inputGEV;
TString system;
TString signal; // "dminus" "dplus" "d0_4B"
Int_t  iVerbose;
TString setup;

bool littrack;
Bool_t useMC;


void HistogramProduction(Int_t nEvents = 100, Int_t ProcID = 1, bool PileUp = false, Int_t PidTyp = 0)
{
// -------------------------------------------------------------------------
TString version = "version1.trunkRef";

gROOT->LoadMacro("CharmSetup.C");
gInterpreter->ProcessLine("CharmSetup()");

switch (PidTyp)
{
case 0:
    TString pidMode = "NONE";
    break;
case 1:
    TString pidMode = "MC";
    break;
case 2:
    TSTring pidMode = "TOF";
    break;
default:
    TString pidMode = "NONE";

}

// Input file (MC events)
TString mcFileName = Form("/hera/cbm/users/psitzmann/data/mc/opencharm.mc.urqmd.%s.%s.%i.%i.%s.%s.root",input.Data(), inputGEV.Data(), nEvents, ProcID, signal.Data(), setup.Data());

// Reco file
TString rcSystem = Form("/hera/cbm/users/psitzmann/data/reco/opencharm.reco.urqmd.%s.%s.%i.%i.%s.%s", input.Data(), inputGEV.Data(), nEvents, ProcID,  signal.Data(), setup.Data());
if(!PileUp)
  {
  if(littrack)
     TString rcFileName = rcSystem + ".littrack.root";
  else
     TString rcFileName = rcSystem + ".l1.root";
  }
  else if(littrack)
     TString rcFileName = rcSystem + ".PileUp.littrack.root";
  else
     TString rcFileName = rcSystem + ".PileUp.l1.root";

  // Pair file
  TString pairSystem = Form("/hera/cbm/users/psitzmann/data/ana/opencharm.pairs.urqmd.%s.%s.%i.%i.%s.%s.pidMode_%s", input.Data(), inputGEV.Data(), nEvents, ProcID, signal.Data(), setup.Data(), pidMode.Data());
  if(useMC) pairSystem += ".mcMode";
  if(!PileUp)
    {
    if(littrack)
       TString pairFileName = pairSystem + ".littrack.root";
    else 
       TString pairFileName = pairSystem + ".l1.root";
    }
    else if(littrack)
       TString pairFileName = pairSystem + ".PileUp.littrack.root";
    else 
       TString pairFileName = pairSystem + ".PileUp.l1.root";
    

    // Track file
  TString trackSystem = Form("/hera/cbm/users/psitzmann/data/ana/opencharm.tracks.urqmd.%s.%s.%i.%i.%s.%s.pidMode_%s", input.Data(), inputGEV.Data(), nEvents, ProcID, signal.Data(), setup.Data(), pidMode.Data());
  if(!PileUp)
    {
    if(littrack)
       TString trackFileName = trackSystem + ".littrack.root";
    else 
       TString trackFileName = trackSystem + ".l1.root";
    }
    else if(littrack)
       TString trackFileName = trackSystem + ".PileUp.littrack.root";
    else 
	TString trackFileName = trackSystem + ".PileUp.l1.root";

  TString outFileName = Form("/hera/cbm/users/psitzmann/data/ana/histo/opencharm.histo.%s.%s.%i.%i.%s.%s.%s.root",input.Data(), inputGEV.Data(), nEvents, ProcID, signal.Data(), setup.Data(), version.Data());

  TFile* outFile = new TFile(outFileName, "RECREATE");
  TFile* mcFile = new TFile(mcFileName, "READ");
 // TFile* pairFile = new TFile(pairFileName);
 // TFile* rcFile = new TFile(rcFileName);
 // TFile* trackFile = new TFile(trackFileName);

// -------------------------------------------------------------------------
    TStopwatch timer;
    timer.Start();

    gDebug=0;
    // -------------------------------------------------------------------------


    const char* group = "MCQA";

    CbmD0HistogramManager* D0histo  = new CbmD0HistogramManager(group, 1, 0.04,  0.01);
    D0histo->SetOutFile(outFile);
    D0histo->SetMCFile(mcFile);
   // D0histo->SetRecoFile(rcFile);
  //  D0histo->SetPairFile(pairFile);
  //  D0histo->SetTrackFile(trackFile);

    D0histo->Init();
    D0histo->Exec("");

   cout << "output file is : " << outFileName << endl;

}

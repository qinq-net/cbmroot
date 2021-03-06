/**
 * \file CbmHistManager.cxx
 * \brief Histogram manager.
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2011
 */

#include "CbmHistManager.h"
#include "utils/CbmUtils.h"

#include "FairLogger.h"

#include "TH1.h"
#include "TH2.h"
#include "TNamed.h"
#include "TGraph.h"
#include "TGraph2D.h"
#include "TProfile.h"
#include "TProfile2D.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TKey.h"
#include "TClass.h"

#include <boost/regex.hpp>
#include <vector>
#include <map>
#include <string>
#include <cassert>
#include <iostream>
#include <algorithm>

using std::string;
using std::map;
using std::vector;
using std::exception;
using std::sort;

#include "TFile.h"

class CompareTNamedMore:
   public std::binary_function<
   const TNamed*,
   const TNamed*,
   Bool_t>
{
public:
   Bool_t operator()(const TNamed* object1, const TNamed* object2) const {
      return string(object1->GetName()) > string(object2->GetName());
   }
};

CbmHistManager::CbmHistManager():
      fMap(), fCanvases()
{

}

CbmHistManager::~CbmHistManager()
{

}

template<class T> vector<T> CbmHistManager::ObjectVector(
         const string& pattern) const
{
  vector<T> objects;

  try {
	const boost::regex e(pattern);
	map<string, TNamed*>::const_iterator it;
	for (it = fMap.begin(); it != fMap.end(); it++) {
		if (boost::regex_match(it->first, e)){
			T ObjectPointer = dynamic_cast<T>(it->second);
			if (ObjectPointer != nullptr)
				objects.push_back(ObjectPointer);
		}

	}
  } catch (exception& ex) {
    LOG(INFO) << "Exception in CbmHistManager::ObjectVector: " << ex.what() << FairLogger::endl;
  }

  sort(objects.begin(), objects.end(), CompareTNamedMore());
  return objects;
}

vector<TH1*> CbmHistManager::H1Vector(
      const string& pattern) const
{
	return ObjectVector<TH1*>(pattern);
}

vector<TH2*> CbmHistManager::H2Vector(
      const string& pattern) const
{
	return ObjectVector<TH2*>(pattern);
}

vector<TGraph*> CbmHistManager::G1Vector(
      const string& pattern) const
{
	return ObjectVector<TGraph*>(pattern);
}

vector<TGraph2D*> CbmHistManager::G2Vector(
      const string& pattern) const
{
	return ObjectVector<TGraph2D*>(pattern);
}

vector<TProfile*> CbmHistManager::P1Vector(
      const string& pattern) const
{
   return ObjectVector<TProfile*>(pattern);
}

vector<TProfile2D*> CbmHistManager::P2Vector(
      const string& pattern) const
{
   return ObjectVector<TProfile2D*>(pattern);
}

void CbmHistManager::WriteToFile()
{
	map<string, TNamed*>::iterator it;
	for (it = fMap.begin(); it != fMap.end(); it++){
		it->second->Write();
	}
}

void CbmHistManager::WriteToFileNew(const string& str)
{
	std::string histoName = "";
	map<string, TNamed*>::iterator it;
	TDirectory * oldir = gDirectory;
	TFile *fHist = new TFile(str.c_str(),"RECREATE");
	fHist->cd();

	TDirectory *cdHistosUpAndDown = fHist->mkdir("HistosUpAndDown");
	cdHistosUpAndDown->cd();
	for (it = fMap.begin(); it != fMap.end(); it++){
		if ( it->first.find("Up") != std::string::npos || it->first.find("Down") != std::string::npos ) {
			if ( !((it->first.find("Pi") != std::string::npos) || (it->first.find("Plus") != std::string::npos) || (it->first.find("Minus") != std::string::npos)) ) {
				it->second->Write();
				std::cout << "Histogram: " << it->first << std::endl;
			}
		}
	}

	TDirectory *cdHistosQa = fHist->mkdir( "HistosQa" );
	cdHistosQa->cd();
	for (it = fMap.begin(); it != fMap.end(); it++){
		if ( !(it->first.find("Up") != std::string::npos || it->first.find("Down") != std::string::npos) ) {
			if ( it->first.find("Pi") == std::string::npos ) { it->second->Write(); }
		}
	}

	fHist->cd(); // make the file root the current directory

	gDirectory->cd( oldir->GetPath() );
	fHist->Close();
}

void CbmHistManager::ReadFromFile(
      TFile* file)
{
   assert(file != NULL);
   LOG(INFO) << "CbmHistManager::ReadFromFile" << FairLogger::endl;
   TDirectory* dir = gDirectory;
   TIter nextkey(dir->GetListOfKeys());
   TKey *key;
//   Int_t c = 0;
   while ((key = (TKey*) nextkey())) {
      TObject* obj = key->ReadObj();
      if (obj->IsA()->InheritsFrom (TH1::Class()) || obj->IsA()->InheritsFrom (TGraph::Class()) || obj->IsA()->InheritsFrom (TGraph2D::Class())) {
         TNamed* h = (TNamed*) obj;
         TNamed* h1 = (TNamed*)file->Get(h->GetName());
         Add(string(h->GetName()), h1);
         //LOG(INFO) << c++ << " " << h->GetName()<< FairLogger::endl;
      }
   }
}

void CbmHistManager::Clear(Option_t*)
{
   map<string, TNamed*>::iterator it;
   for (it = fMap.begin(); it != fMap.end(); it++) {
      delete (*it).second;
   }
   fMap.clear();
}

void CbmHistManager::ShrinkEmptyBinsH1(
      const string& histName)
{
   TH1* hist = H1(histName);
   Int_t nofBins = hist->GetNbinsX();
   Int_t minShrinkBin = std::numeric_limits<Int_t>::max();
   Int_t maxShrinkBin = std::numeric_limits<Int_t>::min();
   Bool_t isSet = false;
   for (Int_t iBin = 1; iBin <= nofBins; iBin++) {
      Double_t content = hist->GetBinContent(iBin);
      if (content != 0.) {
         minShrinkBin = std::min(iBin, minShrinkBin);
         maxShrinkBin = std::max(iBin, maxShrinkBin);
         isSet = true;
      }
   }
   if (isSet) {
      hist->GetXaxis()->SetRange(minShrinkBin, maxShrinkBin);
     // hist->GetYaxis()->SetRange(minShrinkBin, maxShrinkBin);
   }
}

void CbmHistManager::ShrinkEmptyBinsH1ByPattern(
      const string& pattern)
{
	vector<TH1*> effHistos = H1Vector(pattern);
	Int_t nofEffHistos = effHistos.size();
	for (Int_t iHist = 0; iHist < nofEffHistos; iHist++) {
		ShrinkEmptyBinsH1(effHistos[iHist]->GetName());
	}
}

void CbmHistManager::ShrinkEmptyBinsH2(
      const string& histName)
{
   TH1* hist = H2(histName);
   Int_t nofBinsX = hist->GetNbinsX();
   Int_t nofBinsY = hist->GetNbinsY();
   Int_t minShrinkBinX = std::numeric_limits<Int_t>::max();
   Int_t maxShrinkBinX = std::numeric_limits<Int_t>::min();
   Int_t minShrinkBinY = std::numeric_limits<Int_t>::max();
   Int_t maxShrinkBinY = std::numeric_limits<Int_t>::min();
   Bool_t isSet = false;
   for (Int_t iBinX = 1; iBinX <= nofBinsX; iBinX++) {
      for (Int_t iBinY = 1; iBinY <= nofBinsY; iBinY++) {
         Double_t content = hist->GetBinContent(iBinX, iBinY);
         if (content != 0.) {
            minShrinkBinX = std::min(iBinX, minShrinkBinX);
            maxShrinkBinX = std::max(iBinX, maxShrinkBinX);
            minShrinkBinY = std::min(iBinY, minShrinkBinY);
            maxShrinkBinY = std::max(iBinY, maxShrinkBinY);
            isSet = true;
         }
      }
   }
   if (isSet) {
      hist->GetXaxis()->SetRange(minShrinkBinX, maxShrinkBinX);
      hist->GetYaxis()->SetRange(minShrinkBinY, maxShrinkBinY);
   }
}

void CbmHistManager::ShrinkEmptyBinsH2ByPattern(
      const string& pattern)
{
   vector<TH1*> effHistos = H1Vector(pattern);
   Int_t nofEffHistos = effHistos.size();
   for (Int_t iHist = 0; iHist < nofEffHistos; iHist++) {
      ShrinkEmptyBinsH2(effHistos[iHist]->GetName());
   }
}

void CbmHistManager::Scale(
      const string& histName,
      Double_t scale)
{
	H1(histName)->Scale(scale);
}

void CbmHistManager::ScaleByPattern(
      const string& pattern,
      Double_t scale)
{
	vector<TH1*> effHistos = H1Vector(pattern);
	Int_t nofEffHistos = effHistos.size();
	for (Int_t iHist = 0; iHist < nofEffHistos; iHist++) {
		Scale(effHistos[iHist]->GetName(), scale);
	}
}

void CbmHistManager::NormalizeToIntegral(
      const string& histName)
{
   TH1* hist = H1(histName);
   hist->Scale(1. / hist->Integral());
}

void CbmHistManager::NormalizeToIntegralByPattern(
      const string& pattern)
{
   vector<TH1*> effHistos = H1Vector(pattern);
   Int_t nofEffHistos = effHistos.size();
   for (Int_t iHist = 0; iHist < nofEffHistos; iHist++) {
      NormalizeToIntegral(effHistos[iHist]->GetName());
   }
}

void CbmHistManager::Rebin(
      const string& histName,
      Int_t ngroup)
{
	TH1* hist = H1(histName);
	if (ngroup > 1) {
		hist->Rebin(ngroup);
		hist->Scale(1. / (Double_t)ngroup);
	}
}

void CbmHistManager::RebinByPattern(
      const string& pattern,
      Int_t ngroup)
{
	vector<TH1*> effHistos = H1Vector(pattern);
	Int_t nofEffHistos = effHistos.size();
	for (Int_t iHist = 0; iHist < nofEffHistos; iHist++) {
		Rebin(effHistos[iHist]->GetName(), ngroup);
	}
}

string CbmHistManager::ToString() const
{
	string str = "CbmHistManager list of histograms:\n";
	map<string, TNamed*>::const_iterator it;
	for (it = fMap.begin(); it != fMap.end(); it++){
		str += it->first + "\n";
	}
	return str;
}

TCanvas* CbmHistManager::CreateCanvas(
		const std::string& name,
		const std::string& title,
		Int_t width,
		Int_t height)
{
	TCanvas* c = new TCanvas(name.c_str(), title.c_str(), width, height);
	fCanvases.push_back(c);
	return c;
}

void CbmHistManager::SaveCanvasToImage(
		const std::string& outputDir,
		const std::string& options)
{
	for (unsigned int i = 0; i < fCanvases.size(); i++) {
		Cbm::SaveCanvasAsImage(fCanvases[i], outputDir, options);
	}
}

ClassImp(CbmHistManager)

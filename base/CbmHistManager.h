/**
 * \file CbmHistManager.h
 * \brief Histogram manager.
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2011
 */

#ifndef CBMHISTMANAGER_H_
#define CBMHISTMANAGER_H_

#include "TObject.h"

#include "FairLogger.h"

#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TGraph.h"
#include "TGraph2D.h"
#include "TProfile.h"
#include "TProfile2D.h"
#include "TCanvas.h"

#include <map>
#include <string>
#include <vector>
#include <cassert>
#include <utility>
#include <functional>

class TFile;
class TNamed;

/**
 * \class CbmHistManager
 * \brief Histogram manager.
 * \author Semen Lebedev <s.lebedev@gsi.de>
 * \date 2011
 */
class CbmHistManager : public TObject
{
public:

   /**
    * \brief Constructor.
    */
   CbmHistManager();

   /**
    * \brief Destructor.
    */
   virtual ~CbmHistManager();

   /**
    * \brief Add new named object to manager.
    * \param[in] name Name of the object.
    * \param[in] object Pointer to object.
    */
   void Add(
         const std::string& name,
         TNamed* object) {

       std::map<std::string, TNamed*>::iterator it = fMap.find(name);
       if (it != fMap.end()){
           LOG(WARNING) << "CbmHistManager::Add Object with name:" << name << " was already added. Set new object." << FairLogger::endl;
       }

       std::pair<std::string, TNamed*> newpair = std::make_pair(name, object);
       fMap.insert(newpair);
   }

   /**
    * \brief Helper function for creation of 1-dimensional histograms and profiles.
    * Template argument is a real object type that has to be created, for example,
    * Create1<TH1F>("name", "title", 100, 0, 100);
    * \param[in] name Object name.
    * \param[in] title Object title.
    * \param[in] nofBins Number of bins.
    * \param[in] minBin Low axis limit.
    * \param[in] maxBin Upper axis limit.
    */
   template<class T> void Create1(
         const std::string& name,
         const std::string& title,
         Int_t nofBins,
         Double_t minBin,
         Double_t maxBin) {
		T* h = new T(name.c_str(), title.c_str(), nofBins, minBin, maxBin);
		Add(name, h);
	}

   /**
    * \brief Helper function for creation of 2-dimensional histograms and profiles.
    * Template argument is a real object type that has to be created, for example,
    * Create2<TH2F>("name", "title", 100, 0, 100, 200, 0, 200);
    * \param[in] name Object name.
    * \param[in] title Object title.
    * \param[in] nofBinsX Number of bins for X axis.
    * \param[in] minBinX Low X axis limit.
    * \param[in] maxBinX Upper X axis limit.
    * \param[in] nofBinsY Number of bins for Y axis.
    * \param[in] minBinY Low Y axis limit.
    * \param[in] maxBinY Upper Y axis limit.
    */
   template<class T> void Create2(
         const std::string& name,
         const std::string& title,
         Int_t nofBinsX,
         Double_t minBinX,
         Double_t maxBinX,
         Int_t nofBinsY,
         Double_t minBinY,
         Double_t maxBinY) {
   	T* h = new T(name.c_str(), title.c_str(), nofBinsX, minBinX, maxBinX, nofBinsY, minBinY, maxBinY);
   	Add(name, h);
   }

   /**
    * \brief Helper function for creation of 3-dimensional histograms and profiles.
    * Template argument is a real object type that has to be created, for example,
    * Create3<TH3F>("name", "title", 100, 0, 100, 200, 0, 200, 300, 0, 300);
    * \param[in] name Object name.
    * \param[in] title Object title.
    * \param[in] nofBinsX Number of bins for X axis.
    * \param[in] minBinX Low X axis limit.
    * \param[in] maxBinX Upper X axis limit.
    * \param[in] nofBinsY Number of bins for Y axis.
    * \param[in] minBinY Low Y axis limit.
    * \param[in] maxBinY Upper Y axis limit.
    * \param[in] nofBinsZ Number of bins for Z axis.
    * \param[in] minBinZ Low Z axis limit.
    * \param[in] maxBinZ Upper Z axis limit.
    */
   template<class T> void Create3(
         const std::string& name,
         const std::string& title,
         Int_t nofBinsX,
         Double_t minBinX,
         Double_t maxBinX,
         Int_t nofBinsY,
         Double_t minBinY,
         Double_t maxBinY,
		 Int_t nofBinsZ,
		 Double_t minBinZ,
		 Double_t maxBinZ) {
   	T* h = new T(name.c_str(), title.c_str(), nofBinsX, minBinX, maxBinX, nofBinsY, minBinY, maxBinY, nofBinsZ, minBinZ, maxBinZ);
   	Add(name, h);
   }

   /**
    * \brief Return pointer to TH1 histogram.
    * \param[in] name Name of histogram.
    * \return pointer to TH1 histogram.
    */
   TH1* H1(
         const std::string& name) const {
      if (fMap.count(name) == 0) { // Temporarily used for debugging
    	  LOG(ERROR) << "CbmHistManager::H1(name): name=" << name << FairLogger::endl;
      }
      assert(fMap.count(name) != 0);
      return dynamic_cast<TH1*>(fMap.find(name)->second);
   }

   /**
    * \brief Return vector of pointers to TH1 histogram.
    * \param[in] pattern Regex for histogram name.
    * \return Vector of pointers to TH1 histogram.
    */
   std::vector<TH1*> H1Vector(
         const std::string& pattern) const;

   /**
    * \brief Return pointer to TH2 histogram.
    * \param[in] name Name of histogram.
    * \return pointer to TH2 histogram.
    */
   TH2* H2(
         const std::string& name) const {
      if (fMap.count(name) == 0) { // Temporarily used for debugging
    	  LOG(ERROR) << "CbmHistManager::H2(name): name=" << name << FairLogger::endl;
      }
      assert(fMap.count(name) != 0);
      return dynamic_cast<TH2*>(fMap.find(name)->second);
   }

   /**
    * \brief Return vector of pointers to TH2 histogram.
    * \param[in] pattern Regex for histogram name.
    * \return Vector of pointers to TH2 histogram.
    */
   std::vector<TH2*> H2Vector(
         const std::string& pattern) const;

   /**
    * \brief Return pointer to TH3 histogram.
    * \param[in] name Name of histogram.
    * \return pointer to TH3 histogram.
    */
   TH3* H3(
         const std::string& name) const {
      if (fMap.count(name) == 0) { // Temporarily used for debugging
    	  LOG(ERROR) << "CbmHistManager::H3(name): name=" << name << FairLogger::endl;
      }
      assert(fMap.count(name) != 0);
      return dynamic_cast<TH3*>(fMap.find(name)->second);
   }

   /**
    * \brief Return pointer to TGraph.
    * \param[in] name Name of graph.
    * \return pointer to TGraph.
    */
   TGraph* G1(
         const std::string& name) const {
      if (fMap.count(name) == 0) { // Temporarily used for debugging
    	  LOG(ERROR) << "CbmHistManager::G1(name): name=" << name << FairLogger::endl;
      }
      assert(fMap.count(name) != 0);
      return dynamic_cast<TGraph*>(fMap.find(name)->second);
   }

   /**
    * \brief Return vector of pointers to TGraph.
    * \param[in] pattern Regex for object name.
    * \return Vector of pointers to TGraph.
    */
   std::vector<TGraph*> G1Vector(
         const std::string& pattern) const;

   /**
    * \brief Return pointer to TGraph2D.
    * \param[in] name Name of graph.
    * \return pointer to TGraph.
    */
   TGraph2D* G2(
         const std::string& name) const {
      if (fMap.count(name) == 0) { // Temporarily used for debugging
    	  LOG(ERROR) << "CbmHistManager::G2(name): name=" << name << FairLogger::endl;
      }
      assert(fMap.count(name) != 0);
      return dynamic_cast<TGraph2D*>(fMap.find(name)->second);
   }

   /**
    * \brief Return vector of pointers to TGraph2D.
    * \param[in] pattern Regex for object name.
    * \return Vector of pointers to TGraph2D.
    */
   std::vector<TGraph2D*> G2Vector(
         const std::string& pattern) const;

   /**
    * \brief Return pointer to TProfile.
    * \param[in] name Name of profile.
    * \return pointer to TProfile.
    */
   TProfile* P1(
         const std::string& name) const {
      if (fMap.count(name) == 0) { // Temporarily used for debugging
        LOG(ERROR) << "CbmHistManager::P1(name): name=" << name << FairLogger::endl;
      }
      assert(fMap.count(name) != 0);
      return dynamic_cast<TProfile*>(fMap.find(name)->second);
   }

   /**
    * \brief Return vector of pointers to TProfile.
    * \param[in] pattern Regex for profile name.
    * \return Vector of pointers to TProfile.
    */
   std::vector<TProfile*> P1Vector(
         const std::string& pattern) const;

   /**
    * \brief Return pointer to TH2 histogram.
    * \param[in] name Name of histogram.
    * \return pointer to TH1 histogram.
    */
   TProfile2D* P2(
         const std::string& name) const {
      if (fMap.count(name) == 0) { // Temporarily used for debugging
        LOG(ERROR) << "CbmHistManager::P2(name): name=" << name << FairLogger::endl;
      }
      assert(fMap.count(name) != 0);
      return dynamic_cast<TProfile2D*>(fMap.find(name)->second);
   }

   /**
    * \brief Return vector of pointers to TProfile2D.
    * \param[in] pattern Regex for profile name.
    * \return Vector of pointers to TProfile2D.
    */
   std::vector<TProfile2D*> P2Vector(
         const std::string& pattern) const;

   /**
    * \brief Check existence of histogram in manager.
    * \param[in] name Name of histogram.
    * \return True if histogram exists in manager.
    */
   Bool_t Exists(
         const std::string& name) const {
      return (fMap.count(name) == 0) ? false : true;
   }

   /**
    * \brief Write all histograms to current opened file.
    */
   void WriteToFile();

   /**
    * \brief Read histograms from file.
    * \param[in] file Pointer to file with histograms.
    */
   void ReadFromFile(
         TFile* file);

   /**
    * \brief Add TName object to map. Used in ReadFromFile method.
    */
   void AddTNamedObject(
            TObject* obj);

   /**
    * \brief Add all TName objects to map in directory. Used in ReadFromFile method.
    */
   void AddTDirectoryObject(
            TObject* obj);

   /**
    * \brief Clear memory. Remove all histograms.
    */
   void Clear(Option_t*);

   /**
    * \brief Shrink empty bins in H1.
    * \param[in] histName Name of histogram.
    */
   void ShrinkEmptyBinsH1(
         const std::string& histName);

   /**
    * \brief Shrink empty bins in H1.
    * \param[in] histPatternName Regular expression for histogram name.
    */
   void ShrinkEmptyBinsH1ByPattern(
         const std::string& pattern);

   /**
    * \brief Shrink empty bins in H2.
    * \param[in] histName Name of histogram.
    */
   void ShrinkEmptyBinsH2(
         const std::string& histName);

   /**
    * \brief Shrink empty bins in H2.
    * \param[in] histPatternName Regular expression for histogram name.
    */
   void ShrinkEmptyBinsH2ByPattern(
         const std::string& pattern);

   /**
    * \brief Scale histogram.
    * \param[in] histName Name of histogram.
    * \param[in] scale Scaling factor.
    */
   void Scale(
         const std::string& histName,
         Double_t scale);

   /**
    * \brief Scale histograms which name matches specified pattern.
    * \param[in] histPatternName Regular expression for histogram name.
    * \param[in] scale Scaling factor.
    */
   void ScaleByPattern(
         const std::string& pattern,
         Double_t scale);

   /**
    * \brief Normalize histogram to integral.
    * \param[in] histName Name of histogram.
    */
   void NormalizeToIntegral(
         const std::string& histName);

   /**
    * \brief Normalize histograms to integral which name matches specified pattern.
    * \param[in] histPatternName Regular expression for histogram name.
    */
   void NormalizeToIntegralByPattern(
         const std::string& pattern);

   /**
    * \brief Rebin histogram.
    * \param[in] histName Name of histogram.
    * \param[in] ngroup Rebining factor.
    */
   void Rebin(
         const std::string& histName,
         Int_t ngroup);

   /**
    * \brief Rebin histograms which name matches specified pattern.
    * \param[in] histPatternName Regular expression for histogram name.
    * \param[in] ngroup Rebining factor.
    */
   void RebinByPattern(
         const std::string& pattern,
         Int_t ngroup);

   /**
    * \brief Return string representation of class.
    * \return string representation of class.
    */
   std::string ToString() const;

   /**
    * \brief Operator << for convenient output to std::ostream.
    * \return Insertion stream in order to be able to call a succession of insertion operations.
    */
   friend std::ostream& operator<<(std::ostream& strm, const CbmHistManager& histManager) {
      strm << histManager.ToString();
      return strm;
   }

   /**
    * \brief Create and draw TCanvas and store pointer to it.
    * \param[in] name Name of the canvas.
    * \param[in] title Title of the canvas.
    * \param[in] width Width of the canvas.
    * \param[in] height Height of the canvas.
    * \return Pointer to the created canvas.
    */
   TCanvas* CreateCanvas(
   		const std::string& name,
   		const std::string& title,
   		Int_t width,
   		Int_t height);

   /**
    * \brief Save all stored canvases to images.
    * \param[in] outputDir Path to the output directory (could be relative path).
    * \param[in] options You can specify image format: eps, png or gif. Example: "gif,eps,png".
    */
   void SaveCanvasToImage(
   		const std::string& outputDir,
		const std::string& options = "png,eps");

private:
   template<class T> std::vector<T> ObjectVector(
         const std::string& pattern) const;

   std::map<std::string, TNamed*> fMap; // Map of histogram (graph) name to its pointer
   std::vector<TCanvas*> fCanvases; // Pointers to all created canvases

   ClassDef(CbmHistManager, 1)
};

#endif /* CBMHISTMANAGER_H_ */

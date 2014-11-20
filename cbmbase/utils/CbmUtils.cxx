#include "CbmUtils.h"

#include "TCanvas.h"
#include "TSystem.h"

namespace Cbm
{

void SaveCanvasAsImage(
   TCanvas* c,
   const std::string& dir,
   const std::string& option)
{
   if (dir == "") return;
   gSystem->mkdir(dir.c_str(), true); // create directory if it does not exist
   if (option.find("eps") != std::string::npos) c->SaveAs(std::string(dir + std::string(c->GetTitle()) + ".eps").c_str());
   if (option.find("png") != std::string::npos) c->SaveAs(std::string(dir + std::string(c->GetTitle()) + ".png").c_str());
   if (option.find("gif") != std::string::npos) c->SaveAs(std::string(dir + std::string(c->GetTitle()) + ".gif").c_str());
}

string FindAndReplace(
		const string& name,
		const string& oldSubstr,
		const string& newSubstr)
{
	string newName = name;
	Int_t startPos = name.find(oldSubstr);
	newName.replace(startPos, oldSubstr.size(), newSubstr);
	return newName;
}

vector<string> Split(
		const string& name,
		char delimiter)
{
	vector<string> result;
	Int_t begin = 0;
	Int_t end = name.find_first_of(delimiter);
	while (end != string::npos) {
		string str = name.substr(begin, end - begin);
		if (str[0] == delimiter) str.erase(0, 1);
		result.push_back(str);
		begin = end;
		end = name.find_first_of(delimiter, end + 1);
	}
	result.push_back(name.substr(begin + 1));
	return result;
}

}

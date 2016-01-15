using namespace std;
void create_rich_geo_with_mirror_misalignment_jordan(double sigma = 1, int geom_nb = 0)
{
	string vmcDir = string(gSystem->Getenv("VMCWORKDIR"));

	char PATH[256];
	sprintf(PATH, "/geometry/rich/mirror_alignment/rich_v14b_misaligned_%d.gdml", geom_nb);
	string inGeoFileName = vmcDir + "/geometry/rich/rich_v14b.gdml";
	string outGeoFileName = vmcDir + PATH;
	double sigmaGeg = sigma; // sigma for the Gauss distribution of misaligned mirrors in deg.

	ifstream inGeoFile(inGeoFileName.c_str());
	ofstream outGeoFile(outGeoFileName.c_str());
	TRandom3* random = new TRandom3();
	double Var_Random;
	string line;

	if (inGeoFile.is_open()) {
		while (getline (inGeoFile, line) ) {
			size_t found1 = line.find("<variable name=\"misalignment_");
			if (found1 != string::npos) { // select string in which misalignment values are set
				int control = 0;
				size_t found2 = line.find("value=");
				string newStr = line.substr(0, found2);
				stringstream ss;

				cout << "**********************" << endl;
				while (control != 1) {
					Var_Random = random->Gaus(0., sigmaGeg);
					cout << "Random : " << Var_Random << endl;
					if ((Var_Random<-1) || (Var_Random>1)) {continue;}
					else {
						ss << newStr << "value=\"" << Var_Random << "\"/>";
						control = 1;
					}
				}
				newStr = ss.str();
				outGeoFile << newStr << endl;

			} else {
				outGeoFile << line;
			}
		}
		inGeoFile.close();
		outGeoFile.close();
	}

	cout << "New geometry file is created: " << outGeoFileName << endl;
}

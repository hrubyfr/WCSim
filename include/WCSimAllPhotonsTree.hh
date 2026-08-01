#ifndef WCSIMALLPHOTONSTREE_HH
#define WCSIMALLPHOTONSTREE_HH

#include "TTree.h"
#include "TFile.h"   // for TFile* in AllPhotonsTree_Book signature
#include <string>

// Tree pointer
extern TTree* all_photons_tree;

// Branch variables
extern int   a_evt;
extern int   a_trk;
extern int   a_par;
extern int   a_pdg;

extern int   a_muAncestorID;

extern float a_edir_x;
extern float a_edir_y;
extern float a_edir_z;

extern float a_ex_cm;
extern float a_ey_cm;
extern float a_ez_cm;
extern float a_et_ns;

// Existing: muon's initial KE (as currently used)
extern float a_Estart_MeV;

// NEW: muon's KE at photon emission step
extern float a_muKE_MeV;

// NEW: muon's step index at photon emission
extern int   a_muStep;

extern float a_lambda_nm;

extern std::string a_creator;

extern int a_hit_pmt;
extern std::string a_end_boundary;
extern std::string a_end_process;


// lifecycle
void AllPhotonsTree_Book(TFile* fout);  // create tree + branches, attach to file
void AllPhotonsTree_Fill();             // just calls tree->Fill()
void AllPhotonsTree_Write();            // write tree (optional but recommended)

#endif

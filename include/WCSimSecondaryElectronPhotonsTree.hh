#ifndef WCSIMSECONDARYELECTRONPHOTONSTREE_HH
#define WCSIMSECONDARYELECTRONPHOTONSTREE_HH

#include "TTree.h"
#include "TFile.h"
#include <string>

// ==========================================
// Cherenkov photons from secondary electrons
// ==========================================

extern TTree* secondary_electron_photons_tree;

// Branch variables
extern int   sep_evt;
extern int   sep_trk;        // optical photon track ID
extern int   sep_parent;     // parent track ID (the emitting electron)
extern int   sep_pdg;        // optical photon PDG / encoding

// Explicit association back to the emitting secondary electron
extern int   sep_sec_e_trk;

// Photon start point [cm] and time [ns]
extern float sep_sx_cm;
extern float sep_sy_cm;
extern float sep_sz_cm;
extern float sep_st_ns;

// Photon direction at creation
extern float sep_dir_x;
extern float sep_dir_y;
extern float sep_dir_z;

// Photon wavelength [nm]
extern float sep_lambda_nm;

// End / hit information
extern int         sep_hit_pmt;
extern std::string sep_end_boundary;
extern std::string sep_end_process;
extern std::string sep_creator;

void SecondaryElectronPhotonsTree_Book(TFile* fout);
void SecondaryElectronPhotonsTree_Fill();
void SecondaryElectronPhotonsTree_Write();

#endif

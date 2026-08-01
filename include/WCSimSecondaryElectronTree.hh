#ifndef WCSIMSECONDARYELECTRONTREE_HH
#define WCSIMSECONDARYELECTRONTREE_HH

#include "TTree.h"
#include "TFile.h"
#include <string>

// =========================
// Secondary electron tree
// =========================

extern TTree* secondary_electron_tree;

// Branch variables
extern int   se_evt;
extern int   se_trk;
extern int   se_parent;
extern int   se_pdg;

// Start (creation) position/time [cm]/[ns]
extern float se_x0_cm;
extern float se_y0_cm;
extern float se_z0_cm;
extern float se_t0_ns;

// Initial kinetic energy [MeV]
extern float se_ke0_MeV;

// Initial momentum [MeV/c]
extern float se_p0_MeVc;
extern float se_px0_MeVc;
extern float se_py0_MeVc;
extern float se_pz0_MeVc;

// End (termination) position/time/KE [cm]/[ns]/[MeV]
extern float se_x1_cm;
extern float se_y1_cm;
extern float se_z1_cm;
extern float se_t1_ns;
extern float se_ke1_MeV;

// Bookkeeping strings
extern std::string se_creator;      // creator process of the electron track
extern std::string se_end_process;  // process that ended the electron track
extern std::string se_end_volume;   // volume where the track ended

void SecondaryElectronTree_Book(TFile* fout);
void SecondaryElectronTree_Fill();
void SecondaryElectronTree_Write();

#endif

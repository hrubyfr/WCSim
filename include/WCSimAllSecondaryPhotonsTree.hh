#ifndef WCSIM_ALL_SECONDARY_PHOTONS_TREE_HH
#define WCSIM_ALL_SECONDARY_PHOTONS_TREE_HH

#include "TFile.h"
#include "TTree.h"
#include <string>

extern TTree* all_secondary_photons_tree;

extern int   asph_evt;
extern int   asph_trk;
extern int   asph_parent;
extern int   asph_parent_pdg;
extern int   asph_parent_parent;
extern int   asph_parent_step;
extern int   asph_pdg;

extern float asph_parent_ke_MeV;
extern float asph_sx_cm;
extern float asph_sy_cm;
extern float asph_sz_cm;
extern float asph_st_ns;
extern float asph_dir_x;
extern float asph_dir_y;
extern float asph_dir_z;
extern float asph_lambda_nm;

extern int   asph_hit_pmt;
extern float asph_hit_x_cm;
extern float asph_hit_y_cm;
extern float asph_hit_z_cm;
extern float asph_hit_t_ns;

extern int   asph_made_pe;
extern int   asph_pe_pmt;
extern float asph_pe_time_ns;
extern float asph_pe_lambda_nm;

extern float asph_end_x_cm;
extern float asph_end_y_cm;
extern float asph_end_z_cm;
extern float asph_end_t_ns;
extern int   asph_end_status;

extern std::string asph_creator;
extern std::string asph_parent_creator;
extern std::string asph_hit_volume;
extern std::string asph_end_boundary;
extern std::string asph_end_process;

void AllSecondaryPhotonsTree_Book(TFile* fout);
void AllSecondaryPhotonsTree_Fill();
void AllSecondaryPhotonsTree_Write();
void AllSecondaryPhotonsTree_ResetVars();

// Per-event PE cache used to bridge WCSimWCSD QE acceptance back into the
// photon truth row filled by WCSimSteppingAction. This does not alter physics;
// it only records the result after the existing QE/collection-efficiency tests.
void AllSecondaryPhotonsTree_ResetEvent(int evt);
void AllSecondaryPhotonsTree_NotePE(int evt, int photon_trk, int pe_pmt,
                                    float pe_time_ns, float pe_lambda_nm);
bool AllSecondaryPhotonsTree_GetPE(int evt, int photon_trk, int& pe_pmt,
                                   float& pe_time_ns, float& pe_lambda_nm);

#endif

#ifndef WCSIM_ALL_SECONDARIES_TREE_HH
#define WCSIM_ALL_SECONDARIES_TREE_HH

#include "TFile.h"
#include "TTree.h"
#include <string>

extern TTree* all_secondaries_tree;

extern int   alls_evt;
extern int   alls_trk;
extern int   alls_parent;
extern int   alls_pdg;
extern int   alls_step;

extern float alls_x_cm;
extern float alls_y_cm;
extern float alls_z_cm;
extern float alls_t_ns;

extern float alls_post_x_cm;
extern float alls_post_y_cm;
extern float alls_post_z_cm;
extern float alls_post_t_ns;

extern float alls_dir_x;
extern float alls_dir_y;
extern float alls_dir_z;

extern float alls_post_dir_x;
extern float alls_post_dir_y;
extern float alls_post_dir_z;

extern float alls_ke_MeV;
extern float alls_post_ke_MeV;
extern float alls_edep_MeV;
extern float alls_step_length_cm;
extern float alls_track_length_cm;
extern float alls_charge;

extern std::string alls_particle;
extern std::string alls_creator;
extern std::string alls_step_process;
extern std::string alls_volume;
extern std::string alls_post_volume;
extern std::string alls_material;
extern std::string alls_post_material;

void AllSecondariesTree_Book(TFile* fout);
void AllSecondariesTree_Fill();
void AllSecondariesTree_Write();
void AllSecondariesTree_ResetVars();

#endif

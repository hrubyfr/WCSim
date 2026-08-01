#ifndef WCSIMPHOTONTREE_HH
#define WCSIMPHOTONTREE_HH

#include "TTree.h"
#include <string>

// Tree pointer
extern TTree* pmt_photon_tree;

// Branch variables
extern int   b_evt;
extern int   b_pmt;
extern int   b_trk;
extern int   b_par;
extern int   b_pdg;

extern int b_muAncestorID;

extern float b_edir_x;
extern float b_edir_y;
extern float b_edir_z;

extern float b_cos_inc;
extern float b_theta_inc_deg;

extern float b_ex_cm, b_ey_cm, b_ez_cm, b_et_ns;
extern float b_dx_cm, b_dy_cm, b_dz_cm, b_dt_ns;

extern float b_Estart_MeV, b_Eend_MeV, b_lambda_nm;

extern std::string b_creator;

#endif


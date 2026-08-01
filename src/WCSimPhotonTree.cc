#include "WCSimPhotonTree.hh"

// Tree pointer
TTree* pmt_photon_tree = nullptr;

// Branch variables
int   b_evt = -1;
int   b_pmt = -1;
int   b_trk = -1;
int   b_par = -1;
int   b_pdg = -1;
int b_muAncestorID = -1;

float b_edir_x = 0.f;
float b_edir_y = 0.f;
float b_edir_z = 0.f;

float b_cos_inc = 0.f;
float b_theta_inc_deg = 0.f;

float b_ex_cm = 0.f, b_ey_cm = 0.f, b_ez_cm = 0.f, b_et_ns = 0.f;
float b_dx_cm = 0.f, b_dy_cm = 0.f, b_dz_cm = 0.f, b_dt_ns = 0.f;

float b_Estart_MeV = 0.f, b_Eend_MeV = 0.f, b_lambda_nm = 0.f;

std::string b_creator = "";


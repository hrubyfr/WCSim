// WCSimPETree.cc
#include "WCSimPETree.hh"

TTree* pe_tree = nullptr;

int   pe_event   = -1;
int   pe_trigger = 0;
int   pe_tubeid  = -1;
float pe_time_ns = 0.f;
float pe_lambda_nm = 0.f;
int   pe_trackid = -1;

float pe_cos_inc =0.f;

void ResetPETreeVars() {
  pe_event   = -1;
  pe_trigger = 0;
  pe_tubeid  = -1;
  pe_time_ns = 0.f;
  pe_lambda_nm = 0.f;
  pe_trackid = -1;
  pe_cos_inc =0.f;
}



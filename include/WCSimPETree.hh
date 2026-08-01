// WCSimPETree.hh
#ifndef WCSIM_PE_TREE_HH
#define WCSIM_PE_TREE_HH

#include "TTree.h"

// Global tree pointer (same pattern as your photon tree)
extern TTree* pe_tree;

// Branch buffers
extern int   pe_event;
extern int   pe_trigger;    // set to 0 if you don't use triggers/subevents
extern int   pe_tubeid;     // WCSim tube ID (same convention as true_hit_pmt)
extern float pe_time_ns;
extern float pe_lambda_nm;
extern int   pe_trackid;    // optional but useful
extern float pe_cos_inc;
// Optional helper to reset per-entry values (optional)
void ResetPETreeVars();

#endif


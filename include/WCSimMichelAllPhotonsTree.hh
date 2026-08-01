#ifndef WCSIMMICHELALLPHOTONSTREE_HH
#define WCSIMMICHELALLPHOTONSTREE_HH

#include "TTree.h"
#include "TFile.h"   // for TFile* in *_Book signatures
#include <string>
#include "G4VUserTrackInformation.hh"
#include "G4ThreeVector.hh"
#include "G4String.hh"

class MichelPhotonTrackInfo : public G4VUserTrackInformation {
public:
  MichelPhotonTrackInfo() = default;
  virtual ~MichelPhotonTrackInfo() = default;

  // birth (production) info
  int michel_trk = -1;
  int parent_id  = -1;
  int pdg        = 0;          // optical photon = 0 in your tree
  G4ThreeVector s_pos;         // cm later
  double s_t_ns  = 0.0;
  G4ThreeVector dir;
  double lambda_nm = -1.0;
  G4String creator;

  // bookkeeping
  bool filled = false;
};


// ============================
// Michel Cherenkov photons tree
// ============================

// Tree pointer

extern TTree* michel_photons_tree;

extern int  g_michel_trk;
extern bool g_michel_electron_started;
extern int  g_wcsim_evt;

// Branch variables
extern int   mcp_evt;        // event index
extern int   mcp_trk;        // photon track ID (Geant4 track ID)
extern int   mcp_parent;     // parent track ID
extern int   mcp_pdg;        // photon PDG (should be 0 / optical photon in WCSim convention, but stored anyway)

// Michel electron association
extern int   mcp_michel_trk; // track ID of the Michel e- that produced this photon (ancestor/creator track)

// Photon start point (production vertex) [cm] and time [ns]
extern float mcp_sx_cm;
extern float mcp_sy_cm;
extern float mcp_sz_cm;
extern float mcp_st_ns;

// Photon direction at creation (unit vector)
extern float mcp_dir_x;
extern float mcp_dir_y;
extern float mcp_dir_z;

// Photon wavelength [nm]
extern float mcp_lambda_nm;

// Where it ends / what it hits
extern int         mcp_hit_pmt;        // PMT ID if detected, else -1
extern std::string mcp_end_boundary;   // name/label of boundary/volume/surface where it terminates
extern std::string mcp_end_process;    // optional: process at end (Absorption, Boundary, etc.)
extern std::string mcp_creator;        // creator process name (eCerenkov, Scintillation, etc.)

// lifecycle
void MichelPhotonsTree_Book(TFile* fout);  // create tree + branches, attach to file
void MichelPhotonsTree_Fill();             // calls tree->Fill()
void MichelPhotonsTree_Write();            // write tree (optional but recommended)


// ======================
// Michel electron tree
// ======================

// Tree pointer
extern TTree* michel_electron_tree;

// Branch variables
extern int   mce_evt;      // event index
extern int   mce_trk;      // Michel e- track ID
extern int   mce_parent;   // parent track ID (often mu-)
extern int   mce_pdg;      // PDG (11)

// Start (creation) position/time [cm]/[ns]
extern float mce_x0_cm;
extern float mce_y0_cm;
extern float mce_z0_cm;
extern float mce_t0_ns;

// Initial kinetic energy [MeV]
extern float mce_ke0_MeV;

// Optional end (stop) position/time/KE if you choose to fill it later
extern float mce_x1_cm;
extern float mce_y1_cm;
extern float mce_z1_cm;
extern float mce_t1_ns;
extern float mce_ke1_MeV;

// lifecycle
void MichelElectronTree_Book(TFile* fout);
void MichelElectronTree_Fill();
void MichelElectronTree_Write();

#endif

#include "WCSimMichelAllPhotonsTree.hh"

#include "TFile.h"       // REQUIRED for fout->cd()
#include "TTree.h"
#include "TDirectory.h"  // optional
#include "TROOT.h"       // optional

// ============================
// Michel Cherenkov photons tree
// ============================

TTree* michel_photons_tree = nullptr;

// define storage (initialize to safe defaults)
int   mcp_evt        = -1;
int   mcp_trk        = -1;
int   mcp_parent     = -1;
int   mcp_pdg        = -1;

int   mcp_michel_trk = -1;

float mcp_sx_cm = 0.f, mcp_sy_cm = 0.f, mcp_sz_cm = 0.f, mcp_st_ns = 0.f;
float mcp_dir_x = 0.f, mcp_dir_y = 0.f, mcp_dir_z = 0.f;

float mcp_lambda_nm = 0.f;

int         mcp_hit_pmt      = -1;
std::string mcp_end_boundary = "";
std::string mcp_end_process  = "";
std::string mcp_creator      = "";

void MichelPhotonsTree_Book(TFile* fout)
{
  if (michel_photons_tree) return;  // only once
  if (!fout) return;

  fout->cd(); // CRITICAL: attach tree to output file directory

  michel_photons_tree = new TTree("michel_photons", "Cherenkov photons from Michel electron");

  michel_photons_tree->Branch("evt",    &mcp_evt,    "evt/I");
  michel_photons_tree->Branch("trk",    &mcp_trk,    "trk/I");
  michel_photons_tree->Branch("parent", &mcp_parent, "parent/I");
  michel_photons_tree->Branch("pdg",    &mcp_pdg,    "pdg/I");

  michel_photons_tree->Branch("michel_trk", &mcp_michel_trk, "michel_trk/I");

  michel_photons_tree->Branch("sx_cm", &mcp_sx_cm, "sx_cm/F");
  michel_photons_tree->Branch("sy_cm", &mcp_sy_cm, "sy_cm/F");
  michel_photons_tree->Branch("sz_cm", &mcp_sz_cm, "sz_cm/F");
  michel_photons_tree->Branch("st_ns", &mcp_st_ns, "st_ns/F");

  michel_photons_tree->Branch("dir_x", &mcp_dir_x, "dir_x/F");
  michel_photons_tree->Branch("dir_y", &mcp_dir_y, "dir_y/F");
  michel_photons_tree->Branch("dir_z", &mcp_dir_z, "dir_z/F");

  michel_photons_tree->Branch("lambda_nm", &mcp_lambda_nm, "lambda_nm/F");

  michel_photons_tree->Branch("hit_pmt", &mcp_hit_pmt, "hit_pmt/I");

  // std::string branches (ROOT handles std::string fine via dictionary)
  michel_photons_tree->Branch("end_boundary", &mcp_end_boundary);
  michel_photons_tree->Branch("end_process",  &mcp_end_process);
  michel_photons_tree->Branch("creator",      &mcp_creator);
}

void MichelPhotonsTree_Fill()
{
  if (michel_photons_tree) michel_photons_tree->Fill();
}

void MichelPhotonsTree_Write()
{
  if (!michel_photons_tree) return;
  michel_photons_tree->Write("", TObject::kOverwrite);
}


// ======================
// Michel electron tree
// ======================

TTree* michel_electron_tree = nullptr;

// define storage (initialize to safe defaults)
int   mce_evt    = -1;
int   mce_trk    = -1;
int   mce_parent = -1;
int   mce_pdg    = 11;

float mce_x0_cm = 0.f, mce_y0_cm = 0.f, mce_z0_cm = 0.f, mce_t0_ns = 0.f;
float mce_ke0_MeV = 0.f;

float mce_x1_cm = 0.f, mce_y1_cm = 0.f, mce_z1_cm = 0.f, mce_t1_ns = 0.f;
float mce_ke1_MeV = 0.f;

//bool g_michel_electron_started = false;

int  g_michel_trk = -1;
bool g_michel_electron_started = false;
int  g_wcsim_evt = -1;

void MichelElectronTree_Book(TFile* fout)
{
  if (michel_electron_tree) return; // only once
  if (!fout) return;

  fout->cd();

  michel_electron_tree = new TTree("michel_electron", "Michel electron truth / kinematics");

  michel_electron_tree->Branch("evt",    &mce_evt,    "evt/I");
  michel_electron_tree->Branch("trk",    &mce_trk,    "trk/I");
  michel_electron_tree->Branch("parent", &mce_parent, "parent/I");
  michel_electron_tree->Branch("pdg",    &mce_pdg,    "pdg/I");

  michel_electron_tree->Branch("x0_cm", &mce_x0_cm, "x0_cm/F");
  michel_electron_tree->Branch("y0_cm", &mce_y0_cm, "y0_cm/F");
  michel_electron_tree->Branch("z0_cm", &mce_z0_cm, "z0_cm/F");
  michel_electron_tree->Branch("t0_ns", &mce_t0_ns, "t0_ns/F");

  michel_electron_tree->Branch("ke0_MeV", &mce_ke0_MeV, "ke0_MeV/F");

  // optional end-of-track info (you can leave defaults if you don’t fill it)
  michel_electron_tree->Branch("x1_cm", &mce_x1_cm, "x1_cm/F");
  michel_electron_tree->Branch("y1_cm", &mce_y1_cm, "y1_cm/F");
  michel_electron_tree->Branch("z1_cm", &mce_z1_cm, "z1_cm/F");
  michel_electron_tree->Branch("t1_ns", &mce_t1_ns, "t1_ns/F");

  michel_electron_tree->Branch("ke1_MeV", &mce_ke1_MeV, "ke1_MeV/F");
}

void MichelElectronTree_Fill()
{
  if (michel_electron_tree) michel_electron_tree->Fill();
}

void MichelElectronTree_Write()
{
  if (!michel_electron_tree) return;
  michel_electron_tree->Write("", TObject::kOverwrite);
}

#include "WCSimAllPhotonsTree.hh"
#include "TFile.h"   // <-- REQUIRED for fout->cd()
#include "TTree.h"
#include "TDirectory.h"  // optional, but fine
#include "TROOT.h"       // optional

TTree* all_photons_tree = nullptr;

// define storage
int   a_evt = -1, a_trk = -1, a_par = -1, a_pdg = -1, a_muAncestorID = -1;
float a_edir_x = 0.f, a_edir_y = 0.f, a_edir_z = 0.f;
float a_ex_cm = 0.f, a_ey_cm = 0.f, a_ez_cm = 0.f, a_et_ns = 0.f;

// Existing: this appears to be muon's *initial* KE (as you noted)
float a_Estart_MeV = 0.f;

// NEW: muon KE at the emission step (i.e. when photon is created)
float a_muKE_MeV = 0.f;

// NEW: muon step index at emission
int   a_muStep = -1;

float a_lambda_nm = 0.f;
std::string a_creator = "";

int a_hit_pmt = -1;
std::string a_end_boundary = "";
std::string a_end_process = "";

void AllPhotonsTree_Book(TFile* fout)
{
  if (all_photons_tree) return;     // only once
  if (!fout) return;

  fout->cd();                       // CRITICAL: attach tree to output file directory

  all_photons_tree = new TTree("all_photons", "Cherenkov photons from primary muon");

  all_photons_tree->Branch("evt", &a_evt, "evt/I");
  all_photons_tree->Branch("trk", &a_trk, "trk/I");
  all_photons_tree->Branch("par", &a_par, "par/I");
  all_photons_tree->Branch("pdg", &a_pdg, "pdg/I");
  all_photons_tree->Branch("muAncestorID", &a_muAncestorID, "muAncestorID/I");

  all_photons_tree->Branch("edir_x", &a_edir_x, "edir_x/F");
  all_photons_tree->Branch("edir_y", &a_edir_y, "edir_y/F");
  all_photons_tree->Branch("edir_z", &a_edir_z, "edir_z/F");

  all_photons_tree->Branch("ex_cm", &a_ex_cm, "ex_cm/F");
  all_photons_tree->Branch("ey_cm", &a_ey_cm, "ey_cm/F");
  all_photons_tree->Branch("ez_cm", &a_ez_cm, "ez_cm/F");
  all_photons_tree->Branch("et_ns", &a_et_ns, "et_ns/F");

  // Existing (initial muon KE, per your current usage)
  all_photons_tree->Branch("Estart_MeV", &a_Estart_MeV, "Estart_MeV/F");

  // NEW branches
  all_photons_tree->Branch("muKE_MeV", &a_muKE_MeV, "muKE_MeV/F");
  all_photons_tree->Branch("muStep",   &a_muStep,   "muStep/I");

  all_photons_tree->Branch("lambda_nm", &a_lambda_nm, "lambda_nm/F");
  all_photons_tree->Branch("creator", &a_creator);  // std::string branch
  all_photons_tree->Branch("hit_pmt", &a_hit_pmt, "hit_pmt/I");
  all_photons_tree->Branch("end_boundary", &a_end_boundary);
  all_photons_tree->Branch("end_process", &a_end_process);

}

void AllPhotonsTree_Fill()
{
  if (all_photons_tree) all_photons_tree->Fill();
}

void AllPhotonsTree_Write()
{
  if (!all_photons_tree) return;
  all_photons_tree->Write("", TObject::kOverwrite);
}

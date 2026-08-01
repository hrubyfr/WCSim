#include "WCSimSecondaryElectronPhotonsTree.hh"

#include "TFile.h"
#include "TTree.h"

TTree* secondary_electron_photons_tree = nullptr;

int   sep_evt      = -1;
int   sep_trk      = -1;
int   sep_parent   = -1;
int   sep_pdg      = 0;
int   sep_sec_e_trk = -1;

float sep_sx_cm = 0.f;
float sep_sy_cm = 0.f;
float sep_sz_cm = 0.f;
float sep_st_ns = 0.f;

float sep_dir_x = 0.f;
float sep_dir_y = 0.f;
float sep_dir_z = 0.f;

float sep_lambda_nm = 0.f;

int         sep_hit_pmt      = -1;
std::string sep_end_boundary = "";
std::string sep_end_process  = "";
std::string sep_creator      = "";

void SecondaryElectronPhotonsTree_Book(TFile* fout)
{
  if (secondary_electron_photons_tree) return;
  if (!fout) return;

  fout->cd();

  secondary_electron_photons_tree = new TTree(
      "secondary_e_photons",
      "Cherenkov photons produced by secondary electrons");

  secondary_electron_photons_tree->Branch("evt",    &sep_evt,    "evt/I");
  secondary_electron_photons_tree->Branch("trk",    &sep_trk,    "trk/I");
  secondary_electron_photons_tree->Branch("parent", &sep_parent, "parent/I");
  secondary_electron_photons_tree->Branch("pdg",    &sep_pdg,    "pdg/I");

  secondary_electron_photons_tree->Branch("sec_e_trk", &sep_sec_e_trk, "sec_e_trk/I");

  secondary_electron_photons_tree->Branch("sx_cm", &sep_sx_cm, "sx_cm/F");
  secondary_electron_photons_tree->Branch("sy_cm", &sep_sy_cm, "sy_cm/F");
  secondary_electron_photons_tree->Branch("sz_cm", &sep_sz_cm, "sz_cm/F");
  secondary_electron_photons_tree->Branch("st_ns", &sep_st_ns, "st_ns/F");

  secondary_electron_photons_tree->Branch("dir_x", &sep_dir_x, "dir_x/F");
  secondary_electron_photons_tree->Branch("dir_y", &sep_dir_y, "dir_y/F");
  secondary_electron_photons_tree->Branch("dir_z", &sep_dir_z, "dir_z/F");

  secondary_electron_photons_tree->Branch("lambda_nm", &sep_lambda_nm, "lambda_nm/F");

  secondary_electron_photons_tree->Branch("hit_pmt", &sep_hit_pmt, "hit_pmt/I");
  secondary_electron_photons_tree->Branch("end_boundary", &sep_end_boundary);
  secondary_electron_photons_tree->Branch("end_process",  &sep_end_process);
  secondary_electron_photons_tree->Branch("creator",      &sep_creator);
}

void SecondaryElectronPhotonsTree_Fill()
{
  if (secondary_electron_photons_tree) secondary_electron_photons_tree->Fill();
}

void SecondaryElectronPhotonsTree_Write()
{
  if (!secondary_electron_photons_tree) return;
  secondary_electron_photons_tree->Write("", TObject::kOverwrite);
}

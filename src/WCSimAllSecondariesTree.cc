#include "WCSimAllSecondariesTree.hh"

#include "TObject.h"
#include "TFile.h"

TTree* all_secondaries_tree = nullptr;

int   alls_evt    = -1;
int   alls_trk    = -1;
int   alls_parent = -1;
int   alls_pdg    = 0;
int   alls_step   = -1;

float alls_x_cm = 0.f;
float alls_y_cm = 0.f;
float alls_z_cm = 0.f;
float alls_t_ns = 0.f;

float alls_post_x_cm = 0.f;
float alls_post_y_cm = 0.f;
float alls_post_z_cm = 0.f;
float alls_post_t_ns = 0.f;

float alls_dir_x = 0.f;
float alls_dir_y = 0.f;
float alls_dir_z = 0.f;

float alls_post_dir_x = 0.f;
float alls_post_dir_y = 0.f;
float alls_post_dir_z = 0.f;

float alls_ke_MeV = 0.f;
float alls_post_ke_MeV = 0.f;
float alls_edep_MeV = 0.f;
float alls_step_length_cm = 0.f;
float alls_track_length_cm = 0.f;
float alls_charge = 0.f;

std::string alls_particle = "";
std::string alls_creator = "";
std::string alls_step_process = "";
std::string alls_volume = "";
std::string alls_post_volume = "";
std::string alls_material = "";
std::string alls_post_material = "";

void AllSecondariesTree_ResetVars()
{
  alls_evt    = -1;
  alls_trk    = -1;
  alls_parent = -1;
  alls_pdg    = 0;
  alls_step   = -1;

  alls_x_cm = alls_y_cm = alls_z_cm = alls_t_ns = 0.f;
  alls_post_x_cm = alls_post_y_cm = alls_post_z_cm = alls_post_t_ns = 0.f;
  alls_dir_x = alls_dir_y = alls_dir_z = 0.f;
  alls_post_dir_x = alls_post_dir_y = alls_post_dir_z = 0.f;

  alls_ke_MeV = 0.f;
  alls_post_ke_MeV = 0.f;
  alls_edep_MeV = 0.f;
  alls_step_length_cm = 0.f;
  alls_track_length_cm = 0.f;
  alls_charge = 0.f;

  alls_particle.clear();
  alls_creator.clear();
  alls_step_process.clear();
  alls_volume.clear();
  alls_post_volume.clear();
  alls_material.clear();
  alls_post_material.clear();
}

void AllSecondariesTree_Book(TFile* fout)
{
  if (all_secondaries_tree) return;
  if (!fout) return;

  fout->cd();
  all_secondaries_tree = new TTree("AllSecondaries", "Step-by-step truth for the event primary and all non-optical secondary particles");

  all_secondaries_tree->Branch("evt",    &alls_evt,    "evt/I");
  all_secondaries_tree->Branch("trk",    &alls_trk,    "trk/I");
  all_secondaries_tree->Branch("parent", &alls_parent, "parent/I");
  all_secondaries_tree->Branch("pdg",    &alls_pdg,    "pdg/I");
  all_secondaries_tree->Branch("step",   &alls_step,   "step/I");

  all_secondaries_tree->Branch("x_cm", &alls_x_cm, "x_cm/F");
  all_secondaries_tree->Branch("y_cm", &alls_y_cm, "y_cm/F");
  all_secondaries_tree->Branch("z_cm", &alls_z_cm, "z_cm/F");
  all_secondaries_tree->Branch("t_ns", &alls_t_ns, "t_ns/F");

  all_secondaries_tree->Branch("post_x_cm", &alls_post_x_cm, "post_x_cm/F");
  all_secondaries_tree->Branch("post_y_cm", &alls_post_y_cm, "post_y_cm/F");
  all_secondaries_tree->Branch("post_z_cm", &alls_post_z_cm, "post_z_cm/F");
  all_secondaries_tree->Branch("post_t_ns", &alls_post_t_ns, "post_t_ns/F");

  all_secondaries_tree->Branch("dir_x", &alls_dir_x, "dir_x/F");
  all_secondaries_tree->Branch("dir_y", &alls_dir_y, "dir_y/F");
  all_secondaries_tree->Branch("dir_z", &alls_dir_z, "dir_z/F");

  all_secondaries_tree->Branch("post_dir_x", &alls_post_dir_x, "post_dir_x/F");
  all_secondaries_tree->Branch("post_dir_y", &alls_post_dir_y, "post_dir_y/F");
  all_secondaries_tree->Branch("post_dir_z", &alls_post_dir_z, "post_dir_z/F");

  all_secondaries_tree->Branch("ke_MeV",          &alls_ke_MeV,          "ke_MeV/F");
  all_secondaries_tree->Branch("post_ke_MeV",     &alls_post_ke_MeV,     "post_ke_MeV/F");
  all_secondaries_tree->Branch("edep_MeV",        &alls_edep_MeV,        "edep_MeV/F");
  all_secondaries_tree->Branch("step_length_cm",  &alls_step_length_cm,  "step_length_cm/F");
  all_secondaries_tree->Branch("track_length_cm", &alls_track_length_cm, "track_length_cm/F");
  all_secondaries_tree->Branch("charge",          &alls_charge,          "charge/F");

  all_secondaries_tree->Branch("particle",      &alls_particle);
  all_secondaries_tree->Branch("creator",       &alls_creator);
  all_secondaries_tree->Branch("step_process",  &alls_step_process);
  all_secondaries_tree->Branch("volume",        &alls_volume);
  all_secondaries_tree->Branch("post_volume",   &alls_post_volume);
  all_secondaries_tree->Branch("material",      &alls_material);
  all_secondaries_tree->Branch("post_material", &alls_post_material);
}

void AllSecondariesTree_Fill()
{
  if (all_secondaries_tree) all_secondaries_tree->Fill();
}

void AllSecondariesTree_Write()
{
  if (!all_secondaries_tree) return;
  TFile* file = all_secondaries_tree->GetCurrentFile();
  if (!file) return;
  file->cd();
  all_secondaries_tree->Write("", TObject::kOverwrite);
}

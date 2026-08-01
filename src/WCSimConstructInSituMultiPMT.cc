// WCSimConstructInSituMultiPMT.cc  (clean, dual-build version)
// Builds in-situ mPMT logical volumes either with or without a 0.5 mm air shell
// directly under the acrylic dome. Gel then extends from the matrix to either
// the dome inner surface (no gap) or to the inner radius of the air shell (with gap).

#include "WCSimDetectorConstruction.hh"

#include "G4Box.hh"
#include "G4Sphere.hh"
#include "G4Cons.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4VisAttributes.hh"
#include "G4Material.hh"
#include "G4Polycone.hh"
#include "G4Tubs.hh"
#include "G4PVPlacement.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4LogicalBorderSurface.hh"

#include "G4SDManager.hh"
#include "WCSimWCSD.hh"

#include "G4SystemOfUnits.hh"
#include "WCSimMultiPMTParameterisation.hh"
#include "G4PVParameterised.hh"

// Debug file for mapping PMT -> mPMT + tube index
#include <fstream>
//static std::ofstream mapFile("wcsim_pmt_mpmts.csv");

// -----------------------------
// Local helper (private) builder
// -----------------------------
G4LogicalVolume*
WCSimDetectorConstruction::BuildInSituMultiPMT_Impl(G4String PMTName,
                                                    G4String CollectionName,
                                                    G4String detectorElement,
                                                    G4bool withAirGap)
{
  // Unique cache key so we only build each variant once
  G4String keyname = (withAirGap ? "InSitu-mPMT-AirGap" : "InSitu-mPMT-NoGap");
  PMTKey_t key(keyname, CollectionName);
  PMTMap_t::iterator it = PMTLogicalVolumes.find(key);
  if (it != PMTLogicalVolumes.end()) return it->second;

  // ---- Geometry params (as in your original tree) ----
  const G4double domeInnerRadius = 332.*mm;
  const G4double domeOuterRadius = 347.*mm;
  const G4double domeCut         = 235.*mm;

  const G4double vessel_cylinder_height = 77.785*2*mm;
  const G4double vessel_inner_radius    = 250.93*mm;
  const G4double vessel_outer_radius    = 254.*mm;

  // Matrix (inner shell where PMTs live)
  const G4double matrixRmin   = 263.003*mm;
  const G4double matrixRmax   = 325.603*mm;
  const G4double matrixUpshift= 5.7*mm;

  // Gel defaults (no gap: gel touches dome)
  const G4double gelRmin = 323.5*mm;

  // Air gap thickness when enabled
  const G4double kAirGapThickness = 0.5 * mm;
  const G4double gelRmax_noGap    = domeInnerRadius;
  const G4double gelRmax_withGap  = domeInnerRadius - kAirGapThickness;

  // --- BEGIN debug guards ---
G4cout << "[INSITU] BuildInSituMultiPMT_Impl start. withAirGap=" << (withAirGap?"yes":"no") << G4endl;
G4cout << "[INSITU] domeInnerRadius=" << domeInnerRadius/mm << " mm, domeOuterRadius=" << domeOuterRadius/mm
       << " mm, gelRmin=" << gelRmin/mm << " mm" << G4endl;

// Air gap thickness you intend
const G4double airGap = 0.5*mm;

if (withAirGap) {
  if (gelRmin + airGap >= domeInnerRadius) {
    G4Exception("BuildInSituMultiPMT_Impl","WCTE001",FatalException,
                ("Invalid radii: gelRmin+airGap >= domeInnerRadius (" +
                 std::to_string((gelRmin+airGap)/mm) + " vs " +
                 std::to_string(domeInnerRadius/mm) + " mm)").c_str());
  }
}
// --- END debug guards ---


  // --------------------------------------------
  // 1) Outer vessel logical (air-filled mother)
  // --------------------------------------------
  G4double zRange[2] = {0., vessel_cylinder_height};
  G4double Rmax[2]   = {vessel_outer_radius, vessel_outer_radius};
  G4double Rmin[2]   = {0., 0.};

  G4Polycone* solidVesselCyl = new G4Polycone("WCmPMT_vessel",
                                              0.*deg, 360.*deg,
                                              2, zRange, Rmin, Rmax);

  G4Sphere* solidTopSphere = new G4Sphere("WCmPMT_tsphere",
                                          0., domeOuterRadius,
                                          0.*deg, 360.*deg,
                                          0.*deg, 90.*deg);

  G4Box* solidCut = new G4Box("cutOffTubs",
                              domeOuterRadius + 1.*cm,
                              domeOuterRadius + 1.*cm,
                              domeCut);

  G4SubtractionSolid* solidTopCap =
      new G4SubtractionSolid("WCmPMT_tsphere_cap",
                             solidTopSphere, solidCut);

  G4UnionSolid* solidMultiPMT =
      new G4UnionSolid("WCmPMT_solid", solidVesselCyl, solidTopCap,
                       0, G4ThreeVector(0,0,vessel_cylinder_height - domeCut));

  G4LogicalVolume* logicWCMultiPMT =
      new G4LogicalVolume(solidMultiPMT,
                          G4Material::GetMaterial("Air1"),
                          withAirGap ? "WCMultiPMT_AirGap" : "WCMultiPMT_NoGap");

  // --------------------------------------------
  // 2) Acrylic dome
  // --------------------------------------------
  G4Sphere* domeSphere = new G4Sphere("DomeSphere",
                                      domeInnerRadius, domeOuterRadius,
                                      0.*deg, 360.*deg,
                                      0.*deg, 90.*deg);

  G4VSolid* domeSolid =
      new G4SubtractionSolid("domeSolid", domeSphere, solidCut);

  G4LogicalVolume* domeLogic =
      new G4LogicalVolume(domeSolid,
                          G4Material::GetMaterial("G4_PLEXIGLASS"),
                          "logicDome");

  new G4PVPlacement(0,
                    G4ThreeVector(0,0,vessel_cylinder_height - domeCut),
                    domeLogic, "physInsituDome",
                    logicWCMultiPMT, false, 0, checkOverlaps);

  { // visuals
    G4VisAttributes *domeAttr = new G4VisAttributes(G4Colour(1.0,1.0,1.0,0.4));
    domeAttr->SetForceSolid(true);
    domeLogic->SetVisAttributes(domeAttr);
  }

  // --------------------------------------------
  // 3) Cylinder wall
  // --------------------------------------------
  G4Tubs* cylinderSolid =
      new G4Tubs("cylinderSolid",
                 vessel_inner_radius, vessel_outer_radius,
                 vessel_cylinder_height/2., 0.*deg, 360.*deg);

  G4LogicalVolume* logicCylinder =
      new G4LogicalVolume(cylinderSolid,
                          G4Material::GetMaterial("Plastic"),
                          "logicCylinder");

  new G4PVPlacement(0,
                    G4ThreeVector(0,0,vessel_cylinder_height/2.),
                    logicCylinder, "physInsituCylinder",
                    logicWCMultiPMT, false, 0, checkOverlaps);

  new G4LogicalSkinSurface("cylinderSkinSurface", logicCylinder, BSSkinSurface);
  { G4VisAttributes *cylA = new G4VisAttributes(G4Colour(0.2,0.2,0.2,1.0));
    cylA->SetForceSolid(true); logicCylinder->SetVisAttributes(cylA); }

  // --------------------------------------------
  // 4) Matrix (inner shell where PMTs are placed)
  // --------------------------------------------
  G4Sphere* solidMatrix =
      new G4Sphere("solidMatrix",
                   matrixRmin, matrixRmax,
                   0.*deg, 360.*deg, 0.*deg, 43.4549*deg);

  G4LogicalVolume* logicMatrix =
      new G4LogicalVolume(solidMatrix,
                          G4Material::GetMaterial("Plastic"),
                          "logicMatrix");
  new G4LogicalSkinSurface("matrixSkinSurface", logicMatrix, BSSkinSurface);

  // --------------------------------------------
  // 5) Optional 0.5 mm air shell directly under dome
  //    and gel that fills up to the correct boundary.
  // --------------------------------------------
  const G4double gelRmax = withAirGap ? gelRmax_withGap : gelRmax_noGap;

  if (withAirGap) {
    // Air shell: (domeInnerRadius - 0.5mm) -> domeInnerRadius
    G4Sphere* airShell =
        new G4Sphere("AirGapSphere",
                     gelRmax, domeInnerRadius,
                     0.*deg, 360.*deg, 0.*deg, 90.*deg);

    G4LogicalVolume* logicAirGap =
        new G4LogicalVolume(airShell,
                            G4Material::GetMaterial("Air1"),
                            "logicAirGap");

    new G4PVPlacement(0,
                      G4ThreeVector(0,0,vessel_cylinder_height - domeCut),
                      logicAirGap, "AirGap",
                      logicWCMultiPMT, false, 0, checkOverlaps);

    G4VisAttributes* airVis = new G4VisAttributes(G4Colour(0.0,1.0,1.0,0.25));
    airVis->SetForceSolid(true);
    logicAirGap->SetVisAttributes(airVis);
  }

  // Gel: fill from matrix up to gelRmax (touches dome or inner air shell)
  G4Sphere *solidGelSphere =
      new G4Sphere("solidGelSphere",
                   gelRmin, gelRmax,
                   0.*deg, 360.*deg, 0.*deg, 90.*deg);

  // cut off back
  //G4Box *solidGelCutOut =
    //  new G4Box("BoxGelCutOut",
      //          domeInnerRadius+1.*cm, domeInnerRadius+1.*cm, 241.04*mm);

  //G4VSolid *solidGelSphereCut =
    //  new G4SubtractionSolid("solidGelSphereCut",
      //                       solidGelSphere, solidGelCutOut);

G4VSolid *solidGelSphereCut =
    new G4SubtractionSolid("solidGelSphereCut",
                           solidGelSphere, solidCut);

  // subtract matrix to leave the inter-shell volume
  G4VSolid *solidGel =
      new G4SubtractionSolid("solidGel",
                             solidGelSphereCut, solidMatrix,
                             0, G4ThreeVector(0,0,gelRmin - matrixRmax + matrixUpshift));

  G4LogicalVolume *logicGel =
      new G4LogicalVolume(solidGel,
                          G4Material::GetMaterial("SilGel_WCTE"),
                          "logicGel");

  new G4PVPlacement(0,
                    G4ThreeVector(0,0,vessel_cylinder_height - domeCut),
                    logicGel, "Gel",
                    logicWCMultiPMT, false, 0, checkOverlaps);

  { G4VisAttributes *gelA = new G4VisAttributes(G4Colour(1.0,1.0,1.0,0.35));
    gelA->SetForceSolid(true); logicGel->SetVisAttributes(gelA); }

  // --------------------------------------------
  // 6) Blacksheet disk seal at bottom
  // --------------------------------------------
  G4Tubs *solidBS = new G4Tubs("solidBS", 0.*mm, vessel_inner_radius, 1.*mm, 0.*deg, 360.*deg);
  G4LogicalVolume *logicBS = new G4LogicalVolume(solidBS,
                                                 G4Material::GetMaterial("Blacksheet"),
                                                 "logicBS");

  new G4PVPlacement(0, G4ThreeVector(0,0,1.*mm),
                    logicBS, "physInsituMPMTBS",
                    logicWCMultiPMT, false, 0, checkOverlaps);
  new G4LogicalSkinSurface("insituMPMTBSSkinSurface", logicBS, BSSkinSurface);
  { G4VisAttributes* bsA = new G4VisAttributes(G4Colour(1.0,0.0,0.0,1.0));
    bsA->SetForceSolid(true); logicBS->SetVisAttributes(bsA); }

  // --------------------------------------------
  // 7) Build the internal 3-inch PMT logic and place 19 PMTs on the matrix
  // --------------------------------------------
  G4LogicalVolume* logicWCPMT = ConstructInSituPMT(PMTName, CollectionName, detectorElement);

  const G4int nIDPMTs = 19;
  G4double thetaArray[19] =  {0.*deg, 18.1345*deg, 18.1345*deg, 18.1345*deg, 18.1345*deg, 18.1345*deg, 18.1345*deg,
                              35.3149*deg, 35.3149*deg, 35.3149*deg, 35.3149*deg, 35.3149*deg, 35.3149*deg,
                              35.3149*deg, 35.3149*deg, 35.3149*deg, 35.3149*deg, 35.3149*deg, 35.3149*deg };
  G4double phiArray[19]   =  {0.*deg, 0.*deg, 60.*deg, 120.*deg, 180.*deg, -120.*deg, -60.*deg,
                              0.*deg, 30.*deg, 60.*deg, 90.*deg, 120.*deg, 150.*deg, 180.*deg,
                              -150.*deg, -120.*deg, -90.*deg, -60.*deg, -30.*deg};
/*
  for (int i = 0; i < nIDPMTs; ++i) {
    G4RotationMatrix* rot = new G4RotationMatrix;
    rot->rotateZ(-phiArray[i]);
    rot->rotateY(-thetaArray[i]);

    new G4PVPlacement(rot, G4ThreeVector(0,0,0),
                      logicWCPMT, "pmt",
                      logicMatrix, false, i, checkOverlaps);
  }
*/

  for (int tubeIndex = 0; tubeIndex < nIDPMTs; ++tubeIndex) {
    G4RotationMatrix* rot = new G4RotationMatrix;
    rot->rotateZ(-phiArray[tubeIndex]);
    rot->rotateY(-thetaArray[tubeIndex]);

    G4PVPlacement* physPMT = new G4PVPlacement(
        rot, G4ThreeVector(0,0,0),
        logicWCPMT, "pmt",
        logicMatrix, false, tubeIndex, checkOverlaps
    );

    // Get mPMT instance copy number
//    G4int mPMTcopy = physPMT->GetMotherLogical()->GetDaughter(0)->GetCopyNo();

    // After creating each of the 19 sub-PMTs:
  //  G4int parentCopyNo = logicMatrix->GetCopyNo(); // mPMT copy number
/*
std::ofstream csvOut("/eos/home-j/jrimmer/sim_work_dir/WCSim/build/wcsim_pmt_mpmts.csv", std::ios::app);
if (csvOut.is_open()) {
    csvOut << parentCopyNo << "," << i << "\n";
    csvOut.close();
} else {
    G4cerr << "[CSV ERROR] Could not open wcsim_pmt_mpmts.csv for writing" << G4endl;
}
*/
}


  // Place the matrix last
  new G4PVPlacement(0,
                    G4ThreeVector(0,0, vessel_cylinder_height - domeCut + gelRmin - matrixRmax + matrixUpshift),
                    logicMatrix, "physMatrix",
                    logicWCMultiPMT, false, 0, checkOverlaps);

  // visuals on mother
  { G4VisAttributes* VisAttGrey = new G4VisAttributes(G4Colour(0.2,0.2,0.2,1.0));
    VisAttGrey->SetForceSolid(true); logicWCMultiPMT->SetVisAttributes(VisAttGrey); }

  // Cache and return
  PMTLogicalVolumes[key] = logicWCMultiPMT;
  return logicWCMultiPMT;
}

// -----------------------------
// Public variants (call these)
// -----------------------------
G4LogicalVolume*
WCSimDetectorConstruction::ConstructInSituMultiPMT_NoGap(G4String PMTName,
                                                         G4String CollectionName,
                                                         G4String detectorElement)
{
  return BuildInSituMultiPMT_Impl(PMTName, CollectionName, detectorElement, false);
}

G4LogicalVolume*
WCSimDetectorConstruction::ConstructInSituMultiPMT_WithAirGap(G4String PMTName,
                                                              G4String CollectionName,
                                                              G4String detectorElement)
{
  return BuildInSituMultiPMT_Impl(PMTName, CollectionName, detectorElement, true);
}

// This is the original PMT constructor code
// 3-inch PMT construction
G4LogicalVolume* WCSimDetectorConstruction::ConstructInSituPMT(G4String PMTName, G4String CollectionName, G4String detectorElement)
{
  G4String keyname = PMTName+"_InSitu-PMT";
  PMTKey_t key(keyname,CollectionName);
  // Return pre-created PMT Logical Volume if it already exists.
  PMTMap_t::iterator it = PMTLogicalVolumes.find(key);
  if (it != PMTLogicalVolumes.end()) {
    return it->second;
  }

  /////////////////////////////////////////
  // From actual measurements
  G4double pmtGlassThickness = 2.0*mm;
  G4double pmtGlassRadius = 53.0*mm;
  G4double pmtCylRadius = 40.15*mm;
  G4double pmtTubeRadius = 26.1*mm;
  G4double pmtCylHeight = 18.0*mm;
  G4double pmtConeHeight = 20.0*mm;
  G4double pmtGlassCut = 34.597*mm;

  G4double reflectorHeight = 6.5*mm*2;
  G4double reflectorThickness = 0.8*mm;
  G4double reflectorR1 = 36.96*mm;
  G4double reflectorR2 = 45.*mm;

  G4cout << "========================================================" << G4endl;
  G4cout << "In-situ PMT construction: " << G4endl;
  G4cout << "Glass radius for PMT " << CollectionName << " : " << pmtGlassRadius/mm << " mm" << G4endl;
  G4cout << "Expose height : " << (pmtGlassRadius-pmtGlassCut)/mm << " mm" << G4endl;
  G4cout << "Cylinder radius : " << pmtCylRadius/mm << " mm" << G4endl;
  G4cout << "Reflector height, R1, R2 : " << reflectorHeight/mm << " " << reflectorR1/mm << " " << reflectorR2/mm << " mm" << G4endl;
  G4cout << "========================================================" << G4endl;

  ///////////////////////////////Defining PMT Volume/////////////////////////////////
  G4double matrxiRadius = 325.603*mm;
  G4double absorberThickness = 1.*mm;
  G4double pmtVolumeHeight = absorberThickness + pmtConeHeight + pmtCylHeight + 23.6*mm;
  G4double zoffset = matrxiRadius-pmtConeHeight/2.-pmtCylHeight- 23.6*mm;
  G4double openingAngle = 8.14*deg;

  G4Sphere *solidWCPMT = new G4Sphere("WCPMT",
                                      matrxiRadius-pmtVolumeHeight,
                                      matrxiRadius,
                                      0.0*deg, 360.0*deg,
                                      0.0, openingAngle);

  G4LogicalVolume *logicWCPMT = new G4LogicalVolume(solidWCPMT,
                                                    G4Material::GetMaterial("SilGel_WCTE"),
                                                    "WCPMT");

  //////////////////////////
  // Visualize logicWCPMT //
  //////////////////////////

  if (Vis_Choice == "RayTracer"){
    // Makes the volume containing the PMT visible, solid, and forces the auxiliary edges to be viewed.
    G4VisAttributes* WCPMTVisAtt = new G4VisAttributes(G4Colour(0.0,0.0,1.0));
    WCPMTVisAtt->SetForceSolid(true); // force the object to be visualized with a surface
    WCPMTVisAtt->SetForceAuxEdgeVisible(true); // force auxiliary edges to be shown

    logicWCPMT->SetVisAttributes(WCPMTVisAtt);
  } else{
    G4VisAttributes* WCPMTVisAttGrey = new G4VisAttributes(G4Colour(0.2,0.2,0.2));
    WCPMTVisAttGrey->SetForceSolid(true);
    logicWCPMT->SetVisAttributes(G4VisAttributes::Invisible);
    logicWCPMT->SetVisAttributes(WCPMTVisAttGrey);                   
  }

  // inner glass tube
  G4double glassTubeZ[3] = {-pmtConeHeight/2.,
                            pmtConeHeight/2.,
                            pmtConeHeight/2.+pmtCylHeight};
  G4double glassTubeRmin[3] = { pmtTubeRadius-pmtGlassThickness,
                                pmtCylRadius-pmtGlassThickness,
                                pmtCylRadius-pmtGlassThickness};
  G4double glassTubeRmax[3] = { pmtTubeRadius,
                                pmtCylRadius,
                                pmtCylRadius};
  G4Polycone* solidGlassTube = new G4Polycone("solidGlassTube",
                                              0,
                                              360.*deg,
                                              3,
                                              glassTubeZ,
                                              glassTubeRmin,
                                              glassTubeRmax);
  
  G4LogicalVolume *logicGlassTube = new G4LogicalVolume(solidGlassTube,
                                                        G4Material::GetMaterial("GlassWCTE"),
                                                        "logicInnerGlassTube");
                                        
  new G4PVPlacement(0,
                    G4ThreeVector(0.,0.,zoffset),
                    logicGlassTube,
                    "InnerGlassTube",
                    logicWCPMT,
                    false,
                    0,
                    checkOverlaps);
  
  // air inside glass tube
  G4double airTubeZ[4] =  {-pmtConeHeight/2.,
                            pmtConeHeight/2.,
                            pmtConeHeight/2.,
                            pmtConeHeight/2.+pmtCylHeight};
  G4double airTubeRmin[4] = { 0, 0, 0, 0};
  G4double airTubeRmax[4] = { pmtTubeRadius-pmtGlassThickness-0.1*mm,
                              pmtCylRadius-pmtGlassThickness-0.1*mm,
                              pmtCylRadius-pmtGlassThickness,
                              pmtCylRadius-pmtGlassThickness,
                            };
  G4Polycone* solidAirTube  = new G4Polycone("solidAirTube",
                                              0,
                                              360.*deg,
                                              4,
                                              airTubeZ,
                                              airTubeRmin,
                                              airTubeRmax);

  G4LogicalVolume *innerLogicTube = new G4LogicalVolume(solidAirTube, 
                                                        G4Material::GetMaterial("Air1"),
                                                        "logicInnerAir");
  
  new G4PVPlacement(0,
                    G4ThreeVector(0.,0.,zoffset),
                    innerLogicTube,
                    "InnerAir",
                    logicWCPMT,
                    false,
                    0,
                    checkOverlaps);

  //Reflector material inside the PMT
  G4Cons *solidConeTube = new G4Cons("ConeTube",
                                      pmtTubeRadius-pmtGlassThickness-0.1*mm,
                                      pmtTubeRadius-pmtGlassThickness,
                                      pmtCylRadius-pmtGlassThickness-0.1*mm,
                                      pmtCylRadius-pmtGlassThickness,
                                      pmtConeHeight/2.,
                                      0.*deg, 360.*deg);

  G4LogicalVolume *logicLayer = new G4LogicalVolume(solidConeTube, 
						    G4Material::GetMaterial("Aluminum"), 
						    "logicInnerReflector");

  new G4PVPlacement(0,
                    G4ThreeVector(0.,0.,zoffset),
                    logicLayer,
                    "InnerReflector",
                    logicWCPMT,
                    false,
                    0,
                    checkOverlaps);

  new G4LogicalSkinSurface("ReflectorLogSkinSurface", logicLayer, ReflectorSkinSurfaceWCTE);

  G4VisAttributes *layerAttributes = new G4VisAttributes();
  layerAttributes->SetColor(0.85, 0.85, 0.85, 1.0);
  layerAttributes->SetVisibility(true);
  layerAttributes->SetForceSolid(true);
  logicLayer->SetVisAttributes(layerAttributes);
                  
  // artificial absorber at the bottom of glass tube to kill all photons
  G4Cons* solidAbsorber = new G4Cons("solidAbsorber", 
                                      0.0, 
                                      pmtTubeRadius, 
                                      0.0, 
                                      pmtTubeRadius, 
                                      absorberThickness/2., 
                                      0.0, 360.0*deg);
  
  G4LogicalVolume *logicabsorber = new G4LogicalVolume( solidAbsorber, 
                                                        G4Material::GetMaterial("customAbsorber"),
                                                        "logicabsorber");
  
  new G4PVPlacement(0, 
                    G4ThreeVector(0.,0.,-absorberThickness/2.-pmtConeHeight/2.+zoffset),
                    logicabsorber, 
                    "absorber", 
                    logicWCPMT, 
                    false, 
                    0, 
                    checkOverlaps);

  new G4LogicalSkinSurface("AbsorberSurfaceProperties", logicabsorber, AbsorberSkinSurface);
  
  G4VisAttributes *visAttributes = new G4VisAttributes();
  visAttributes->SetColor(G4Color::Gray());
  visAttributes->SetVisibility(true);
  visAttributes->SetForceSolid(true);
  logicabsorber->SetVisAttributes(visAttributes);

  // Photocathode glass
  G4Box *solidCutOff = new G4Box( "cutOffTubs",
                                  pmtGlassRadius+1.*cm,                                                                     
                                  pmtGlassRadius+1.*cm,                                                                    
                                  pmtGlassCut);

  G4Sphere *pmtGlassSolid = new G4Sphere("pmtglass", 
                                          pmtGlassRadius-pmtGlassThickness, 
                                          pmtGlassRadius, 
                                          0.0*deg, 360.0*deg, 
                                          0.0, 90*deg);  
  
  G4SubtractionSolid *solidGlassFaceWCPMT = new G4SubtractionSolid( CollectionName,                                       
                                                                    pmtGlassSolid,                                   
                                                                    solidCutOff);

  G4LogicalVolume *logicGlassFaceWCPMT = new G4LogicalVolume( solidGlassFaceWCPMT, 
                                                              G4Material::GetMaterial("GlassWCTE"),
                                                              CollectionName);
  
  G4VPhysicalVolume *physiGlassFaceWCPMT = new G4PVPlacement( 0, 
                                                              G4ThreeVector(0.,0.,-pmtGlassCut+pmtConeHeight/2.+pmtCylHeight+zoffset), 
                                                              logicGlassFaceWCPMT, 
                                                              CollectionName, 
                                                              logicWCPMT, 
                                                              false, 
                                                              0, 
                                                              checkOverlaps);

  if (Vis_Choice == "RayTracer"){
    // Blue wireframe visual style
    // Used in the RayTracer visualizer
    G4VisAttributes* WCPMTVisAtt = new G4VisAttributes(G4Colour(0.0,0.0,1.0));
    WCPMTVisAtt->SetForceSolid(true); // force the object to be visualized with a surface
    WCPMTVisAtt->SetForceAuxEdgeVisible(true); // force auxiliary edges to be shown
    //logicGlassFaceWCPMT->SetVisAttributes(G4VisAttributes::Invisible);

    logicGlassFaceWCPMT->SetVisAttributes(WCPMTVisAtt);
  } else {

    // Gray wireframe visual style
    // used in OGLSX visualizer
    //G4VisAttributes* WCPMTVisAtt = new G4VisAttributes(G4Colour(0.2,0.2,0.2));  //better for seeing hits
    //WCPMTVisAtt->SetForceWireframe(true);
    G4VisAttributes* WCPMTVisAtt = new G4VisAttributes(G4Colour(0.0,1.0,0.0)); //better for seeing geometry
    WCPMTVisAtt->SetForceSolid(true);
    //logicGlassFaceWCPMT->SetVisAttributes(G4VisAttributes::Invisible);
    logicGlassFaceWCPMT->SetVisAttributes(WCPMTVisAtt);

  }

  // air below photocathode
  G4Sphere *pmtInnerGlassSolid = 
  new G4Sphere("pmtInnerglassSolid",
					      0.*mm,
					      pmtGlassRadius-pmtGlassThickness,
					      0.0*deg, 360.0*deg,
					      0.0, 90*deg);
  G4Box *boxCutOff = 
  new G4Box("cutOffTubs",                                                                            
			       pmtGlassRadius+1.*cm,                                                    
			       pmtGlassRadius+1.*cm,                                             
			       pmtGlassCut);


  G4SubtractionSolid *solidInteriorWCPMT = 
  new G4SubtractionSolid("InteriorWCPMT",     
                          pmtInnerGlassSolid,
                          boxCutOff); 

  G4LogicalVolume *logicInteriorWCPMT = 
  new G4LogicalVolume(solidInteriorWCPMT,
                      G4Material::GetMaterial("Air1"),
                      "InteriorWCPMT");
  
  G4VPhysicalVolume *physiInteriorWCPMT = 
  new G4PVPlacement(0,
                    G4ThreeVector(0.,0.,-pmtGlassCut+pmtConeHeight/2.+pmtCylHeight+zoffset),
                    logicInteriorWCPMT,
                    "InteriorWCPMT",
                    logicWCPMT,
                    false,
                    0,
                    checkOverlaps);

  if (Vis_Choice == "RayTracer"){
    // Adding color and forcing the inner portion of the PMT's to be viewed
    G4VisAttributes* WCPMTVisAtt = new G4VisAttributes(G4Colour(0.0,0.0,1.0));
    WCPMTVisAtt->SetForceSolid(true); // force the object to be visualized with a surface
    WCPMTVisAtt->SetForceAuxEdgeVisible(true); // force auxiliary edges to be shown

    logicInteriorWCPMT->SetVisAttributes(WCPMTVisAtt);
  } else {
    // Making the inner portion of the detector invisible for OGLSX visualization
    logicInteriorWCPMT->SetVisAttributes(G4VisAttributes::Invisible);
  }

  //Add Logical Border Surface
  new G4LogicalBorderSurface("GlassCathodeSurface",
                              physiGlassFaceWCPMT,
                              physiInteriorWCPMT,
                              OpGlassCathodeSurface);
  // now it is possible for photons to travel from air to cathode
  new G4LogicalBorderSurface("AirCathodeSurface",
                              physiInteriorWCPMT,
                              physiGlassFaceWCPMT,
                              OpGlassCathodeSurface);

  // Top Reflector
  G4Cons *reflectorSolid = new G4Cons("solidReflector",
                                      reflectorR1,
                                      reflectorR1+reflectorThickness,
                                      reflectorR2,
                                      reflectorR2+reflectorThickness,
                                      reflectorHeight/2.,
                                      0.*deg, 360.*deg);

  G4LogicalVolume *reflectorLogic = new G4LogicalVolume(reflectorSolid,
                                                        G4Material::GetMaterial("Aluminum"), 
                                                        "logicReflector");
  
  new G4PVPlacement(0,
                    G4ThreeVector(0.,0.,39.9*mm+zoffset),
                    reflectorLogic,                                                         
                    "Reflector",                                                       
                    logicWCPMT,                                          
                    false,                                               
                    0,                                               
                    checkOverlaps); 

  new G4LogicalSkinSurface("ReflectorCupSkinSurface",reflectorLogic,ReflectorSkinSurfaceWCTE);

  G4VisAttributes *refAttributes = new G4VisAttributes();
  refAttributes->SetColor(0.85, 0.85, 0.85, 1.0);
  refAttributes->SetVisibility(true);
  refAttributes->SetForceSolid(true);
  reflectorLogic->SetVisAttributes(refAttributes);  

  /////////////////////////
  // Sensitive detector ///
  /////////////////////////

  // Instantiate a new sensitive detector
  // and register this sensitive detector volume with the SD Manager.
  G4SDManager* SDman = G4SDManager::GetSDMpointer();
  G4String SDName = "/WCSim/";
  SDName += CollectionName;

  // If there is no such sensitive detector with that SDName yet,
  // make a new one
  if( ! SDman->FindSensitiveDetector(SDName, false) ) {

    aWCPMT = new WCSimWCSD(CollectionName,SDName,this,detectorElement);
    SDman->AddNewDetector( aWCPMT );
  }

  logicGlassFaceWCPMT->SetSensitiveDetector( aWCPMT );


  PMTLogicalVolumes[key] = logicWCPMT;

  return logicWCPMT;
}


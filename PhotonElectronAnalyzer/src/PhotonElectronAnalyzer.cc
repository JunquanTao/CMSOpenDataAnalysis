// -*- C++ -*-
//
// Package:    PhotonElectronAnalyzer
// Class:      PhotonElectronAnalyzer
// 
/**\class PhotonElectronAnalyzer PhotonElectronAnalyzer.cc CMSOpenDataAnalysis/PhotonElectronAnalyzer/src/PhotonElectronAnalyzer.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Junquan Tao
//         Created:  Mon Jul 16 09:15:26 CEST 2018
// $Id$
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/Candidate/interface/Particle.h"
#include "DataFormats/Common/interface/RefVector.h"

#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"
#include "CLHEP/Vector/LorentzVector.h"
#include <HepMC/WeightContainer.h>
#include <HepMC/GenEvent.h>
#include <HepMC/GenParticle.h>

#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
//HLT
#include "DataFormats/Common/interface/TriggerResults.h"
#include "FWCore/Common/interface/TriggerNames.h"

#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"

//Vertex
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"

//Photon
//#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/EgammaCandidates/interface/PhotonFwd.h"

#include "DataFormats/EgammaReco/interface/SuperCluster.h"
#include "DataFormats/EgammaReco/interface/SuperClusterFwd.h"

//Electron
//#include "DataFormats/EgammaCandidates/interface/Electron.h"
#include "DataFormats/EgammaCandidates/interface/ElectronFwd.h"
//#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
//#include "DataFormats/EgammaCandidates/interface/GsfElectronFwd.h"

//
// class decleration
//

#include <iostream>
#include <vector>
#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>

using namespace std;
using namespace reco;
using namespace edm;

#define TAODEBUG 0

class PhotonElectronAnalyzer : public edm::EDAnalyzer {
public:
  explicit PhotonElectronAnalyzer(const edm::ParameterSet&);
  ~PhotonElectronAnalyzer();


private:
  virtual void beginJob(const edm::EventSetup&) ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  // ----------member data ---------------------------
  edm::InputTag puInforProducer_;
  edm::InputTag rhoCollection_;
  edm::InputTag vertexProducer_;
  edm::InputTag photonProducer_;
  edm::InputTag electronProducer_;

  int nEvt;// used to count the number of events

  // to be used for root output tree
  std::string outputFile; // output file
  TFile *outputfile;
  //====================
  TTree *eventTree;
  TTree *photonTree;
  TTree *diphotonTree;
  TTree *electronTree;
  TTree *dielectronTree;


  // Variables to fill
  Int_t run, lumis, event, IsData;
  int nvtx;
  double rho, gen_weight;
  int pu_n;

  //single photon
  double pho_eta, pho_phi, pho_energy;
  double pho_pt, pho_SCeta, pho_Escraw;
  double pho_full5x5_r9, pho_r9;
  double pho_sieie, pho_covieip, pho_s4ratio;
  double pho_etawidth, pho_phiwidth, pho_ESEffSigmaRR;
  double pho_pfPhoIso03, pho_pfNeutIso03, pho_pfChgIso03, pho_pfChgIso03worst;


  //diphoton
  double dipho_pt,  dipho_eta,  dipho_phi, dipho_energy, dipho_mass;
  double dipho_CosThetaStar, dipho_DeltaPhi, dipho_DeltaR;

  double dipho_pho1_pt, dipho_pho1_eta, dipho_pho1_phi, dipho_pho1_energy;
  double dipho_pho1_r9, dipho_pho1_hoe, dipho_pho1_sieie;

  double dipho_pho2_pt, dipho_pho2_eta, dipho_pho2_phi, dipho_pho2_energy;
  double dipho_pho2_r9, dipho_pho2_hoe, dipho_pho2_sieie;


 
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
PhotonElectronAnalyzer::PhotonElectronAnalyzer(const edm::ParameterSet& iConfig)
{
   //now do what ever initialization is needed
  nEvt=0;
  outputFile= iConfig.getUntrackedParameter<std::string>("outputfileName");
  outputfile= TFile::Open(outputFile.c_str(),"RECREATE"); // open output file

  puInforProducer_ =iConfig.getParameter<edm::InputTag>("puInforProducer"); 
  rhoCollection_ =iConfig.getParameter<edm::InputTag>("rhoCorrectionProducer");
  vertexProducer_ = iConfig.getParameter<edm::InputTag>("vertexProducer");
  photonProducer_ = iConfig.getParameter<edm::InputTag>("photonProducer");
  electronProducer_ = iConfig.getParameter<edm::InputTag>("electronProducer");

  outputfile->cd();
  eventTree = new TTree("eventTree", "per-event tree");

  eventTree->Branch( "IsData", &IsData, "IsData/I" );
  eventTree->Branch( "run", &run, "run/I" );
  eventTree->Branch( "event", &event, "event/I" );
  eventTree->Branch( "lumis", &lumis, "lumis/I" );
  eventTree->Branch( "pu_n", &pu_n, "pu_n/I" );
  eventTree->Branch( "nvtx", &nvtx, "nvtx/I" );
  eventTree->Branch( "gen_weight", &gen_weight, "gen_weight/D" );
  eventTree->Branch( "rho", &rho, "rho/F" );

  //tree->Branch("", &,"/D");
  //tree->Branch("", &,"/I");

}


PhotonElectronAnalyzer::~PhotonElectronAnalyzer()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to for each event  ------------
void
PhotonElectronAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;
  using namespace reco;
  using namespace std;

  nEvt++;

  if(TAODEBUG==1) cout<<"JTao: event info"<<endl;
  run = iEvent.id().run();
  event = iEvent.id().event();
  lumis = iEvent.eventAuxiliary().luminosityBlock();
  IsData = iEvent.isRealData();

  gen_weight = 1.0;
  if( ! IsData ) { //MC
    edm::Handle<GenEventInfoProduct> genEvent;
    try{
      iEvent.getByLabel("generator", genEvent);
      if( !genEvent.isValid() ){
	cout <<  "   ===> No Valid GenEventInfoProduct, skip PDF Infos" << endl;
      }else{
	//const auto &weights = genEvent->weights();
	//gen_weight = weights[0];
	if(genEvent->weights().size()>0 ) gen_weight=genEvent->weight();
      }
    }catch ( cms::Exception& ex ) { 
    edm::LogError("TaoAnalyzer") <<"Error! can't get collection with label : generator"; 
  }
  }
  //PU infor
  pu_n = 0;
  edm::Handle<std::vector< PileupSummaryInfo> > PupInfo;
  try {
    iEvent.getByLabel("addPileupInfo", PupInfo);
  } catch ( cms::Exception& ex ) { 
    edm::LogError("TaoAnalyzer") <<"Error! can't get collection with label : addPileupInfo"; 
  }
  std::vector<PileupSummaryInfo>::const_iterator PVI;
  for(PVI = PupInfo->begin(); PVI != PupInfo->end(); ++PVI) {
    int PU_Bunchcrossing = PVI->getBunchCrossing();
    if(PU_Bunchcrossing == 0) {
      pu_n = PVI->getPU_NumInteractions();
      break;
    }
  }
  //Rho
  edm::Handle<double> rhoH;
  iEvent.getByLabel(rhoCollection_,rhoH);
  rho = (*rhoH);

  //RecVertices
  nvtx = 0;   
  Handle<reco::VertexCollection> recVtxs;
  //edm::Handle<std::vector<reco::Vertex> > recVtxs;
  try {
    iEvent.getByLabel(vertexProducer_, recVtxs);
  } catch ( cms::Exception& ex ) {
    edm::LogError("TaoAnalyzer") <<"Error! can't get collection with label : Vertices"; }
  reco::VertexCollection::const_iterator myvtx;
  //std::vector<reco::Vertex>::const_iterator myvtx;
  //nvtx = recVtxs->size();
  for(myvtx=recVtxs->begin(); myvtx!=recVtxs->end(); myvtx++){
    nvtx++; 
  }
  // //////////////////////////////////Fill event tree
  eventTree->Fill();

  //std::vector<reco::Photon>
  //event.getByLabel('photons', self.photonHandle)






  ////////////////////////////////////////////////////////


}


// ------------ method called once each job just before starting event loop  ------------
void 
PhotonElectronAnalyzer::beginJob(const edm::EventSetup&)
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
PhotonElectronAnalyzer::endJob() {
  std::cout << "++++++++++++++++++++++++++++++++++++++" << std::endl;
  std::cout << "analyzed " << nEvt << " events " << std::endl;
  std::cout << "writing information into file: " << outputfile->GetName() << std::endl;
  outputfile->Write();
  outputfile->Close();
}

//define this as a plug-in
DEFINE_FWK_MODULE(PhotonElectronAnalyzer);

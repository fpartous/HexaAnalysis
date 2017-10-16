// -*- C++ -*-
//
// Package:    TreeProducer_AOD
// Class:      TreeProducer_AOD
///**\class TreeProducer_AOD TreeProducer_AOD.cc HexaAnalysis/TreeProducer/src/TreeProducer_AOD.cc
/*
 Description: EDAnalyzer produce flat trees from AOD for HexaAnalysis

*/

// C++ lib
#include <memory>
#include <vector>

// ROOT
#include "TTree.h"
#include "TLorentzVector.h"
#include "TPRegexp.h"

// CMSSW standard lib
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

// CMSSW specific lib
//#include "DataFormats/Common/interface/ValueMap.h"
#include "RecoVertex/PrimaryVertexProducer/interface/PrimaryVertexSorter.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
//#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/L1Trigger/interface/L1EtMissParticle.h"
//#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"

#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"
#include "HLTrigger/HLTcore/interface/HLTPrescaleProvider.h"

#include "DataFormats/Math/interface/deltaR.h"

// others
using namespace std;
int verbose=1;
const UInt_t nJ=8; // #jets
const UInt_t nGJ=4; // #genjets
const UInt_t nV=3; // #vertices
const UInt_t nT=10; // #tracks
const UInt_t nP=4; // #photons

//
// class declaration
//

class TreeProducer_AOD : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> {
 public:
  explicit TreeProducer_AOD(const edm::ParameterSet&);
  ~TreeProducer_AOD();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  static bool ptSorter(const reco::Track & i, const reco::Track & j);

 private:
  virtual void beginJob() ;
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void endJob() ;

  virtual void beginRun(edm::Run const&, edm::EventSetup const&);
  virtual void endRun(edm::Run const&, edm::EventSetup const&);
  virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);
  virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);

  void Init();

  // ----------member data ---------------------------
  edm::InputTag _trigResultsTag;
  edm::InputTag _genjetCollectionTag;
  edm::InputTag _vertexCollectionTag;
  edm::InputTag _METCollectionTag;
  edm::InputTag _trackCollectionTag;

  edm::EDGetTokenT<edm::TriggerResults> _trigResultsToken;
  edm::EDGetTokenT<vector<reco::GenJet> > _genjetCollectionToken;
  edm::EDGetTokenT<vector<reco::Vertex> > _vertexCollectionToken;
  edm::EDGetTokenT<vector<reco::Track> > _trackCollectionToken;
  bool _isData;

  HLTConfigProvider hltConfig_;
  HLTPrescaleProvider hltPrescale_;

  std::vector<std::string> triggerNames_;
  std::vector<unsigned int> triggerIndex_;

//   GlobalPoint vertexPosition;

  // Tree and its branches
  TTree* _tree;

  // Global quantities
  int _nEvent, _nRun, _nLumi, _nJet, _nJet_stored, _nGenJet, _nGenJet_stored, _nTrack, _nTrack_stored;

  // Vertices
  int _vtx_N, _vtx_N_stored;
  std::vector<int> _vtx_ndof, _vtx_nTracks;
  std::vector<double> _vtx_x, _vtx_y, _vtx_z;
  std::vector<double> _vtx_normalizedChi2, _vtx_d0;

	//Tracks
	std::vector<int> _track_fromPV, _track_Nhits, _track_NpixHits, _track_purity, _track_ndof;
	std::vector<double> _track_eta, _track_pt, _track_phi, _track_ptError, _track_dxy, _track_d0, _track_dzError, _track_dz, _track_normalizedChi2;

  // GenJets
  std::vector<double> _genjet_vx, _genjet_vy, _genjet_vz;//vertex position
  std::vector<double> _genjet_area;
  std::vector<double> _genjet_eta, _genjet_phi, _genjet_pt, _genjet_e, _genjet_m;
  std::vector<double> _genjet_efrac_ch;// charged energy fractions

  //Trigger
  std::vector<std::string> triggerPathsVector;
  std::map<std::string, int> triggerPathsMap;
  int _dijet_170_0p1, _dijet_220_0p3, _dijet_330_0p5, _dijet_430, _dijet_170, _singlejet_170_0p1, _photon_120, _photon_175, _singlejet_450, _singlejet_500;
  int _isomu24, _isomu27;
  //prescales
  double  _pswgt_dijet_170,  _pswgt_singlejet_170_0p1;
  double _pswgt_photon_120, _pswgt_photon_175;

  //MET filters
  std::vector<std::string>   filterPathsVector;
  std::map<std::string, int> filterPathsMap;
  int _HBHENoiseFlag, _HBHENoiseIsoFlag, _ECALFlag, _vertexFlag, _eeFlag, _beamhaloFlag;


  //PFRho
double _pfrho;




};

namespace reco {
	template<typename T>
	class RecoPtSorter{
	public:
		bool operator ()(const T & i, const T & j) const {
			return (i->pt() > j->pt());
		}

	};
}

//
// constants, enums and typedefs
//
namespace reco {
  typedef std::vector<Track> TrackCollection;
  typedef edm::Ref<TrackCollection> TrackRef;
}
//
// static data member definitions
//
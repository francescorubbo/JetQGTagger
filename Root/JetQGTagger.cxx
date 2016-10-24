#include "JetQGTagger/JetQGTagger.h"

#include <TRandom3.h>
#include "InDetTrackSelectionTool/InDetTrackSelectionTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthFilterTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h"
#include "InDetTrackSystematicsTools/JetTrackFilterTool.h"


namespace CP {

  JetQGTagger::JetQGTagger( const std::string& name): asg::AsgTool( name )
  {
    declareProperty( "Tagger", m_taggername = "ntrack");
    declareProperty( "MinPt", m_minpt = 50e3);
    declareProperty( "MinEta", m_maxeta = 2.1);
    declareProperty( "WeightDecorationName", m_weight_decoration_name = "qgTaggerWeight");
    declareProperty( "TaggerDecorationName", m_tagger_decoration_name = "qgTagger");
    applySystematicVariation(SystematicSet()).ignore();
  }

  StatusCode JetQGTagger::initialize(){
    
    m_weightdec = new SG::AuxElement::Decorator< float>(m_weight_decoration_name);
    m_taggerdec  = new SG::AuxElement::Decorator< float>(m_tagger_decoration_name);
    
    m_trkSelectionTool = new InDet::InDetTrackSelectionTool("trackselectiontool");
    m_trkSelectionTool->setProperty( "CutLevel", "Loose" );
    m_trkSelectionTool->setProperty( "maxAbsEta", 2.5 );
    m_trkSelectionTool->setProperty( "minNSiHits", 7 );
    m_trkSelectionTool->setProperty( "maxNPixelSharedHits", 1 );
    m_trkSelectionTool->setProperty( "maxOneSharedModule", true );
    m_trkSelectionTool->setProperty( "maxNSiHoles", 2 );
    m_trkSelectionTool->setProperty( "maxNPixelHoles", 1 );
    m_trkSelectionTool->initialize();


    m_trkTruthOriginTool = new InDet::InDetTrackTruthOriginTool("InDet::InDetTrackTruthOriginTool");
    m_trkTruthOriginTool->initialize();
    
    m_trkTruthFilterTool = new InDet::InDetTrackTruthFilterTool("trackfiltertool");
    m_trkTruthFilterTool->setProperty( "Seed", 1234 );
    m_trkTruthFilterTool->initialize();
    CP::SystematicSet systSetTrk = {
      InDet::TrackSystematicMap[InDet::TRK_EFF_LOOSE_GLOBAL],
      InDet::TrackSystematicMap[InDet::TRK_EFF_LOOSE_IBL],
      InDet::TrackSystematicMap[InDet::TRK_EFF_LOOSE_PP0],
      InDet::TrackSystematicMap[InDet::TRK_EFF_LOOSE_PHYSMODEL]
    };
    m_trkTruthFilterTool->applySystematicVariation(systSetTrk);
    
    m_trkFakeTool = new InDet::InDetTrackTruthFilterTool("trackfaketool");
    m_trkFakeTool->setProperty( "Seed", 1234 );
    m_trkFakeTool->initialize();
    CP::SystematicSet systSetTrkFake = {
      InDet::TrackSystematicMap[InDet::TRK_FAKE_RATE]
    };
    m_trkFakeTool->applySystematicVariation(systSetTrkFake);
    
    m_jetTrackFilterTool = new InDet::JetTrackFilterTool("jettrackfiltertool");
    m_jetTrackFilterTool->setProperty( "Seed", 1234 );
    m_jetTrackFilterTool->initialize();
    CP::SystematicSet systSetJet = {
      InDet::TrackSystematicMap[InDet::TRK_EFF_LOOSE_TIDE]
    };
    m_jetTrackFilterTool->applySystematicVariation(systSetJet);
    
    return StatusCode::SUCCESS;
  }

  StatusCode JetQGTagger::finalize(){

    delete m_trkSelectionTool;
    delete m_trkTruthFilterTool;
    delete m_trkFakeTool;
    delete m_jetTrackFilterTool;
    
    return StatusCode::SUCCESS;
  }

  StatusCode JetQGTagger::setTagger(const xAOD::Jet * jet, const xAOD::Vertex * pv){


    float tagger = -1;
    float weight = -1;

    if(jet->pt()<m_minpt){
      ATH_MSG_WARNING("Jet pT is below allowed range");
    }
    else if(fabs(jet->eta())>m_maxeta){
      ATH_MSG_WARNING("Jet eta is beyond allowed range");
    }
    else{
      tagger = getNTrack(jet, pv);
      weight = 1.0;
    }

    (*m_taggerdec)(*jet) = tagger;
    (*m_weightdec)(*jet) = weight;

    return StatusCode::SUCCESS;
  }

  int JetQGTagger::getNTrack(const xAOD::Jet * jet, const xAOD::Vertex * pv){
    int ntracks = 0;    
    
    std::vector<const xAOD::IParticle*> jettracks;
    jet->getAssociatedObjects<xAOD::IParticle>(xAOD::JetAttribute::GhostTrack,jettracks);
    for (size_t i = 0; i < jettracks.size(); i++) {
      
      const xAOD::TrackParticle* trk = static_cast<const xAOD::TrackParticle*>(jettracks[i]);
      
      bool acceptSyst = 
	(
	 m_appliedSystEnum==NONE || 
	 m_appliedSystEnum==QG_NCHARGEDEXP || m_appliedSystEnum==QG_NCHARGEDME || m_appliedSystEnum==QG_NCHARGEDPDF ||
	 (m_appliedSystEnum==QG_TRACKEFFICIENCY && m_trkTruthFilterTool->accept(trk) && m_jetTrackFilterTool->accept(trk,jet)) ||
	 (m_appliedSystEnum==QG_TRACKFAKES && m_trkFakeTool->accept(trk))
	 );
      if (!acceptSyst) continue;

      bool accept = (trk->pt()>500 && m_trkSelectionTool->accept(*trk) && 
		     (trk->vertex()==pv ||
		      (!trk->vertex() && 
		       fabs((trk->z0()+trk->vz()-pv->z())*sin(trk->theta()))<3.) ) );
      if (!accept) continue;
      
      ntracks++;
    }
    
    return ntracks;
  }
  
  SystematicCode JetQGTagger::sysApplySystematicVariation(const SystematicSet& systSet){
    m_appliedSystEnum = NONE;
    if (systSet.size()==0) {
      ATH_MSG_DEBUG("No affecting systematics received.");
      return SystematicCode::Ok;
    } else if (systSet.size()>1) {
      ATH_MSG_WARNING("Tool does not support multiple systematics, returning unsupported" );
      return CP::SystematicCode::Unsupported;
    }
    SystematicVariation systVar = *systSet.begin();
    if (systVar == SystematicVariation("")) m_appliedSystEnum = NONE;
    else if (systVar == QGntrackSyst::trackefficiency) m_appliedSystEnum = QG_TRACKEFFICIENCY;
    else if (systVar == QGntrackSyst::trackfakes) m_appliedSystEnum = QG_TRACKFAKES;
    else if (systVar == QGntrackSyst::nchargedexp) m_appliedSystEnum = QG_NCHARGEDEXP;
    else if (systVar == QGntrackSyst::nchargedme) m_appliedSystEnum = QG_NCHARGEDME;
    else if (systVar == QGntrackSyst::nchargedpdf) m_appliedSystEnum = QG_NCHARGEDPDF;
    else {
      ATH_MSG_WARNING("unsupported systematic applied");
      return SystematicCode::Unsupported;
    }
    
    ATH_MSG_DEBUG("applied systematic is " << m_appliedSystEnum);
    return SystematicCode::Ok;
  }
  
} /* namespace CP */

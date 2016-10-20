#include "JetQGTagger/JetQGTagger.h"

#include "InDetTrackSelectionTool/InDetTrackSelectionTool.h"

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

    return StatusCode::SUCCESS;
  }

  StatusCode JetQGTagger::finalize(){

  delete m_trkSelectionTool;

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

  }

} /* namespace CP */

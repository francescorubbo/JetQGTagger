#include "JetQGTagger/JetQGTagger.h"

#include <TRandom3.h>
#include "InDetTrackSelectionTool/InDetTrackSelectionTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthFilterTool.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h"
#include "InDetTrackSystematicsTools/JetTrackFilterTool.h"
#include "PathResolver/PathResolver.h"

#include "CxxUtils/make_unique.h"
using CxxUtils::make_unique;

namespace CP {

  JetQGTagger::JetQGTagger( const std::string& name): asg::AsgTool( name ),
						      m_appliedSystEnum(NONE),
						      m_hquark(nullptr),
						      m_hgluon(nullptr),
						      m_exp_hquark_up(nullptr),
						      m_exp_hquark_down(nullptr),
						      m_exp_hgluon_up(nullptr),
						      m_exp_hgluon_down(nullptr),
						      m_me_hquark_up(nullptr),
						      m_me_hquark_down(nullptr),
						      m_me_hgluon_up(nullptr),
						      m_me_hgluon_down(nullptr),
						      m_pdf_hquark_up(nullptr),
						      m_pdf_hquark_down(nullptr),
						      m_pdf_hgluon_up(nullptr),
						      m_pdf_hgluon_down(nullptr),
						      m_trkSelectionTool(name+"_trackselectiontool", this),
						      m_trkTruthFilterTool(name+"_trackfiltertool",this),
						      m_trkFakeTool(name+"_trackfaketool",this),
						      m_jetTrackFilterTool(name+"_jettrackfiltertool",this)
  {
    declareProperty( "Tagger", m_taggername = "ntrack");
    declareProperty( "ExpWeightFile", m_expfile = "JetQGTagger/qgsyst_exp.root");
    declareProperty( "MEWeightFile", m_mefile = "JetQGTagger/qgsyst_me.root");
    declareProperty( "PDFWeightFile", m_pdffile = "JetQGTagger/qgsyst_pdf.root");
    declareProperty( "MinPt", m_minpt = 50e3);
    declareProperty( "MinEta", m_maxeta = 2.1);
    declareProperty( "WeightDecorationName", m_weight_decoration_name = "qgTaggerWeight");
    declareProperty( "TaggerDecorationName", m_tagger_decoration_name = "qgTagger");
    applySystematicVariation(SystematicSet()).ignore();
  }

  StatusCode JetQGTagger::initialize(){
    
    m_weightdec = new SG::AuxElement::Decorator< float>(m_weight_decoration_name);
    m_taggerdec  = new SG::AuxElement::Decorator< float>(m_tagger_decoration_name);
    
    assert( ASG_MAKE_ANA_TOOL( m_trkSelectionTool,  InDet::InDetTrackSelectionTool ) );
    assert( m_trkSelectionTool.setProperty( "CutLevel", "Loose" ) );
    assert( m_trkSelectionTool.setProperty( "maxAbsEta", 2.5 ) );
    assert( m_trkSelectionTool.setProperty( "minNSiHits", 7 ) );
    assert( m_trkSelectionTool.setProperty( "maxNPixelSharedHits", 1 ) );
    assert( m_trkSelectionTool.setProperty( "maxOneSharedModule", true ) );
    assert( m_trkSelectionTool.setProperty( "maxNSiHoles", 2 ) );
    assert( m_trkSelectionTool.setProperty( "maxNPixelHoles", 1 ) );
    assert( m_trkSelectionTool.retrieve() );

    assert( ASG_MAKE_ANA_TOOL( m_trkTruthFilterTool, InDet::InDetTrackTruthFilterTool ) );
    assert( ASG_MAKE_ANA_TOOL( m_trkFakeTool, InDet::InDetTrackTruthFilterTool ) );

    m_originTool = make_unique<InDet::InDetTrackTruthOriginTool> ( "InDetTrackTruthOriginTool" );
    assert( m_originTool->initialize() );
    ToolHandle< InDet::IInDetTrackTruthOriginTool > trackTruthOriginToolHandle( m_originTool.get() );
    
    assert( m_trkTruthFilterTool.setProperty( "Seed", 1234 ) );
    assert( m_trkTruthFilterTool.setProperty( "trackOriginTool", trackTruthOriginToolHandle ) );
    assert( m_trkTruthFilterTool.retrieve() );
    CP::SystematicSet systSetTrk = {
      InDet::TrackSystematicMap[InDet::TRK_EFF_LOOSE_GLOBAL],
      InDet::TrackSystematicMap[InDet::TRK_EFF_LOOSE_IBL],
      InDet::TrackSystematicMap[InDet::TRK_EFF_LOOSE_PP0],
      InDet::TrackSystematicMap[InDet::TRK_EFF_LOOSE_PHYSMODEL]
    };
    assert( m_trkTruthFilterTool->applySystematicVariation(systSetTrk) );
    
    assert( m_trkFakeTool.setProperty( "Seed", 1234 ) );
    assert( m_trkFakeTool.setProperty( "trackOriginTool", trackTruthOriginToolHandle ) );
    assert( m_trkFakeTool.retrieve() );
    CP::SystematicSet systSetTrkFake = {
      InDet::TrackSystematicMap[InDet::TRK_FAKE_RATE]
    };
    assert( m_trkFakeTool->applySystematicVariation(systSetTrkFake) );
    
    assert( ASG_MAKE_ANA_TOOL( m_jetTrackFilterTool, InDet::JetTrackFilterTool ) );
    assert( m_jetTrackFilterTool.setProperty( "Seed", 1234 ) );
    assert( m_jetTrackFilterTool.retrieve() );
    CP::SystematicSet systSetJet = {
      InDet::TrackSystematicMap[InDet::TRK_EFF_LOOSE_TIDE]
    };
    assert( m_jetTrackFilterTool->applySystematicVariation(systSetJet) );

    if (!addAffectingSystematic(QGntrackSyst::trackfakes,true) || 
	!addAffectingSystematic(QGntrackSyst::trackefficiency,true) ||
	!addAffectingSystematic(QGntrackSyst::nchargedexp_up,true) ||
	!addAffectingSystematic(QGntrackSyst::nchargedme_up,true) ||
	!addAffectingSystematic(QGntrackSyst::nchargedpdf_up,true) ||
	!addAffectingSystematic(QGntrackSyst::nchargedexp_down,true) ||
	!addAffectingSystematic(QGntrackSyst::nchargedme_down,true) ||
	!addAffectingSystematic(QGntrackSyst::nchargedpdf_down,true)
	) {
      ATH_MSG_ERROR("failed to set up JetQGTagger systematics");
      return StatusCode::FAILURE;
    }
    
    this->loadHist(m_exp_hquark_up,  m_expfile,"h2dquark_up");
    this->loadHist(m_exp_hquark_down,m_expfile,"h2dquark_down");
    this->loadHist(m_exp_hgluon_up,  m_expfile,"h2dgluon_up");
    this->loadHist(m_exp_hgluon_down,m_expfile,"h2dgluon_down");
    this->loadHist(m_me_hquark_up,  m_mefile,"h2dquark_up");
    this->loadHist(m_me_hquark_down,m_mefile,"h2dquark_down");
    this->loadHist(m_me_hgluon_up,  m_mefile,"h2dgluon_up");
    this->loadHist(m_me_hgluon_down,m_mefile,"h2dgluon_down");
    this->loadHist(m_pdf_hquark_up,  m_pdffile,"h2dquark_up");
    this->loadHist(m_pdf_hquark_down,m_pdffile,"h2dquark_down");
    this->loadHist(m_pdf_hgluon_up,  m_pdffile,"h2dgluon_up");
    this->loadHist(m_pdf_hgluon_down,m_pdffile,"h2dgluon_down");
   
    return StatusCode::SUCCESS;
  }

  StatusCode JetQGTagger::finalize(){
    
    delete m_exp_hquark_up; 
    delete m_exp_hquark_down;
    delete m_exp_hgluon_up;
    delete m_exp_hgluon_down;
    delete m_me_hquark_up;
    delete m_me_hquark_down;
    delete m_me_hgluon_up;
    delete m_me_hgluon_down;
    delete m_pdf_hquark_up; 
    delete m_pdf_hquark_down;
    delete m_pdf_hgluon_up;
    delete m_pdf_hgluon_down;
    
    delete m_hquark;
    delete m_hgluon;

    return StatusCode::SUCCESS;
  }

  StatusCode JetQGTagger::setTagger(const xAOD::Jet * jet, const xAOD::Vertex * pv){


    double tagger = -1;
    double weight = -1;

    if(jet->pt()<m_minpt){
      ATH_MSG_WARNING("Jet pT is below allowed range");
    }
    else if(fabs(jet->eta())>m_maxeta){
      ATH_MSG_WARNING("Jet eta is beyond allowed range");
    }
    else{
      int ntrack = 0;
      getNTrack(jet, pv, ntrack);
      tagger = ntrack;
      getNTrackWeight(jet, weight);
    }

    (*m_taggerdec)(*jet) = tagger;
    (*m_weightdec)(*jet) = weight;

    return StatusCode::SUCCESS;
  }

  StatusCode JetQGTagger::getNTrack(const xAOD::Jet * jet, const xAOD::Vertex * pv, int &ntracks){
    ntracks = 0;
    
    std::vector<const xAOD::IParticle*> jettracks;
    jet->getAssociatedObjects<xAOD::IParticle>(xAOD::JetAttribute::GhostTrack,jettracks);
    for (size_t i = 0; i < jettracks.size(); i++) {
      
      const xAOD::TrackParticle* trk = static_cast<const xAOD::TrackParticle*>(jettracks[i]);

      bool acceptSyst = true;
      if ( m_appliedSystEnum==QG_TRACKEFFICIENCY )
      	acceptSyst = ( m_trkTruthFilterTool->accept(trk) && m_jetTrackFilterTool->accept(trk,jet) );
      else if ( m_appliedSystEnum==QG_TRACKFAKES )
      	acceptSyst = m_trkFakeTool->accept(trk); 
      if (!acceptSyst) continue;

      bool accept = (trk->pt()>500 && m_trkSelectionTool->accept(*trk) && 
		     (trk->vertex()==pv ||
		      (!trk->vertex() && 
		       fabs((trk->z0()+trk->vz()-pv->z())*sin(trk->theta()))<3.) ) );
      if (!accept) continue;
      
      ntracks++;
    }

    return StatusCode::SUCCESS;
  }

  StatusCode JetQGTagger::getNTrackWeight(const xAOD::Jet * jet, double &weight){
    weight = 1.0;

    if ( m_appliedSystEnum!=QG_NCHARGEDEXP_UP && m_appliedSystEnum!=QG_NCHARGEDME_UP && m_appliedSystEnum!=QG_NCHARGEDPDF_UP &&
	 m_appliedSystEnum!=QG_NCHARGEDEXP_DOWN && m_appliedSystEnum!=QG_NCHARGEDME_DOWN && m_appliedSystEnum!=QG_NCHARGEDPDF_DOWN )
      return StatusCode::SUCCESS;

    int pdgid = jet->getAttribute<int>("PartonTruthLabelID");
    if ( pdgid<0 ) {
      ATH_MSG_INFO("Undefined pdg ID: setting weight to 1");
      return StatusCode::SUCCESS;
    }// if pdgid<0
    
    const xAOD::Jet* tjet;
    if ( jet->isAvailable< ElementLink<xAOD::JetContainer> >("GhostTruthAssociationLink") ){
      if (jet->auxdata< ElementLink<xAOD::JetContainer> >("GhostTruthAssociationLink").isValid() )
	tjet = * jet->auxdata< ElementLink<xAOD::JetContainer> >("GhostTruthAssociationLink");
      else {
	ATH_MSG_WARNING("Cannot access truth: setting weight to 1");
	return StatusCode::FAILURE;
      } //endelse isValid
    } //endif isAvailable
    else {
      ATH_MSG_WARNING("Cannot access truth: setting weight to 1");
      return StatusCode::FAILURE;
    }//endelse isAvailable
    
    double tjetpt = tjet->pt()*0.001;
    double tjeteta = tjet->eta();
    if( tjetpt<50 || fabs(tjeteta)>2.1){
      ATH_MSG_INFO("Outside of fiducial region: setting weight to 1");
      return StatusCode::SUCCESS;
    }
    
    // compute truth ntrk
    int tntrk = 0;
    for (size_t ind = 0; ind < tjet->numConstituents(); ind++) {
      const xAOD::TruthParticle *part = static_cast<const xAOD::TruthParticle*>(tjet->rawConstituent(ind));
      if (!part) continue;
      if( ! (part->status() == 1) ) continue; // final state
      if ((part->barcode())>2e5) continue;
      if( ! (part->pt()>500.) )  continue; //pt>500 MeV
      if( !(part->isCharged()) ) continue; // charged
      double pt = part->pt();
      if( pt>500 ) tntrk++;
    }

    if ( pdgid==21 ){
      int ptbin = m_hgluon->GetXaxis()->FindBin(tjetpt);
      int ntrkbin = m_hgluon->GetYaxis()->FindBin(tntrk);
      weight = m_hgluon->GetBinContent(ptbin,ntrkbin);
    }//gluon
    else if ( pdgid<6 ){
      int ptbin = m_hquark->GetXaxis()->FindBin(tjetpt);
      int ntrkbin = m_hquark->GetYaxis()->FindBin(tntrk);
      weight = m_hquark->GetBinContent(ptbin,ntrkbin);
    }//quarks
    else{
      ATH_MSG_INFO("Neither quark nor gluon jet: setting weight to 1");
    }
    return StatusCode::SUCCESS;
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
    else if (systVar == QGntrackSyst::nchargedexp_up){ 
      m_appliedSystEnum = QG_NCHARGEDEXP_UP; m_hquark=m_exp_hquark_up; m_hgluon=m_exp_hgluon_up;}
    else if (systVar == QGntrackSyst::nchargedme_up){ 
      m_appliedSystEnum = QG_NCHARGEDME_UP; m_hquark=m_me_hquark_up; m_hgluon=m_me_hgluon_up;}
    else if (systVar == QGntrackSyst::nchargedpdf_up){ 
      m_appliedSystEnum = QG_NCHARGEDPDF_UP; m_hquark=m_pdf_hquark_up; m_hgluon=m_pdf_hgluon_up;}
    else if (systVar == QGntrackSyst::nchargedexp_down){ 
      m_appliedSystEnum = QG_NCHARGEDEXP_DOWN; m_hquark=m_exp_hquark_down; m_hgluon=m_exp_hgluon_down;}
    else if (systVar == QGntrackSyst::nchargedme_down){ 
      m_appliedSystEnum = QG_NCHARGEDME_DOWN; m_hquark=m_me_hquark_down; m_hgluon=m_me_hgluon_down;}
    else if (systVar == QGntrackSyst::nchargedpdf_down){ 
      m_appliedSystEnum = QG_NCHARGEDPDF_DOWN; m_hquark=m_pdf_hquark_down; m_hgluon=m_pdf_hgluon_down;}
    else {
      ATH_MSG_WARNING("unsupported systematic applied");
      return SystematicCode::Unsupported;
    }
    
    ATH_MSG_DEBUG("applied systematic is " << m_appliedSystEnum);
    return SystematicCode::Ok;
  }

  StatusCode JetQGTagger::loadHist(TH2D *&hist,std::string fname,std::string histname){
    std::string filename = PathResolverFindCalibFile(fname);
    if (filename.empty()){
      ATH_MSG_WARNING ( "Could NOT resolve file name " << fname);
      return StatusCode::FAILURE;
    }  else{
      ATH_MSG_DEBUG(" Path found = "<<filename);
    }
    TFile* infile = TFile::Open(filename.c_str());
    hist = dynamic_cast<TH2D*>(infile->Get(histname.c_str()));

    hist->SetDirectory(0);
    return StatusCode::SUCCESS;
  }
  
  
} /* namespace CP */

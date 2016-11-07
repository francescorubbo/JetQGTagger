#ifndef IJETQGTAGGER_H
#define IJETQGTAGGER_H

#include "PATInterfaces/CorrectionCode.h"
#include "PATInterfaces/ISystematicsTool.h"

#include "xAODJet/Jet.h"

namespace CP {

  namespace QGntrackSyst {
    const static SystematicVariation trackefficiency("JET_QG_trackEfficiency");
    const static SystematicVariation trackfakes("JET_QG_trackFakes");
    const static SystematicVariation nchargedexp_up("JET_QG_nchargedExp_up");
    const static SystematicVariation nchargedme_up("JET_QG_nchargedME_up");
    const static SystematicVariation nchargedpdf_up("JET_QG_nchargedPDF_up");
    const static SystematicVariation nchargedexp_down("JET_QG_nchargedExp_down");
    const static SystematicVariation nchargedme_down("JET_QG_nchargedME_down");
    const static SystematicVariation nchargedpdf_down("JET_QG_nchargedPDF_down");
  } //namespace QGntrackSyst

  class IJetQGTagger : public virtual CP::ISystematicsTool {
    
    ASG_TOOL_INTERFACE( CP::IJetQGTagger )
      
      public:
    
    virtual ~IJetQGTagger() {}
    
    virtual StatusCode setTagger(const xAOD::Jet * jet, const xAOD::Vertex * pv) = 0;
    
  };

} // namespace CP

#endif /* IJETQGTAGGER_H */

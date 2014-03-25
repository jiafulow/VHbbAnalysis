# Step 1

There are many ways to make Step 1 in the CMSSW world. We will use the following as reference in building our own:

* Standard PF2PAT <https://twiki.cern.ch/twiki/bin/view/CMSPublic/SWGuidePATReleaseNotes70X>
  * cfg: `$CMSSW_BASE/src/PhysicsTools/PatAlgos/python/patTemplate_cfg.py`
  * cfg: `$CMSSW_BASE/src/PhysicsTools/PatAlgos/test/patTuple_PF2PAT_cfg.py`

* B2G framework <https://twiki.cern.ch/twiki/bin/view/CMS/B2GTopLikeBSM53X>
  * cfg: <http://cvs.web.cern.ch/cvs/cgi-bin/viewcvs.cgi/CMSSW/TopQuarkAnalysis/TopPairBSM/test/ttbsm_cfg.py?view=log> (last checked: V04-02-09)

* JME framework #1 (jetToolbox) <https://twiki.cern.ch/twiki/bin/view/CMS/JetToolbox>
  * cfg: <https://github.com/jstupak/cmssw/blob/jetToolboxRedux/RecoJets/JetProducers/test/jettoolbox_cfg.py> (last checked: b99c01ece7)

* JME framework #2 (Bacon/PUPPI) <https://twiki.cern.ch/twiki/bin/viewauth/CMS/PUPPI>
  * cfg: <https://github.com/violatingcp/Bacon/blob/master/BaconProd/Ntupler/python/makingBacon_MC_PF.py> (last checked: b43e63e061)


* CMG framework (miniAOD) <https://github.com/gpetruc/cmssw/compare/micro-from700>
  * cfg: <https://github.com/gpetruc/cmssw/blob/micro-from700/PhysicsTools/PatAlgos/test/patTuple_micro.py> (last checked: d038714052)

* VHbb framework (NtupleV42) <https://twiki.cern.ch/twiki/bin/viewauth/CMS/VHbbAnalysisNewCode> <https://twiki.cern.ch/twiki/bin/viewauth/CMS/VHbbAnalysisGitCodeMigration>
  * cfg: ?




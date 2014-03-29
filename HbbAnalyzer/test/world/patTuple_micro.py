## import skeleton process
from PhysicsTools.PatAlgos.patTemplate_cfg import cms, process
## switch to uncheduled mode
process.options.allowUnscheduled = cms.untracked.bool(True)
#process.Tracer = cms.Service("Tracer")

process.load("PhysicsTools.PatAlgos.producersLayer1.patCandidates_cff")
process.load("PhysicsTools.PatAlgos.selectionLayer1.selectedPatCandidates_cff")

## ------------------------------------------------------
#  In addition you usually want to change the following
#  parameters:
## ------------------------------------------------------
#
#   process.GlobalTag.globaltag =  ...    ##  (according to https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideFrontierConditions)
#                                         ##
#process.source.fileNames = {'/store/relval/CMSSW_7_0_0/RelValTTbar_13/GEN-SIM-RECO/PU25ns_POSTLS170_V3-v2/00000/5A98DF7C-C998-E311-8FF8-003048FEADBC.root'}
process.source.fileNames = {
'/store/relval/CMSSW_7_0_0/RelValTTbar_13/GEN-SIM-RECO/PU50ns_POSTLS170_V4-v2/00000/265B9219-FF98-E311-BF4A-02163E00EA95.root',
'/store/relval/CMSSW_7_0_0/RelValTTbar_13/GEN-SIM-RECO/PU50ns_POSTLS170_V4-v2/00000/36598DF8-D098-E311-972E-02163E00E744.root',
'/store/relval/CMSSW_7_0_0/RelValTTbar_13/GEN-SIM-RECO/PU50ns_POSTLS170_V4-v2/00000/542AC938-CA98-E311-8928-02163E00E5F5.root',
'/store/relval/CMSSW_7_0_0/RelValTTbar_13/GEN-SIM-RECO/PU50ns_POSTLS170_V4-v2/00000/6A95EE20-CD98-E311-8FAE-02163E00A1F2.root',
'/store/relval/CMSSW_7_0_0/RelValTTbar_13/GEN-SIM-RECO/PU50ns_POSTLS170_V4-v2/00000/822E181D-D898-E311-8A29-02163E00E928.root',
'/store/relval/CMSSW_7_0_0/RelValTTbar_13/GEN-SIM-RECO/PU50ns_POSTLS170_V4-v2/00000/E00EF5A1-CE98-E311-B221-02163E00E8AE.root',
'/store/relval/CMSSW_7_0_0/RelValTTbar_13/GEN-SIM-RECO/PU50ns_POSTLS170_V4-v2/00000/F60ED2AC-CB98-E311-ACBA-02163E00E62F.root'
}

##'/store/relval/CMSSW_7_0_0/RelValTTbar_13/GEN-SIM-RECO/PU50ns_POSTLS170_V4-v2/00000/36598DF8-D098-E311-972E-02163E00E744.root'}
#                                         ##
process.maxEvents.input = -1

process.load("SimGeneral.HepPDTESSource.pythiapdt_cfi")

process.load("PhysicsTools.PatAlgos.slimming.slimming_cff")

process.muonMatch.matched = "prunedGenParticles"
process.electronMatch.matched = "prunedGenParticles"
process.photonMatch.matched = "prunedGenParticles"
process.tauMatch.matched = "prunedGenParticles"
process.patJetPartonMatch.matched = "prunedGenParticles"
process.patJetGenJetMatch.matched = "slimmedGenJets"
process.patMuons.embedGenMatch = False
process.patElectrons.embedGenMatch = False
process.patPhotons.embedGenMatch = False
process.patTaus.embedGenMatch = False
process.patJets.embedGenPartonMatch = False

process.patMuons.isoDeposits = cms.PSet()
process.patElectrons.isoDeposits = cms.PSet()
process.patTaus.isoDeposits = cms.PSet()
process.patPhotons.isoDeposits = cms.PSet()

process.patMuons.embedTrack         = True  # used for IDs
process.patMuons.embedCombinedMuon  = True  # used for IDs
process.patMuons.embedMuonBestTrack = True  # used for IDs
process.patMuons.embedStandAloneMuon = True # maybe?
process.patMuons.embedPickyMuon = False   # no, use best track
process.patMuons.embedTpfmsMuon = False   # no, use best track
process.patMuons.embedDytMuon   = False   # no, use best track

process.selectedPatJets.cut = cms.string("pt > 10")
process.selectedPatMuons.cut = cms.string("pt > 3") 
process.selectedPatElectrons.cut = cms.string("pt > 5") 
process.selectedPatTaus.cut = cms.string("pt > 20")

process.slimmedJets.clearDaughters = False
#process.slimmedElectrons.dropRecHits = True
#process.slimmedElectrons.dropBasicClusters = True
#process.slimmedElectrons.dropPFlowClusters = True
#process.slimmedElectrons.dropPreshowerClusters = True

from PhysicsTools.PatAlgos.tools.trigTools import switchOnTriggerStandAlone
switchOnTriggerStandAlone( process )
process.patTrigger.packTriggerPathNames = cms.bool(True)

#                                         ##
#   process.options.wantSummary = False   ##  (to suppress the long output at the end of the job)
#                                         ##

# apply type I/type I + II PFMEt corrections to pat::MET object
# and estimate systematic uncertainties on MET
from PhysicsTools.PatAlgos.tools.jetTools import addJetCollection
from PhysicsTools.PatUtils.tools.metUncertaintyTools import runMEtUncertainties
addJetCollection(process, postfix   = "ForMetUnc", labelName = 'AK5PF', jetSource = cms.InputTag('ak5PFJets'), jetCorrections = ('AK5PF', ['L1FastJet', 'L2Relative', 'L3Absolute'], ''), btagDiscriminators = ['combinedSecondaryVertexBJetTags' ] )
runMEtUncertainties(process,jetCollection="selectedPatJetsAK5PFForMetUnc", outputModule=None)

#   process.out.outputCommands = [ ... ]  ##  (e.g. taken from PhysicsTools/PatAlgos/python/patEventContent_cff.py)
#                                         ##
process.out.fileName = 'patTuple_micro.root'
process.out.outputCommands = process.MicroEventContentMC.outputCommands
process.out.dropMetaData = cms.untracked.string('ALL')
process.out.fastCloning= cms.untracked.bool(False)
process.out.overrideInputFileSplitLevels = cms.untracked.bool(True)


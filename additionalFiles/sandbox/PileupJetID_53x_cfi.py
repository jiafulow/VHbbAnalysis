import FWCore.ParameterSet.Config as cms

from RecoJets.JetProducers.PileupJetIDParams_cfi import *

_stdalgos_53x = cms.VPSet(full_53x,cutbased)
_chsalgos_53x = cms.VPSet(full_53x_chs,cutbased)

pileupJetIdProducer = cms.EDProducer('PileupJetIdProducer',
    produceJetIds = cms.bool(True),
    jetids = cms.InputTag(""),
    runMvas = cms.bool(True),
    jets = cms.InputTag("selectedPatJetsPFlow"),
    vertexes = cms.InputTag("offlinePrimaryVertices"),
    algos = cms.VPSet(_stdalgos_53x),

    rho = cms.InputTag("kt6PFJets", "rho"),
    jec = cms.string("AK5PF"),
    applyJec = cms.bool(False),
    inputIsCorrected = cms.bool(True),
    residualsFromTxt = cms.bool(False),
    residualsTxt = cms.FileInPath("RecoJets/JetProducers/data/dummy.txt"),
)

pileupJetIdProducerChs = cms.EDProducer('PileupJetIdProducer',
    produceJetIds = cms.bool(True),
    jetids = cms.InputTag(""),
    runMvas = cms.bool(True),
    jets = cms.InputTag("selectedPatJetsPFlow"),
    vertexes = cms.InputTag("offlinePrimaryVertices"),
    algos = cms.VPSet(_chsalgos_53x),

    rho = cms.InputTag("kt6PFJets", "rho"),
    jec = cms.string("AK5PFchs"),
    applyJec = cms.bool(False),
    inputIsCorrected = cms.bool(True),
    residualsFromTxt = cms.bool(False),
    residualsTxt = cms.FileInPath("RecoJets/JetProducers/data/dummy.txt"),
)

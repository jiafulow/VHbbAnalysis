import FWCore.ParameterSet.Config as cms


# MET
MetEdmNtuple = cms.EDProducer("CandViewNtpWithP4Producer",
    src=cms.InputTag("patMETs"),
    lazyParser=cms.untracked.bool(True),
    prefix=cms.untracked.string(""),
    eventInfo=cms.untracked.bool(True),
    variables = cms.VPSet(
        cms.PSet(
            tag      = cms.untracked.string("mEtSig"),
            quantity = cms.untracked.string("mEtSig"),
            ),
        cms.PSet(
            tag      = cms.untracked.string("significance"),
            quantity = cms.untracked.string("significance"),
            ),
        cms.PSet(
            tag      = cms.untracked.string("sumEt"),
            quantity = cms.untracked.string("sumEt"),
            ),
        ),
    p4_variables = cms.VPSet(
        cms.PSet(
            tag      = cms.untracked.string("p4"),
            quantity = cms.untracked.string("p4"),
            ),
        ),
    p3_variables = cms.VPSet(),
    )

MetType1EdmNtuple = MetEdmNtuple.clone(
    src=cms.InputTag("patMETsType1"),
    eventInfo=cms.untracked.bool(False),
    )


# Muons
MuEdmNtuple = cms.EDProducer("CandViewNtpWithP4Producer",
    src=cms.InputTag("selectedPatMuons"),
    lazyParser=cms.untracked.bool(True),
    prefix=cms.untracked.string(""),
    eventInfo=cms.untracked.bool(False),
    variables = cms.VPSet(
        cms.PSet(
            tag      = cms.untracked.string("charge"),
            quantity = cms.untracked.string("charge")
            ),
        cms.PSet(
            tag      = cms.untracked.string("glbPt"),
            quantity = cms.untracked.string("globalTrack.pt"),
            ),
        cms.PSet(
            tag      = cms.untracked.string("glbPtError"),
            quantity = cms.untracked.string("globalTrack.ptError"),
            ),
        cms.PSet(
            tag      = cms.untracked.string("trkPt"),
            quantity = cms.untracked.string("innerTrack.pt"),
            ),
        cms.PSet(
            tag      = cms.untracked.string("trkPtError"),
            quantity = cms.untracked.string("innerTrack.ptError"),
            ),
        cms.PSet(
            tag      = cms.untracked.string("trackIso"),
            quantity = cms.untracked.string("trackIso"),
            ),
        cms.PSet(
            tag      = cms.untracked.string("ecalIso"),
            quantity = cms.untracked.string("ecalIso"),
            ),
        cms.PSet(
            tag      = cms.untracked.string("hcalIso"),
            quantity = cms.untracked.string("hcalIso"),
            ),
        ),
    p4_variables = cms.VPSet(
        cms.PSet(
            tag      = cms.untracked.string("p4"),
            quantity = cms.untracked.string("p4"),
            ),
        cms.PSet(
            tag      = cms.untracked.string("genp4"),
            quantity = cms.untracked.string("genLepton.p4"),
            ),
        ),
    p3_variables = cms.VPSet(
        cms.PSet(
            tag      = cms.untracked.string("vertex"),
            quantity = cms.untracked.string("vertex"),
            ),
        ),
    )


# Jets
JetEdmNtuple =  cms.EDProducer("CandViewNtpWithP4Producer",
    src=cms.InputTag("cleanCentralPatJets"),
    lazyParser=cms.untracked.bool(True),
    prefix=cms.untracked.string(""),
    eventInfo=cms.untracked.bool(False),
    variables = cms.VPSet(
        cms.PSet(
            tag      = cms.untracked.string("partonFlavour"),
            quantity = cms.untracked.string("partonFlavour")
            ),
        cms.PSet(
            tag      = cms.untracked.string("csv"),
            quantity = cms.untracked.string("bDiscriminator(\"combinedSecondaryVertexBJetTags\") ")
            ),
        cms.PSet(
            tag      = cms.untracked.string("csvmva"),
            quantity = cms.untracked.string("bDiscriminator(\"combinedSecondaryVertexMVABJetTags\") ")
            ),
        cms.PSet(
            tag      = cms.untracked.string("jetProb"),
            quantity = cms.untracked.string("bDiscriminator(\"jetProbabilityBJetTags\")")
            ),
        cms.PSet(
            tag      = cms.untracked.string("jetBProb"),
            quantity = cms.untracked.string("bDiscriminator(\"jetBProbabilityBJetTags\")")
            ),
        cms.PSet(
            tag      = cms.untracked.string("ssvhe"),
            quantity = cms.untracked.string("bDiscriminator(\"simpleSecondaryVertexHighEffBJetTags\") ")
            ),
        cms.PSet(
            tag      = cms.untracked.string("ssvhp"),
            quantity = cms.untracked.string("bDiscriminator(\"simpleSecondaryVertexHighPurBJetTags\") ")
            ),
        cms.PSet(
            tag      = cms.untracked.string("softElePt"),
            quantity = cms.untracked.string("bDiscriminator(\"softElectronByPtBJetTags\")")
            ),
        cms.PSet(
            tag      = cms.untracked.string("softEleIP"),
            quantity = cms.untracked.string("bDiscriminator(\"softElectronByIP3dBJetTags\")")
            ),
        cms.PSet(
            tag      = cms.untracked.string("softMu"),
            quantity = cms.untracked.string("bDiscriminator(\"softMuonBJetTags\") ")
            ),
        cms.PSet(
            tag      = cms.untracked.string("softMuPt"),
            quantity = cms.untracked.string("bDiscriminator(\"softMuonByPtBJetTags\") ")
            ),
        cms.PSet(
            tag      = cms.untracked.string("softMuIp"),
            quantity = cms.untracked.string("bDiscriminator(\"softMuonByIP3dBJetTags\") ")
            ),
        cms.PSet(
            tag      = cms.untracked.string("tche"),
            quantity = cms.untracked.string("bDiscriminator(\"trackCountingHighEffBJetTags\") ")
            ),
        cms.PSet(
            tag      = cms.untracked.string("tchp"),
            quantity = cms.untracked.string("bDiscriminator(\"trackCountingHighPurBJetTags\") ")
            ),
        ),
    p4_variables = cms.VPSet(
        cms.PSet(
            tag      = cms.untracked.string("p4"),
            quantity = cms.untracked.string("p4"),
            ),
        cms.PSet(
            tag      = cms.untracked.string("genp4"),
            quantity = cms.untracked.string("genJet.p4"),
            ),
        ),
    p3_variables = cms.VPSet(
        cms.PSet(
            tag      = cms.untracked.string("vertex"),
            quantity = cms.untracked.string("vertex"),
            ),
        ),
    )


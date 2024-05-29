logLikelyHood = torch.sum(logLikelyHoodVec)
logPMu = torch.log(self.pMu)
logPTau = torch.log(self.pTau)
# a and lambda
logPAVec = torch.log(self.pA)
logPA = torch.sum(logPAVec)
logLambdaVec = torch.log(self.pLambda)
logPLambda = torch.sum(logLambdaVec)
# the E() items
logEMu = torch.log(self.lastMu)
logETau = torch.log(self.lastTau)
# a and lambda
logEAVec = torch.log(self.lastAi)
logEA = torch.sum(logEAVec)
logELambdaVec = torch.log(self.lastLambda)
logELambda = torch.sum(logLambdaVec)

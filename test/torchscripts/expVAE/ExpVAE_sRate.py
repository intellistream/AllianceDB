# FIX ME LATER!!!!
import torch
import torch.nn as nn
import torch.optim as optim
from torch.autograd import Variable
import torch.nn.functional as F
from torch.autograd import Variable
from torchviz import make_dot


class TransformerEncoder(nn.Module):
    def __init__(self, input_dim, hidden_dim, num_layers, num_heads):
        super(TransformerEncoder, self).__init__()
        self.transformer = nn.TransformerEncoder(
            nn.TransformerEncoderLayer(d_model=input_dim, nhead=num_heads, dim_feedforward=hidden_dim),
            num_layers=num_layers)

    def forward(self, x):
        # x = x.permute(1, 0, 2)
        # x = x.permute(1, 0, 2)
        return self.transformer(x)


#
class VAE(nn.Module):
    def __init__(self, input_dim, hidden_dim, latent_dim, num_layers, num_heads):
        super(VAE, self).__init__()

        self.encoder = nn.Sequential(
            # TransformerEncoder(input_dim, hidden_dim, num_layers, num_heads),
            nn.Linear(input_dim, hidden_dim),
            # nn.ReLU(),
            nn.Linear(hidden_dim, hidden_dim),
            nn.ReLU(),
            # nn.Linear(hidden_dim, latent_dim*2 + 2),
        )
        self.lambdaLayer = nn.Sequential(
            nn.Linear(hidden_dim, latent_dim * 2),  # muLambda
        )
        self.muZAmptitude = nn.Sequential(
            nn.Sigmoid()
            # nn.ReLU()
        )
        self.muALayer = nn.Sequential(
            nn.Linear(hidden_dim, 1 + latent_dim),  # mu and ai
            nn.ReLU()
        )
        self.logVarLayer = nn.Sequential(
            nn.Linear(1 + latent_dim, hidden_dim),
            nn.Linear(hidden_dim, hidden_dim),
            nn.Linear(hidden_dim, 1),
            nn.ReLU()
        )
        self.decoder = nn.Sequential(
            nn.Linear(latent_dim, hidden_dim),
            nn.ReLU(),
            nn.Linear(hidden_dim, input_dim),
        )
        self.latent_dim = latent_dim
        self.inputDim = input_dim
        # for mu
        self.priorMu = torch.tensor([0.0])
        self.priorSigma = torch.tensor([1.0])
        self.lastMu = self.priorMu
        self.lastSigma = self.priorSigma
        # for tau, exp(-logvar)
        self.priorA0 = torch.tensor([0.0])
        self.priorB0 = torch.tensor([1.0])
        self.lastA0 = self.priorA0
        self.lastB0 = self.priorB0

    @torch.jit.export
    def loadPriorDist(self, pmu, psigma, pa0, pb0):
        self.lastMu = self.priorMu
        self.lastSigma = self.priorSigma
        self.priorMu = pmu
        self.priorSigma = psigma
        # for tau
        self.lastA0 = self.priorA0
        self.lastB0 = self.priorB0
        self.priorA0 = pa0
        self.priorB0 = pb0

    def reparameterize(self, mu, logvar):
        std = torch.exp(0.5 * logvar)
        eps = torch.randn_like(std)
        z = mu + eps * std
        return z

    @torch.jit.export
    def getLastDist(self):
        return self.lastMu, (0.5 * self.lastSigma).exp(), self.lastA0, self.lastB0

    # get the estmation value of mu
    @torch.jit.export
    def getMuEstimation(self):
        return (self.lastMu), ((0.5 * self.lastSigma).exp())

    def forward(self, x):
        xWarp = self.encoder(x)
        # lambda
        lambdaEvaluate = self.lambdaLayer(xWarp)
        muLambda = lambdaEvaluate[:, :self.latent_dim]
        muLambda = self.muZAmptitude(muLambda)
        muLambda = self.latent_dim / torch.sum(muLambda) * muLambda
        logvarZ = lambdaEvaluate[:, self.latent_dim:(self.latent_dim * 2)]
        # global
        muA = self.muALayer(xWarp)
        mu = muA[:, 0]
        EA = muA[:, 1:]
        logvar = self.logVarLayer(xWarp)
        # update mu
        self.lastMu = mu
        self.lastSigma = logvar
        # update \ta
        # logvar=xWarp[:, self.latent_dim*2+1
        # z = torch.cat((z, mu.unsqueeze(1), logvar.unsqueeze(1)), dim=1)
        kMu = torch.ones_like(muLambda) * torch.mean(mu)
        # self.lastA0 = (self.priorA0 + self.inputDim / 2)
        # self.lastB0 = (self.priorB0 + 0.5 * (torch.sum(x / muZ - kMu)).pow(2))
        # kMu=self.eMu(kMu)
        # print(mu[0])
        kLogVar = torch.ones_like(muLambda) * torch.mean(logvar)
        # print(kMu[0])
        z = self.reparameterize(kMu, kLogVar)
        z = z * muZ
        # z=z*muZ
        x_recon = self.decoder(z)

        return x_recon, muZ, logvarZ, mu, logvar

    @torch.jit.export
    def getDimension(self):
        return self.inputDim, self.latent_dim

    # this one has problems, fix later
    @torch.jit.export
    def lossUnderNormal(self, x_recon, x, mu, logvar):
        recon_loss = F.mse_loss(x_recon, x, reduction='mean')
        mu_prior = self.priorMu
        sigma_prior = self.priorSigma
        a = self.priorA0
        b = self.priorB0
        # print(a)
        # print(b)
        kl_div = -0.5 * torch.sum(
            1 + logvar - torch.log(sigma_prior.pow(2)) - ((mu - mu_prior).pow(2) + logvar.exp()) / sigma_prior.pow(2))
        kl_div += -0.5 * torch.sum(
            1 + logvar - torch.log(b) - torch.lgamma(a) + (a - 1) * (torch.digamma(a) - torch.log(mu)) - (
                    mu / b) - a * torch.exp(torch.log(mu) - torch.log(b)))
        return recon_loss + kl_div

    @torch.jit.export
    def lossUnderPretrain(self, x_recon, x, pmu, mu):
        # recon_loss = F.mse_loss(x_recon, x, reduction='mean')
        recon_loss = 0
        mu_loss = F.mse_loss(mu, pmu, reduction='mean')
        return mu_loss + recon_loss

    def loss_function(self, x_recon, x, muZ, logvarZ, mu, logvar):
        recon_loss = F.mse_loss(x_recon, x, reduction='mean')
        mu_prior = self.priorMu
        sigma_prior = self.priorSigma
        a = self.priorA0
        b = self.priorB0
        # print(a)
        # print(b)
        kl_div = -0.5 * torch.sum(
            1 + logvar - torch.log(sigma_prior.pow(2)) - ((mu - mu_prior).pow(2) + logvar.exp()) / sigma_prior.pow(2))
        kl_div += -0.5 * torch.sum(
            1 + logvar - torch.log(b) - torch.lgamma(a) + (a - 1) * (torch.digamma(a) - torch.log(mu)) - (
                    mu / b) - a * torch.exp(torch.log(mu) - torch.log(b)))
        # kl_divergence = -0.5 * torch.sum(1 + logvarZ - muZ.pow(2) - logvarZ.exp())
        # kl_divergence = -0.5 * torch.sum(1 + logvar - mu.pow(2) - logvar.exp())
        return recon_loss + kl_div
    # a loss function to force mu aligned with pmu


def save_model(model, path, X):
    tx = X.to('cpu')
    # tx=X
    model2 = model.to('cpu')
    # model2=model
    model2.eval()
    X = torch.tensor([1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0])

    traced_model = torch.jit.script(model2)
    ru = traced_model(tx)
    # export the self-defined functions here
    traced_model.save(path)


def draw_model(model, X, fname):
    m2 = model.to('cpu')
    x2 = X.to('cpu')
    dot = make_dot(m2(x2), params=None, show_attrs=False, show_saved=False)
    dot.render(fname, format='pdf')


def genX(num_samples, input_dim, maxBase, noiseAmp):
    amptitude = torch.tensor([maxBase])
    noiseX = torch.randn(1, input_dim) * (noiseAmp * maxBase)
    baseX = torch.ones_like(noiseX) * maxBase
    sx = baseX + noiseX
    sy = amptitude
    scalingFac = torch.rand(1) * 0.5 + 0.5
    sx = sx * scalingFac
    sy = sy * scalingFac
    for i in range(num_samples - 1):
        amptitude = torch.tensor([maxBase])
        noiseX = torch.randn(1, input_dim) * (noiseAmp * maxBase)
        baseX = torch.ones_like(noiseX) * maxBase
        tx = baseX + noiseX
        ty = amptitude
        scalingFac = torch.rand(1) * 0.5 + 0.5
        tx = tx * scalingFac
        ty = ty * scalingFac
        sx = torch.cat((sx, tx), dim=0)
        sy = torch.cat((sy, ty), dim=0)
        # sx=torch.cat((sx,tx.unsqueeze(0)), dim=0)
    return sx, sy


def supervisedTrain(model, X, Y, batch_size, learningRate, epochs, device):
    optimizer = optim.Adam(model.parameters(), lr=learningRate)
    num_samples, input_dim = X.shape
    for epoch in range(1, epochs + 1):
        train_loss = 0
        for batch_idx in range(0, num_samples, batch_size):
            model.train()

            # x = X[batch_idx:batch_idx+batch_size].to('cuda')
            x = X[batch_idx:batch_idx + batch_size].to(device)
            y = Y[batch_idx:batch_idx + batch_size].to(device)
            model.loadPriorDist(torch.mean(x[0]), torch.std(x[0]), torch.tensor(1.0), torch.tensor(1.0))
            optimizer.zero_grad()
            x_recon, muZ, logvarZ, mu, logvar = model(x)
            # loss = model.loss_function(x_recon, x, muZ, logvarZ, mu, logvar)
            loss = model.lossUnderPretrain(x_recon, x, y, mu)
            loss.backward()
            optimizer.step()
            train_loss += loss.item()
            # optimizer.step()
            if batch_idx % 100 == 0:
                print('Epoch {} [{}/{} ({:.0f}%)]\tLoss: {:.6f}'.format(
                    epoch, batch_idx, num_samples,
                    100. * batch_idx / num_samples,
                    loss.item() / len(x)))


def unSupervisedTrain(model, X, batch_size, learningRate, epochs, device):
    optimizer = optim.Adam(model.parameters(), lr=learningRate)
    num_samples, input_dim = X.shape
    for epoch in range(1, epochs + 1):
        train_loss = 0
        for batch_idx in range(0, num_samples, batch_size):
            model.train()

            # x = X[batch_idx:batch_idx+batch_size].to('cuda')
            x = X[batch_idx:batch_idx + batch_size].to(device)
            model.loadPriorDist(torch.mean(x[0]), torch.std(x[0]), torch.tensor(1.0), torch.tensor(1.0))
            optimizer.zero_grad()
            x_recon, muZ, logvarZ, mu, logvar = model(x)
            # loss = model.loss_function(x_recon, x, muZ, logvarZ, mu, logvar)
            loss = model.lossUnderNormal(x_recon, x, mu, logvar)
            loss.backward()
            optimizer.step()
            train_loss += loss.item()
            # optimizer.step()
            if batch_idx % 100 == 0:
                print('Epoch {} [{}/{} ({:.0f}%)]\tLoss: {:.6f}'.format(
                    epoch, batch_idx, num_samples,
                    100. * batch_idx / num_samples,
                    loss.item() / len(x)))


# This function assumes that a single tensor is stored in *.pt bt c++
def loadCppTensorFile(fname):
    cppModule = torch.jit.load(fname)
    return getattr(cppModule, "0")


def pretrainModel(device, prefixTag, saveTag):
    input_dim = 10
    hidden_dim = 64
    latent_dim = 10
    num_layers = 4
    num_heads = 1
    # Define the hyperparameters
    epochs = 100
    batch_size = 100
    learning_rate = 1e-3

    # Generate the input data X
    X = loadCppTensorFile(prefixTag + '_x.pt').to(device)
    Y = loadCppTensorFile(prefixTag + '_y.pt').to(device)
    print(X)

    # model = VAE(input_size, hidden_size, latent_size).to(device)
    model = VAE(input_dim, hidden_dim, latent_dim, num_layers, num_heads)

    model.train()
    model = model.to(device)

    # Train the model

    # Note: first learn the certainties, then get the uncertainties
    supervisedTrain(model, X, Y, batch_size, 1e-3, 200, device)
    # unSupervisedTrain(model, X, batch_size, 1e-3, 1, device)
    # model.eval()
    # model=model.to('cpu')
    # X, Y = genX(1, input_dim, 10, 0.2)

    # print(tmu,tSigma,ta/tb)
    x = X[0:100, :].to(device)
    model.eval()
    x_recon, muZ, logvarZ, mu, logvar = model(x)
    # print(mu,logvar.exp(),muZ)
    tmu, tsigma = model.getMuEstimation()
    # print(tmu, tsigma, Y)

    # [ru,mu,logvar]=model.forward(t)
    # draw_model(model,Y,"linearVAE")
    save_model(model.to('cpu'), saveTag, X.to('cpu'))


def main():
    # Set the device
    # Define the main function
    # X,Y= genX(1,10,10,0.2)
    # print(X,Y)
    # return X,Y
    device = 'cpu'
    prefixTag = 'tensor_sRate'
    pretrainModel(device, prefixTag, "linearVAE_sRate.pt")

    # print(logvar)


if __name__ == '__main__':
    main()

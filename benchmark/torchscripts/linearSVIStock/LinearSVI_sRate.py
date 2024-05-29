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
class LinearSVI(nn.Module):
    def __init__(self, latentDimension):
        super(LinearSVI, self).__init__()

        # Initialize custom parameter tensor
        tz = torch.rand((1, latentDimension))
        sumZ = tz.sum()
        self.latentZ = torch.nn.Parameter((latentDimension * tz) / sumZ)
        self.mu = torch.nn.Parameter(torch.zeros(1))
        self.tau = torch.nn.Parameter(torch.zeros(1))
        # Register the custom parameter tensor
        # self.register_parameter("latentZ", self.latentZ)
        # self.register_parameter("mu", self.mu)
        # self.register_parameter("tau", self.tau)

    def forward(self, data):
        n = data.size(1)
        rows = data.size(0)
        rowX = data[0]
        tempMu = torch.zeros(rows, 1)
        tempTau = torch.zeros(rows, 1)
        for i in range(data.size(0)):
            rowX = data[i]
            xz = torch.mul(self.latentZ, rowX)
            xzPmu = xz - self.mu
            xzPmu2 = torch.mul(xzPmu, xzPmu)
            irMu = (torch.sum(xz) + self.mu) / (1 + n)
            tempMu[i] = irMu
            irTau = (self.tau + n / 2) * torch.reciprocal(
                (1 + 0.5 * torch.sum(xzPmu2) + torch.mul(irMu - self.mu, irMu - self.mu))
            )
            tempTau[i] = irTau
        return tempMu, tempTau

    def setPrior(self, tmu, sigma2):
        self.mu = tmu
        self.tau = 1 / sigma2

    # a loss function to force mu aligned with pmu
    def save_model(self, path):
        tensor_dict = {
            "latentZ": self.latentZ,
            "mu": (self.mu),
            "tau": (self.tau),
        }
        torch.save(tensor_dict, path, pickle_protocol=torch.serialization.DEFAULT_PROTOCOL)
    # Add more tensors with appropriate tags


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
            optimizer.zero_grad()
            mu, tau = model(x)
            # loss = model.loss_function(x_recon, x, muZ, logvarZ, mu, logvar)
            loss = torch.nn.functional.mse_loss(mu, y)
            loss.backward()
            optimizer.step()
            train_loss += loss.item()
            # optimizer.step()
            if batch_idx % 100 == 0:
                print('Epoch {} [{}/{} ({:.0f}%)]\tLoss: {:.6f}'.format(
                    epoch, batch_idx, num_samples,
                    100. * batch_idx / num_samples,
                    loss.item() / len(x)))


def loadCppTensorFile(fname):
    cppModule = torch.jit.load(fname)
    return getattr(cppModule, "0")


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


def main():
    # Set the device
    # Define the main function
    # X,Y= genX(1,10,10,0.2)
    # print(X,Y)
    # return X,Y
    device = 'cpu'
    prefixTag = 'tensor_sRate'
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

    num_samples = 1000
    X = loadCppTensorFile(prefixTag + '_x.pt').to(device)
    Y = loadCppTensorFile(prefixTag + '_y.pt').to(device)
    model = LinearSVI(latent_dim)
    supervisedTrain(model, X, Y, batch_size, 1e-2, 1000, device)
    print(model.latentZ)
    # save_model("linearSVI_sRate.pt")
    save_model(model.to('cpu'), "linearSVI_sRate.pt", X.to('cpu'))

    # print(t.size())
    # print(logvar)


if __name__ == '__main__':
    main()

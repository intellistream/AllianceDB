import torch
import torch.nn as nn
import torch.optim as optim
from torch.autograd import Variable


# Define the VAE model
class VAE(nn.Module):
    def __init__(self, input_size, hidden_size, latent_size):
        super(VAE, self).__init__()

        # Encoder
        self.fc1 = nn.Linear(input_size, hidden_size)
        self.fc21 = nn.Linear(hidden_size, latent_size)
        self.fc22 = nn.Linear(hidden_size, latent_size)

        # Decoder
        self.fc3 = nn.Linear(latent_size, hidden_size)
        self.fc4 = nn.Linear(hidden_size, input_size)

    def encode(self, x):
        h1 = nn.functional.relu(self.fc1(x))
        return self.fc21(h1), self.fc22(h1)

    def reparameterize(self, mu, logvar):
        std = torch.exp(0.5 * logvar)
        eps = torch.randn_like(std)
        return eps.mul(std).add_(mu)

    def decode(self, z):
        h3 = nn.functional.relu(self.fc3(z))
        return self.fc4(h3)

    @torch.jit.export
    def testFunc(self, x: torch.Tensor, y: float) -> tuple[torch.Tensor, float]:
        return x + x + y, sum(x)

    def forward(self, x):
        # self.testFunc(x,0.5)
        mu, logvar = self.encode(x)
        z = self.reparameterize(mu, logvar)
        return self.decode(z), mu, logvar

    # Note: to export something, we must use the @torch.jit.export indicator
    @torch.jit.export
    def loss_function1(self, recon_x, x, mu, logvar):
        BCE = nn.functional.mse_loss(recon_x, x, reduction='sum')
        KLD = -0.5 * torch.sum(1 + logvar - mu.pow(2) - logvar.exp())
        return BCE + KLD

    def myTrain(self, X, epochs, batch_size):
        num_samples = len(X)
        optimizer = optim.Adam(self.parameters(), lr=1e-3)
        for epoch in range(1, epochs + 1):
            self.train()
            train_loss = 0
            for batch_idx in range(0, num_samples, batch_size):
                data = X[batch_idx:batch_idx + batch_size]
                optimizer.zero_grad()
                recon_batch, mu, logvar = self(data)
                loss = self.loss_function1(recon_batch, data, mu, logvar)
                loss.backward()
                train_loss += loss.item()
                optimizer.step()
                if batch_idx % 100 == 0:
                    print('Epoch {} [{}/{} ({:.0f}%)]\tLoss: {:.6f}'.format(
                        epoch, batch_idx, num_samples,
                        100. * batch_idx / num_samples,
                        loss.item() / len(data)))

        print('====> Epoch: {} Average loss: {:.4f}'.format(
            epoch, train_loss / num_samples))
        return train_loss / num_samples


# Define the loss function
def loss_function(recon_x, x, mu, logvar):
    BCE = nn.functional.mse_loss(recon_x, x, reduction='sum')
    KLD = -0.5 * torch.sum(1 + logvar - mu.pow(2) - logvar.exp())
    return BCE + KLD


def save_model(model, path, tx):
    # device = torch.device("cpu")
    # tx=X.to(device)
    # tx=X
    # model2=model.to(device)
    # model2=model
    model.eval()
    X = torch.tensor([1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0])

    traced_model = torch.jit.script(model)
    ru = traced_model(tx)
    # export the self-defined functions here
    traced_model.save(path)


# Set the device
device = torch.device("cuda" if torch.cuda.is_available() else "cpu")


# Define the main function
def main():
    # Define the hyperparameters
    epochs = 1
    batch_size = 128
    learning_rate = 1e-3

    # Generate the input data X
    input_size = 10
    num_samples = 1000

    noiseX = torch.randn(num_samples, input_size)
    baseX = torch.ones_like(noiseX) * 5
    X = baseX + noiseX
    X = torch.tensor([1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0])
    # X = torch.randn(num_samples, input_size)
    print(X)
    # Initialize the model and optimizer
    hidden_size = 50
    latent_size = 10
    # model = VAE(input_size, hidden_size, latent_size).to(device)
    model = VAE(input_size, hidden_size, latent_size)
    optimizer = optim.Adam(model.parameters(), lr=learning_rate)
    save_model(model, "vae_raw.pt", X)
    # Train the model

    model.myTrain(X, 1000, 128)
    t = torch.tensor([1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0])
    print(t)
    [ru, mu, logvar] = model.forward(t)
    save_model(model, "vae.pt", X)
    # print(t.size())
    # print(logvar)


if __name__ == '__main__':
    main()

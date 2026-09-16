import matplotlib.pyplot as plt
import random
import itertools
import numpy as np

numCoeffs = 41
populationSize = 20
generations = 50
mutationRate = 0.15
functionRange = (-np.pi, np.pi)
sampleCount = 100

def getTargetFunction(functionName="sin_cos"):
    def sinCosFunction(t):
        """Target function: sin(2πt) + 0.5*cos(4πt)."""
        return np.sin(2 * np.pi * t) + 0.5 * np.cos(4 * np.pi * t)

    def linearFunction(t):
        """Simple linear function: y = 2t + 1."""
        return 2 * t + 1

    def quadraticFunction(t):
        """Quadratic function: y = 4t^2 - 4t + 2."""
        return 4 * (t**2) - 4 * t + 2

    def cubicFunction(t):
        """Cubic function: y = 8t^3 - 12t^2 + 6t."""
        return 8 * (t**3) - 12 * (t**2) + 6 * t

    def gaussianFunction(t):
        """Gaussian function centered at t=0.5."""
        mu = 0.5
        sigma = 0.1 
        return np.exp(-((t - mu) ** 2) / (2 * sigma**2))

    def squareWaveFunction(t):
        """Approximation of a square wave. Smoothed for better Fourier approximation."""
        return 0.5 * (np.sign(np.sin(2 * np.pi * t)) + 1)

    def sawtoothFunction(t):
        """Sawtooth wave, normalized to [0, 1]."""
        return (t * 5) % 1

    def complexFourierFunction(t):
        return (
            np.sin(2 * np.pi * t)
            + 0.3 * np.cos(4 * np.pi * t)
            + 0.2 * np.sin(6 * np.pi * t)
            + 0.1 * np.cos(8 * np.pi * t)
        )

    def polynomialFunction(t):
        return 10 * (t**5) - 20 * (t**4) + 15 * (t**3) - 4 * (t**2) + t + 0.5

    functionOptions = {
        "sin_cos": sinCosFunction,
        "linear": linearFunction,
        "quadratic": quadraticFunction,
        "cubic": cubicFunction,
        "gaussian": gaussianFunction,
        "square_wave": squareWaveFunction,
        "sawtooth": sawtoothFunction,
        "complex_fourier": complexFourierFunction,
        "polynomial": polynomialFunction,
    }

    selectedFunction = functionOptions.get(functionName.lower())
    if selectedFunction:
        return selectedFunction
    
tSamples = np.linspace(functionRange[0], functionRange[1], sampleCount)
fSamples = getTargetFunction()(tSamples)


def choromosomeGen():
    return [random.uniform(-1,1) for _ in range(numCoeffs)]

def initPop():
    return [choromosomeGen() for _ in range(populationSize)]

population = initPop()

def evaluateChoromosome(chromosome, tValues):
    result = np.zeros_like(tValues)
    for k, coeff in enumerate(chromosome):
        if k % 2 == 0:
            result += coeff * np.sin((k+1) * tValues)
        else:
            result += coeff * np.cos((k+1) * tValues)
    return result

def fitness(chromosome):
    approx = evaluateChoromosome(chromosome, tSamples)
    rmse = np.sqrt(np.mean((fSamples - approx) ** 2))
    return -rmse

def getBestChromosome(population):
    best = None
    bestFitness = float('-inf')
    for chromosome in population:
        fit = fitness(chromosome)
        if fit > bestFitness:
            bestFitness = fit
            best = chromosome
    return best, bestFitness

bestChromosome, bestFitness = getBestChromosome(population)

plt.figure(figsize=(10, 5))
plt.plot(tSamples, fSamples, label="Target Function", linewidth=2)
plt.plot(tSamples, evaluateChoromosome(bestChromosome, tSamples), label="Best Approximation", linestyle="--")
plt.title("Best Chromosome Approximation vs Target Function")
plt.legend()
plt.grid(True)
plt.show()
from sklearn.metrics import mean_squared_error, mean_absolute_error, r2_score
import matplotlib.pyplot as plt
import random
import itertools
import numpy as np

# algorithm parameters
numCoeffs = 41
populationSize = 100
generations = 50
mutationRate = 0.15
functionRange = (-np.pi, np.pi)
sampleCount = 100

# These functions are given as samples to use in the algorithm
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
        sigma = 0.1  # Adjust sigma to control the width of the peak
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
    
# generate samples
tSamples = np.linspace(functionRange[0], functionRange[1], sampleCount)
fSamples = getTargetFunction()(tSamples)

# Generate one chromosome
def choromosomeGen():
    return [random.uniform(-1, 1) for _ in range(numCoeffs)]

# Generate initial population
def initPop():
    return [choromosomeGen() for _ in range(populationSize)]

# Crossover: single-point crossover
def crossover(parent1, parent2):
    point = random.randint(1, numCoeffs - 1)
    return parent1[:point] + parent2[point:]

# Mutation: random perturbation with mutationRate probability
def mutate(chromosome):
    return [gene + random.uniform(-0.5, 0.5) if random.random() < mutationRate else gene for gene in chromosome]

# Tournament selection: select best of k random individuals
def tournamentSelection(population, k=3):
    selected = random.sample(population, k)
    selected.sort(key=fitness, reverse=True)
    return selected[0]

# Evaluate approximation of chromosome
def evaluateChoromosome(chromosome, tValues):
    result = np.zeros_like(tValues)
    for k, coeff in enumerate(chromosome):
        if k % 2 == 0:
            result += coeff * np.sin((k+1) * tValues)
        else:
            result += coeff * np.cos((k+1) * tValues)
    return result

# Fitness function: negative MSE
def fitness(chromosome):
    approx = evaluateChoromosome(chromosome, tSamples)
    mse = np.mean((fSamples - approx) ** 2)
    return -mse

# Return best chromosome in population
def getBestChromosome(population):
    best = None
    bestFitness = float('-inf')
    for chromosome in population:
        fit = fitness(chromosome)
        if fit > bestFitness:
            bestFitness = fit
            best = chromosome
    return best, bestFitness

# Generate new population
def generateNewPopulation(population):
    newPop = []
    while len(newPop) < populationSize:
        parent1 = tournamentSelection(population)
        parent2 = tournamentSelection(population)
        child = crossover(parent1, parent2)
        child = mutate(child)
        newPop.append(child)
    return newPop

# Main evolution loop
population = initPop()
bestChromosome, bestFitness = getBestChromosome(population)

for generation in range(generations):
    population = generateNewPopulation(population)
    currentBest, currentFitness = getBestChromosome(population)
    if currentFitness > bestFitness:
        bestChromosome = currentBest
        bestFitness = currentFitness
    print(f"Generation {generation+1}: Best Fitness = {bestFitness:.6f}")

# Evaluate the best approximation
approx = evaluateChoromosome(bestChromosome, tSamples)

# Calculate error metrics
mse = mean_squared_error(fSamples, approx)
rmse = np.sqrt(mse)
mae = mean_absolute_error(fSamples, approx)
r2 = r2_score(fSamples, approx)

# Print metrics
print(f"MSE  = {mse:.6f}")
print(f"RMSE = {rmse:.6f}")
print(f"MAE  = {mae:.6f}")
print(f"R²   = {r2:.6f}")

# Plot result
plt.figure(figsize=(10, 6))
plt.plot(tSamples, fSamples, label="Target Function", linewidth=2)
plt.plot(tSamples, approx, label="Best Approximation", linestyle="--")
plt.title("Best Chromosome Approximation After Evolution")
plt.figtext(0.15, 0.02, f"MSE={mse:.6f} | RMSE={rmse:.6f} | MAE={mae:.6f} | R²={r2:.6f}", fontsize=10, ha='left')
plt.xlabel("t")
plt.ylabel("f(t)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
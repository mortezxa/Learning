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


def generate_individual():
    return [random.uniform(-1, 1) for _ in range(numCoeffs)]

def create_initial_population():
    return [generate_individual() for _ in range(populationSize)]

def one_point_crossover(p1, p2):
    idx = random.randint(1, numCoeffs - 1)
    return p1[:idx] + p2[idx:]

def apply_mutation(individual):
    return [gene + random.uniform(-0.5, 0.5) if random.random() < mutationRate else gene for gene in individual]

def select_via_tournament(pop, k=3):
    candidates = random.sample(pop, k)
    candidates.sort(key=fitness, reverse=True)
    return candidates[0]

def evaluate(individual, t_vals):
    output = np.zeros_like(t_vals)
    for i, coef in enumerate(individual):
        if i % 2 == 0:
            output += coef * np.sin((i + 1) * t_vals)
        else:
            output += coef * np.cos((i + 1) * t_vals)
    return output

def fitness(individual):
    prediction = evaluate(individual, tSamples)
    error = np.sqrt(np.mean((fSamples - prediction) ** 2))
    return -error

def find_best(pop):
    top = None
    top_score = float('-inf')
    for ind in pop:
        score = fitness(ind)
        if score > top_score:
            top_score = score
            top = ind
    return top, top_score

def evolve(pop):
    new_generation = []
    while len(new_generation) < populationSize:
        parent1 = select_via_tournament(pop)
        parent2 = select_via_tournament(pop)
        offspring = one_point_crossover(parent1, parent2)
        offspring = apply_mutation(offspring)
        new_generation.append(offspring)
    return new_generation

# Evolution process
population = create_initial_population()
best_individual, best_score = find_best(population)

for gen in range(generations):
    population = evolve(population)
    current_best, current_score = find_best(population)
    if current_score > best_score:
        best_individual = current_best
        best_score = current_score
    print(f"Generation {gen + 1}: Best Fitness = {best_score:.6f}")

# Final plot
plt.figure(figsize=(10, 5))
plt.plot(tSamples, fSamples, label="Target Function", linewidth=2)
plt.plot(tSamples, evaluate(best_individual, tSamples), label="Best Approximation", linestyle="--")
plt.title("Best Chromosome Approximation After Evolution")
plt.legend()
plt.grid(True)
plt.show()
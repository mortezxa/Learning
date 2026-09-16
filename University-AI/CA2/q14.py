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

#generation samples and
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
    mse = np.mean((fSamples - approx) ** 2)
    return -mse

# Evaluate all chromosomes and find the best one
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

# تعریف معیارها
def mse(y_true, y_pred):
    return np.mean((y_true - y_pred)**2)

def rmse(y_true, y_pred):
    return np.sqrt(np.mean((y_true - y_pred)**2))

def mae(y_true, y_pred):
    return np.mean(np.abs(y_true - y_pred))

def r_squared(y_true, y_pred):
    ss_res = np.sum((y_true - y_pred)**2)
    ss_tot = np.sum((y_true - np.mean(y_true))**2)
    return 1 - (ss_res / ss_tot)

# تابع ارزیابی جمعیت نسبت به هر معیار
def evaluatePopulation(population, metricFunc, maximize=False):
    bestChrom = None
    bestScore = float('-inf') if maximize else float('inf')
    bestPred = None

    for chrom in population:
        prediction = evaluateChoromosome(chrom, tSamples)
        score = metricFunc(fSamples, prediction)
        if (maximize and score > bestScore) or (not maximize and score < bestScore):
            bestScore = score
            bestChrom = chrom
            bestPred = prediction

    return bestChrom, bestScore, bestPred

# لیست معیارها و مشخصاتشون (max یا min)
metrics = {
    "MSE": (mse, False),
    "RMSE": (rmse, False),
    "MAE": (mae, False),
    "R-squared": (r_squared, True)
}

# فقط یک بار figure تعریف می‌کنیم
plt.figure(figsize=(10, 6))

# رنگ‌های واضح‌تر برای تمایز
colors = ['r', 'g', 'b', 'orange']

# رسم هر معیار روی نمودار
for i, (name, (metricFunc, maximize)) in enumerate(metrics.items()):
    bestChrom, score, prediction = evaluatePopulation(population, metricFunc, maximize)
    plt.plot(tSamples, prediction, label=f"{name}: {score:.4f}", color=colors[i], linewidth=2)

# رسم تابع هدف (Target)
plt.plot(tSamples, fSamples, label="Target Function", color='k', linewidth=3, linestyle='--')

# عنوان و تنظیمات
plt.title('Comparison of Metrics')
plt.xlabel('Time')
plt.ylabel('Function Value')
plt.legend()
plt.grid(True)
plt.tight_layout()

# فقط یک بار نمایش نمودار
plt.show()
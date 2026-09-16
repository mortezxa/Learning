import numpy as np
import matplotlib.pyplot as plt

# پارامترهای کلی
number_population = 100
number_gens = 20
gens_min = -10
gens_max = 10
T = 1
generations = 50
mutation_rate = 0.1

# نقاط تصادفی از تابع واقعی
t_values = np.linspace(0, T, 100)
f_true = np.sin(2 * np.pi * t_values)

# تابع بازسازی تابع با استفاده از کروموزوم
def reconstruct_function(chromosome, t_values, T):
    a_n = chromosome[:number_gens]
    b_n = chromosome[number_gens:]
    result = np.zeros_like(t_values)
    for i in range(number_gens):
        result += a_n[i] * np.cos(2 * np.pi * i * t_values / T) + b_n[i] * np.sin(2 * np.pi * i * t_values / T)
    return result

# تابع محاسبه fitness (اینجا از MSE استفاده می‌کنیم)
def fitness(chromosome):
    reconstructed = reconstruct_function(chromosome, t_values, T)
    error = reconstructed - f_true
    return np.mean(error**2)

# تابع crossover تک نقطه‌ای
def crossover(parent1, parent2):
    point = np.random.randint(1, len(parent1) - 1)
    child1 = np.concatenate((parent1[:point], parent2[point:]))
    child2 = np.concatenate((parent2[:point], parent1[point:]))
    return child1, child2

# تابع mutation با احتمال تغییر هر ژن
def mutate(chromosome, mutation_rate=0.1):
    for i in range(len(chromosome)):
        if np.random.rand() < mutation_rate:
            chromosome[i] = np.random.uniform(gens_min, gens_max)
    return chromosome

# جمعیت اولیه
population = np.random.uniform(gens_min, gens_max, (number_population, number_gens * 2))

# لیست‌ها برای ذخیره نتایج هر نسل
mse_over_time = []
rmse_over_time = []
mae_over_time = []

# الگوریتم ژنتیک
for gen in range(generations):
    fitness_values = np.array([fitness(chrom) for chrom in population])
    sorted_indices = np.argsort(fitness_values)
    population = population[sorted_indices]

    # انتخاب بهترین‌ها برای تولید نسل بعد
    next_population = population[:number_population // 2]

    # تولید نسل جدید با crossover و mutation
    children = []
    while len(children) < number_population // 2:
        parents = np.random.choice(len(next_population), 2, replace=False)
        p1, p2 = next_population[parents[0]], next_population[parents[1]]
        c1, c2 = crossover(p1, p2)
        children.append(mutate(c1, mutation_rate))
        children.append(mutate(c2, mutation_rate))

    population = np.vstack((next_population, children[:number_population // 2]))

    # ثبت مقادیر خطا برای بهترین کروموزوم نسل فعلی
    best_chrom = population[0]
    reconstructed = reconstruct_function(best_chrom, t_values, T)
    error = reconstructed - f_true
    mse = np.mean(error**2)
    rmse = np.sqrt(mse)
    mae = np.mean(np.abs(error))

    mse_over_time.append(mse)
    rmse_over_time.append(rmse)
    mae_over_time.append(mae)

# رسم نمودار
plt.figure(figsize=(15, 4))

plt.subplot(1, 3, 1)
plt.plot(mse_over_time, 'r')
plt.title('MSE over Generations')
plt.xlabel('Generation')
plt.ylabel('MSE')

plt.subplot(1, 3, 2)
plt.plot(rmse_over_time, 'g')
plt.title('RMSE over Generations')
plt.xlabel('Generation')
plt.ylabel('RMSE')

plt.subplot(1, 3, 3)
plt.plot(mae_over_time, 'b')
plt.title('MAE over Generations')
plt.xlabel('Generation')
plt.ylabel('MAE')

plt.tight_layout()
plt.show()
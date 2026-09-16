import numpy as np

number_population = 100
number_gens = 20
gens_min = -2
gens_max = 2
T = gens_max - gens_min

pop = np.linspace(gens_min, gens_max, number_population)

a_0_list = np.zeros(number_population)
a_n_list = np.zeros((number_population, number_gens))
b_n_list = np.zeros((number_population, number_gens))

for i in range(number_population):
    f_pop = np.sin(2*np.pi*pop[i])

    a_0_list[i] = (2/T) * np.sum(f_pop)
    for n in range(1, number_gens+1):
        a_n_list[i, n-1] = (2/T) * np.sum(f_pop * np.cos(2*np.pi*n*pop[i]/T))
        b_n_list[i, n-1] = (2/T) * np.sum(f_pop * np.sin(2*np.pi*n*pop[i]/T))

print(a_0_list[0])
print(a_n_list[0])
print(b_n_list[0])
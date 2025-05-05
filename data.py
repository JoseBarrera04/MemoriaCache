import matplotlib.pyplot as plt

with open('data/misshitsplot.txt', 'r') as file:
    lines = file.readlines()

hit_percentage = float(lines[0].split(":")[1].strip().replace('%', ''))
miss_percentage = float(lines[1].split(":")[1].strip().replace('%', ''))
titulos = ['Hits', 'Misses']
porcentajes = [hit_percentage, miss_percentage]

plt.bar(titulos, porcentajes, color = ['blue', 'red'])
plt.title('Grafica de HITS y MISSES')
plt.xlabel('Resultad0')
plt.ylabel('Porcentaje %')
plt.ylim(0, 100)
plt.show()

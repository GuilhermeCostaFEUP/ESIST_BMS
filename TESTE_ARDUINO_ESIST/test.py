import numpy as np
import matplotlib.pyplot as plt

# Parâmetros da bateria
capacidade_nominal = 2.5  # Ah
soc_inicial = 1.0         # 100%

# Tempo de simulação
tempo_total_s = 3600
dt = 1  # passo de tempo (s)
tempo = np.arange(0, tempo_total_s, dt)

# Corrente como função curva suave (ex: senoidal decrescente)
# Começa com 0.5A, oscila suavemente, e termina por volta de -2A
corrente = 0.5 * np.cos(2 * np.pi * tempo / tempo_total_s) - 1.25

# Inicializar SoC
soc = [soc_inicial]

# Contagem de coulombs
for i in range(1, tempo_total_s):
    corrente_inst = corrente[i]
    delta_soc = -(corrente_inst * dt / 3600) / capacidade_nominal
    novo_soc = soc[-1] + delta_soc
    novo_soc = max(0, min(1, novo_soc))  # limitar entre 0 e 1
    soc.append(novo_soc)

    # Mostrar dados no terminal a cada 5 minutos
    if i % 300 == 0:
        print(f"Tempo: {i//60:02d} min - Corrente: {corrente_inst:.2f} A - SoC: {novo_soc*100:.1f}%")

# Plot
tempo_horas = tempo / 3600

fig, ax1 = plt.subplots()

color = 'tab:blue'
ax1.set_xlabel('Tempo (h)')
ax1.set_ylabel('SoC [%]', color=color)
ax1.plot(tempo_horas, np.array(soc) * 100, color=color)
ax1.tick_params(axis='y', labelcolor=color)
ax1.grid(True)

ax2 = ax1.twinx()
color = 'tab:orange'
ax2.set_ylabel('Corrente (A)', color=color)
ax2.plot(tempo_horas, corrente, color=color, linestyle='--')
ax2.tick_params(axis='y', labelcolor=color)

plt.title('SoC com Corrente Curva Suave (Coulomb Counting)')
plt.tight_layout()
plt.show()

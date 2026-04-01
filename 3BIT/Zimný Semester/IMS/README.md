# Preklad

```bash
# Kompilácia
make

# Spustenie základnej simulácie
make run

# Spustenie všetkých experimentov
make experiments

make doc

make clean
```
# Spustenie

```bash
# Pracovný deň (1 deň)
./kampus_sim

# Víkend (1 deň)
./kampus_sim --weekend

# Viacero dní
./kampus_sim --days 5

# Víkend, 2 dni
./kampus_sim --weekend --days 2

# Nápoveda
./kampus_sim --help
```

# Vizualizácia

```bash
# Vytvorenie virtuálneho stroja
python3 -m venv venv

# Spustenie VM
source venv/bin/activate

# Inštalácia knižníc
pip install -r requirements.txt

# Spustenie vizualizácie
python visualize.py
```
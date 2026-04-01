#!/usr/bin/env python3

#/*******************************************************************************
#*                                                                              *
#*                        Brno University of Technology                         *
#*                      Faculty of Information Technology                       *
#*                                                                              *
#*                               Modely a Simulácie                             *
#*                                                                              *
#*            Author: Hugo Bohácsek [xbohach00 AT stud.fit.vutbr.cz]            *
#*            Author: Adam Sloboda [xsloboa00 AT stud.fit.vutbr.cz]             *
#*                                   Brno 2025                                  *
#*                                                                              *
#*     Implementation of the DPMB Campus terminus simulation visualisation      *
#*                                                                              *
#*******************************************************************************/

import sys
import matplotlib
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt


# Slovak font support
matplotlib.rcParams['font.family'] = 'DejaVu Sans'

def parse_percentage(value):
    """Parsuje hodnotu s % na číslo"""
    if isinstance(value, str):
        return float(value.replace('%', '').strip())
    return float(value)

def load_results(filename):
    """Načítanie výsledkov zo CSV súboru"""
    try:
        df = pd.read_csv(filename, encoding='utf-8')
        results = {}
        for _, row in df.iterrows():
            key = row['Metrika']
            value = row['Hodnota']
            
            # Parsovanie hodnôt
            if isinstance(value, str):
                if '%' in value:
                    results[key] = parse_percentage(value)
                else:
                    try:
                        results[key] = float(value)
                    except:
                        results[key] = value
            else:
                results[key] = value
        
        print(f"Načítané z {filename}:")
        for key, value in results.items():
            print(f"  {key}: {value}")
        return results
    except Exception as e:
        print(f"Chyba pri načítaní {filename}: {e}")
        return None

def plot_track_utilization(workday_results, weekend_results):
    """Graf využitia koľají"""
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
    
    tracks = ['A', 'B', 'C', 'D']
    
    # Pracovný deň
    if workday_results:
        utilization_wd = [
            workday_results.get('Vyuzitie kolaje A', 0.0),
            workday_results.get('Vyuzitie kolaje B', 0.0),
            workday_results.get('Vyuzitie kolaje C', 0.0),
            workday_results.get('Vyuzitie kolaje D', 0.0)
        ]
        colors_wd = ['#2ecc71' if u < 50 else '#f39c12' if u < 80 else '#e74c3c' 
                     for u in utilization_wd]
        bars1 = ax1.bar(tracks, utilization_wd, color=colors_wd, alpha=0.8, edgecolor='black')
        ax1.set_ylabel('Využitie (%)', fontsize=12)
        ax1.set_title('Pracovný deň', fontsize=14, fontweight='bold')
        ax1.set_ylim([0, 100])
        ax1.grid(axis='y', alpha=0.3)
        
        # Hodnoty na stĺpcoch
        for bar in bars1:
            height = bar.get_height()
            ax1.text(bar.get_x() + bar.get_width()/2., height,
                    f'{height:.1f}%',
                    ha='center', va='bottom', fontsize=10)
    
    # Víkend
    if weekend_results:
        utilization_we = [
            weekend_results.get('Vyuzitie kolaje A', 0.0),
            weekend_results.get('Vyuzitie kolaje B', 0.0),
            weekend_results.get('Vyuzitie kolaje C', 0.0),
            weekend_results.get('Vyuzitie kolaje D', 0.0)
        ]
        colors_we = ['#2ecc71' if u < 50 else '#f39c12' if u < 80 else '#e74c3c' 
                     for u in utilization_we]
        bars2 = ax2.bar(tracks, utilization_we, color=colors_we, alpha=0.8, edgecolor='black')
        ax2.set_ylabel('Využitie (%)', fontsize=12)
        ax2.set_title('Víkend', fontsize=14, fontweight='bold')
        ax2.set_ylim([0, 100])
        ax2.grid(axis='y', alpha=0.3)
        
        # Hodnoty na stĺpcoch
        for bar in bars2:
            height = bar.get_height()
            ax2.text(bar.get_x() + bar.get_width()/2., height,
                    f'{height:.1f}%',
                    ha='center', va='bottom', fontsize=10)
    
    plt.suptitle('Využitie koľají', fontsize=16, fontweight='bold', y=1.02)
    plt.tight_layout()
    plt.savefig('graf_vyuzitie_kolaji.png', dpi=300, bbox_inches='tight')
    print("Graf uložený: graf_vyuzitie_kolaji.png")
    plt.close()

def plot_comparison(workday_results, weekend_results):
    """Porovnanie kľúčových metrík"""
    fig, ((ax1, ax2), (ax3, ax4)) = plt.subplots(2, 2, figsize=(14, 10))
    
    categories = ['Pracovný\ndeň', 'Víkend']
    width = 0.35
    x = np.arange(len(categories))
    
    # Počet električiek
    if workday_results and weekend_results:
        trams = [workday_results['Celkovy pocet elektricky'],
                weekend_results['Celkovy pocet elektricky']]
        bars = ax1.bar(x, trams, width, color=['#3498db', '#9b59b6'], alpha=0.8, edgecolor='black')
        ax1.set_ylabel('Počet', fontsize=12)
        ax1.set_title('Celkový počet električiek', fontsize=12, fontweight='bold')
        ax1.set_xticks(x)
        ax1.set_xticklabels(categories)
        ax1.grid(axis='y', alpha=0.3)
        for bar in bars:
            height = bar.get_height()
            ax1.text(bar.get_x() + bar.get_width()/2., height,
                    f'{int(height)}',
                    ha='center', va='bottom', fontsize=10)
    
    # Meškajúce električky (%)
    if workday_results and weekend_results:
        delayed_pct = [
            (workday_results['Meskajuce elektricky'] / 
             workday_results['Celkovy pocet elektricky'] * 100),
            (weekend_results['Meskajuce elektricky'] / 
             weekend_results['Celkovy pocet elektricky'] * 100)
        ]
        bars = ax2.bar(x, delayed_pct, width, color=['#e74c3c', '#f39c12'], alpha=0.8, edgecolor='black')
        ax2.set_ylabel('Percento (%)', fontsize=12)
        ax2.set_title('Meškajúce električky', fontsize=12, fontweight='bold')
        ax2.set_xticks(x)
        ax2.set_xticklabels(categories)
        ax2.grid(axis='y', alpha=0.3)
        for bar in bars:
            height = bar.get_height()
            ax2.text(bar.get_x() + bar.get_width()/2., height,
                    f'{height:.1f}%',
                    ha='center', va='bottom', fontsize=10)
    
    # Presmerované električky (%)
    if workday_results and weekend_results:
        rerouted_pct = [
            (workday_results['Presmerovane elektricky'] / 
             workday_results['Celkovy pocet elektricky'] * 100),
            (weekend_results['Presmerovane elektricky'] / 
             weekend_results['Celkovy pocet elektricky'] * 100)
        ]
        bars = ax3.bar(x, rerouted_pct, width, color=['#f39c12', '#2ecc71'], alpha=0.8, edgecolor='black')
        ax3.set_ylabel('Percento (%)', fontsize=12)
        ax3.set_title('Presmerované električky', fontsize=12, fontweight='bold')
        ax3.set_xticks(x)
        ax3.set_xticklabels(categories)
        ax3.grid(axis='y', alpha=0.3)
        for bar in bars:
            height = bar.get_height()
            ax3.text(bar.get_x() + bar.get_width()/2., height,
                    f'{height:.1f}%',
                    ha='center', va='bottom', fontsize=10)
    
    # Priemerný čas čakania
    if workday_results and weekend_results:
        wait_times = [workday_results['Priemerna doba cakania'],
                     weekend_results['Priemerna doba cakania']]
        bars = ax4.bar(x, wait_times, width, color=['#3498db', '#9b59b6'], alpha=0.8, edgecolor='black')
        ax4.set_ylabel('Čas (minúty)', fontsize=12)
        ax4.set_title('Priemerná doba čakania', fontsize=12, fontweight='bold')
        ax4.set_xticks(x)
        ax4.set_xticklabels(categories)
        ax4.grid(axis='y', alpha=0.3)
        for bar in bars:
            height = bar.get_height()
            ax4.text(bar.get_x() + bar.get_width()/2., height,
                    f'{height:.2f}',
                    ha='center', va='bottom', fontsize=10)
    
    plt.suptitle('Porovnanie pracovného dňa a víkendu', fontsize=16, fontweight='bold', y=1.00)
    plt.tight_layout()
    plt.savefig('graf_porovnanie.png', dpi=300, bbox_inches='tight')
    print("Graf uložený: graf_porovnanie.png")
    plt.close()

def plot_failures(workday_results, weekend_results):
    """Graf porúch"""
    fig, ax = plt.subplots(figsize=(10, 6))
    
    categories = ['Pracovný deň', 'Víkend']
    tunnel_failures = []
    tram_failures = []
    
    if workday_results:
        tunnel_failures.append(workday_results['Poruchy tunela'])
        tram_failures.append(workday_results['Poruchy elektricky'])
    else:
        tunnel_failures.append(0)
        tram_failures.append(0)
    
    if weekend_results:
        tunnel_failures.append(weekend_results['Poruchy tunela'])
        tram_failures.append(weekend_results['Poruchy elektricky'])
    else:
        tunnel_failures.append(0)
        tram_failures.append(0)
    
    x = np.arange(len(categories))
    width = 0.35
    
    bars1 = ax.bar(x - width/2, tunnel_failures, width, label='Poruchy tunela',
                   color='#e74c3c', alpha=0.8, edgecolor='black')
    bars2 = ax.bar(x + width/2, tram_failures, width, label='Poruchy električiek',
                   color='#f39c12', alpha=0.8, edgecolor='black')
    
    ax.set_ylabel('Počet porúch', fontsize=12)
    ax.set_title('Počet porúch za deň', fontsize=14, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(categories)
    ax.legend(fontsize=11)
    ax.grid(axis='y', alpha=0.3)
    
    # Hodnoty na stĺpcoch
    for bars in [bars1, bars2]:
        for bar in bars:
            height = bar.get_height()
            ax.text(bar.get_x() + bar.get_width()/2., height,
                   f'{int(height)}',
                   ha='center', va='bottom', fontsize=10)
    
    plt.tight_layout()
    plt.savefig('graf_poruchy.png', dpi=300, bbox_inches='tight')
    print("Graf uložený: graf_poruchy.png")
    plt.close()

def create_summary_table(workday_results, weekend_results):
    """Vytvorenie súhrnnej tabuľky"""
    fig, ax = plt.subplots(figsize=(12, 8))
    ax.axis('tight')
    ax.axis('off')
    
    # Pripravenie dát
    metrics = [
        'Celkový počet električiek',
        'Meškajúce električky',
        'Presmerované električky',
        'Poruchy tunela',
        'Poruchy električiek',
        'Priemerná doba čakania (min)',
        'Max doba čakania (min)',
        'Priemerná doba otáčania (min)',
        'Využitie koľaje A (%)',
        'Využitie koľaje B (%)',
        'Využitie koľaje C (%)',
        'Využitie koľaje D (%)'
    ]
    
    keys = [
        'Celkovy pocet elektricky',
        'Meskajuce elektricky',
        'Presmerovane elektricky',
        'Poruchy tunela',
        'Poruchy elektricky',
        'Priemerna doba cakania',
        'Max doba cakania',
        'Priemerna doba otacania',
        'Vyuzitie kolaje A',
        'Vyuzitie kolaje B',
        'Vyuzitie kolaje C',
        'Vyuzitie kolaje D'
    ]
    
    table_data = []
    for metric, key in zip(metrics, keys):
        wd_val = workday_results.get(key, '-') if workday_results else '-'
        we_val = weekend_results.get(key, '-') if weekend_results else '-'
        
        # Formátovanie hodnôt
        if isinstance(wd_val, (int, float)):
            if 'Využitie' in metric:
                wd_val = f"{wd_val:.1f}%"
            elif 'doba' in metric:
                wd_val = f"{wd_val:.2f}"
            else:
                wd_val = f"{int(wd_val)}"
        
        if isinstance(we_val, (int, float)):
            if 'Využitie' in metric:
                we_val = f"{we_val:.1f}%"
            elif 'doba' in metric:
                we_val = f"{we_val:.2f}"
            else:
                we_val = f"{int(we_val)}"
        
        table_data.append([metric, wd_val, we_val])
    
    table = ax.table(cellText=table_data,
                    colLabels=['Metrika', 'Pracovný deň', 'Víkend'],
                    cellLoc='left',
                    loc='center',
                    colWidths=[0.5, 0.25, 0.25])
    
    table.auto_set_font_size(False)
    table.set_fontsize(10)
    table.scale(1, 2)
    
    # Štýlovanie hlavičky
    for i in range(3):
        table[(0, i)].set_facecolor('#3498db')
        table[(0, i)].set_text_props(weight='bold', color='white')
    
    # Striedavé farby riadkov
    for i in range(1, len(table_data) + 1):
        if i % 2 == 0:
            for j in range(3):
                table[(i, j)].set_facecolor('#ecf0f1')
    
    plt.title('Súhrnná tabuľka výsledkov', fontsize=16, fontweight='bold', pad=20)
    plt.savefig('tabulka_vysledkov.png', dpi=300, bbox_inches='tight')
    print("Tabuľka uložená: tabulka_vysledkov.png")
    plt.close()

def main():
    print("="*60)
    print("  VIZUALIZÁCIA VÝSLEDKOV SIMULÁCIE")
    print("  Konečná stanica Kampus Bohunice")
    print("="*60)
    print()
    
    # Načítanie výsledkov
    workday_results = load_results('vysledky_pracovny_den.csv')
    weekend_results = load_results('vysledky_vikend.csv')
    
    if not workday_results and not weekend_results:
        print("CHYBA: Nenašli sa žiadne výsledkové súbory!")
        print("Najprv spustite simuláciu: make run")
        return 1
    
    # Generovanie grafov
    print("\nGenerujem grafy...")
    
    if workday_results and weekend_results:
        plot_track_utilization(workday_results, weekend_results)
        plot_comparison(workday_results, weekend_results)
        plot_failures(workday_results, weekend_results)
        create_summary_table(workday_results, weekend_results)
    else:
        print("UPOZORNENIE: Chýbajú niektoré výsledky, grafy môžu byť neúplné")
        if workday_results:
            print("Vizualizujem len pracovný deň...")
        if weekend_results:
            print("Vizualizujem len víkend...")
    
    print()
    print("="*60)
    print("Vizualizácia dokončená!")
    print("Vygenerované súbory:")
    print("  - graf_vyuzitie_kolaji.png")
    print("  - graf_porovnanie.png")
    print("  - graf_poruchy.png")
    print("  - tabulka_vysledkov.png")
    print("="*60)
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
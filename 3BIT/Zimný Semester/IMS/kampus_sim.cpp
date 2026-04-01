/*******************************************************************************
*                                                                              *
*                        Brno University of Technology                         *
*                      Faculty of Information Technology                       *
*                                                                              *
*                               Modely a Simulacie                             *
*                                                                              *
*            Author: Hugo Bohacsek [xbohach00 AT stud.fit.vutbr.cz]            *
*            Author: Adam Sloboda [xsloboa00 AT stud.fit.vutbr.cz]             *
*                                   Brno 2025                                  *
*                                                                              *
*             Implementation of the DPMB Campus terminus simulation            *
*                                                                              *
*******************************************************************************/
#include <simlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <cmath>
#include <iomanip>
#include <algorithm>

using namespace std;

// Konfiguracne parametre simulacie
const double TURNAROUND_TIME = 3.5;  // Cas na otocenie elektricky (minuty)
const double PEAK_DELAY_MAX = 5.0;   // Max. meskanie v spicke (minuty)
const double OFFPEAK_DELAY_MAX = 2.0; // Max. meskanie mimo spicky (minuty)
const double TUNNEL_BREAKDOWN_INTERVAL = 360.0; // Interval poruch tunela (minuty)
const double TUNNEL_BREAKDOWN_DURATION = 6.0;   // Trvanie poruchy tunela (minuty)
const double TRAM_BREAKDOWN_INTERVAL = 480.0;   // Interval poruch elektricky (minuty)
const double TRAM_BREAKDOWN_DURATION = 30.0;    // Trvanie poruchy elektricky (minuty)

// Dopravna spicka (7-9h a 15-18h)
bool isPeakHour(double time) {
    int hour = ((int)(time / 60)) % 24;
    return (hour >= 7 && hour < 9) || (hour >= 15 && hour < 18);
}

// Facilities pre jednotlive kolaje
Facility trackA("Kolaj A");
Facility trackB("Kolaj B");
Facility trackC("Kolaj C");
Facility trackD("Kolaj D");

// Facilities pre obmedzeny pristup k C a D
Facility accessToC("Pristup k C cez A");
Facility accessToD("Pristup k D cez B");

// Statistiky
Stat tramWaitTime("Cas cakania elektricky");
Stat tramTurnaroundTime("Cas otacania elektricky");
Histogram waitTimeHist("Histogram cakacich casov", 0, 2, 10);

// TStat pre sledovanie vyuzitia kolaji v case
TStat trackAUtil("Vyuzitie kolaje A");
TStat trackBUtil("Vyuzitie kolaje B");
TStat trackCUtil("Vyuzitie kolaje C");
TStat trackDUtil("Vyuzitie kolaje D");

// Globalne pocitadla
unsigned long totalTrams = 0;
unsigned long delayedTrams = 0;
unsigned long reroutedTrams = 0;
unsigned long tunnelBreakdowns = 0;
unsigned long tramBreakdowns = 0;

// Stav poruchy tunela
bool tunnelBroken = false;

// Databaza prichodov elektriciek (cas v minutach od polnoci, pozadovana kolaj)
struct TramScheduleEntry {
    int hour;
    int minute;
    char track;
    bool isWeekend;
};

vector<TramScheduleEntry> tramSchedule;

/**
 * @brief Nacitanie harmonogramu zo suboru
 */
void loadSchedule() {
    // Pracovne dni - huste intervaly simulujuce realnu prevadzku
    
    // Skore rano (4-6h) - zaciatok prevadzky, interval 10 min
    for (int h = 4; h < 6; h++) {
        for (int m = 0; m < 60; m += 10) {
            char tracks[] = {'A', 'B'};
            tramSchedule.push_back({h, m, tracks[(h * 6 + m / 10) % 2], false});
        }
    }
    
    // Ranna spicka (6-9h) - interval 3 minuty, vsetky kolaje
    for (int h = 6; h < 9; h++) {
        for (int m = 0; m < 60; m += 3) {
            // Vacsina na A a B, menej na C a D
            int idx = (h * 20 + m / 3) % 8;
            char track = (idx < 3) ? 'A' : (idx < 6) ? 'B' : (idx < 7) ? 'C' : 'D';
            tramSchedule.push_back({h, m, track, false});
        }
    }
    
    // Dopoludnie (9-14h) - interval 6 minut
    for (int h = 9; h < 14; h++) {
        for (int m = 0; m < 60; m += 6) {
            int idx = (h * 10 + m / 6) % 6;
            char track = (idx < 2) ? 'A' : (idx < 4) ? 'B' : (idx < 5) ? 'C' : 'D';
            tramSchedule.push_back({h, m, track, false});
        }
    }
    
    // Popoludnajsia spicka (14-18h) - interval 3 minuty, husta prevadzka
    for (int h = 14; h < 18; h++) {
        for (int m = 0; m < 60; m += 3) {
            int idx = (h * 20 + m / 3) % 8;
            char track = (idx < 3) ? 'A' : (idx < 6) ? 'B' : (idx < 7) ? 'C' : 'D';
            tramSchedule.push_back({h, m, track, false});
        }
    }
    
    // Vecer (18-22h) - interval 8 minut
    for (int h = 18; h < 22; h++) {
        for (int m = 0; m < 60; m += 8) {
            int idx = (h * 7 + m / 8) % 5;
            char track = (idx < 2) ? 'A' : (idx < 3) ? 'B' : (idx < 4) ? 'C' : 'D';
            tramSchedule.push_back({h, m, track, false});
        }
    }
    
    // Noc (22-24h) - interval 12 minut
    for (int h = 22; h < 24; h++) {
        for (int m = 0; m < 60; m += 12) {
            char tracks[] = {'A', 'B'};
            tramSchedule.push_back({h, m, tracks[(h + m / 12) % 2], true});
        }
    }
    
    // Vikend - riedkejsi harmonogram
    
    // Rano (6-10h) - interval 10 minut, iba A a B
    for (int h = 6; h < 10; h++) {
        for (int m = 0; m < 60; m += 10) {
            char tracks[] = {'A', 'B'};
            tramSchedule.push_back({h, m, tracks[(h * 6 + m / 10) % 2], true});
        }
    }
    
    // Den (10-18h) - interval 8 minut
    for (int h = 10; h < 18; h++) {
        for (int m = 0; m < 60; m += 8) {
            char tracks[] = {'A', 'B', 'C', 'D'};
            int idx = (h * 7 + m / 8) % 4;
            tramSchedule.push_back({h, m, tracks[idx], true});
        }
    }
    
    // Vecer (18-22h) - interval 12 minut
    for (int h = 18; h < 22; h++) {
        for (int m = 0; m < 60; m += 12) {
            char tracks[] = {'A', 'B'};
            tramSchedule.push_back({h, m, tracks[(h + m / 12) % 2], true});
        }
    }
    
    // Usporiadaj harmonogram podla casu
    sort(tramSchedule.begin(), tramSchedule.end(), 
         [](const TramScheduleEntry& a, const TramScheduleEntry& b) {
             int timeA = a.hour * 60 + a.minute;
             int timeB = b.hour * 60 + b.minute;
             if (timeA != timeB) return timeA < timeB;
             return a.isWeekend < b.isWeekend;
         });
}

/**
 * @brief Proces periodickeho vzorkovania pre TStat
 */
class UtilizationSampler : public Event {
    void Behavior() {
        // Zaznamenaj aktualny stav kolaji
        trackAUtil(trackA.Busy() ? 1.0 : 0.0);
        trackBUtil(trackB.Busy() ? 1.0 : 0.0);
        trackCUtil(trackC.Busy() ? 1.0 : 0.0);
        trackDUtil(trackD.Busy() ? 1.0 : 0.0);
        
        // Planuj dalsie vzorkovanie za 0.1 minuty
        Activate(Time + 0.1);
    }
};

/**
 * @brief Proces poruchy tunela
 */
class TunnelBreakdown : public Process {
    void Behavior() {
        while (true) {
            Wait(Exponential(TUNNEL_BREAKDOWN_INTERVAL));
            tunnelBroken = true;
            tunnelBreakdowns++;
            cout << "TUNEL PORUCHA v case " << Time << " min" << endl;
            Wait(TUNNEL_BREAKDOWN_DURATION);
            tunnelBroken = false;
            cout << "TUNEL OPRAVENY v case " << Time << " min" << endl;
        }
    }
};

/**
 * @brief Proces prichodu a obsluhy elektricky
 */
class TramArrival : public Process {
private:
    char requestedTrack;
    double arrivalTime;
    
public:
    TramArrival(char track, double time) : requestedTrack(track), arrivalTime(time) {}
    
    void Behavior() {
        totalTrams++;
        double waitStart = Time;
        
        // Aplikovanie meskania
        double delay = 0;
        if (isPeakHour(Time)) {
            delay = Uniform(0, PEAK_DELAY_MAX);
        } else {
            delay = Uniform(0, OFFPEAK_DELAY_MAX);
        }
        
        if (delay > 1.0) {
            delayedTrams++;
        }
        
        Wait(delay);
        
        // Kontrola poruchy tunela
        if (tunnelBroken) {
            Wait(TUNNEL_BREAKDOWN_DURATION);
        }
        
        // Nahodna porucha elektricky
        if (Random() < 0.01) {  // 1% sanca na poruchu
            tramBreakdowns++;
            cout << "ELEKTRICKA PORUCHA v case " << Time << " min" << endl;
            Wait(TRAM_BREAKDOWN_DURATION);
        }
        
        // Pokus o obsadenie pozadovanej kolaje
        bool success = false;
        Facility* selectedTrack = nullptr;
        
        switch (requestedTrack) {
            case 'A':
                if (!trackA.Busy()) {
                    Seize(trackA);
                    selectedTrack = &trackA;
                    success = true;
                }
                break;
                
            case 'B':
                if (!trackB.Busy()) {
                    Seize(trackB);
                    selectedTrack = &trackB;
                    success = true;
                }
                break;
                
            case 'C':
                // C vyzaduje volne A
                if (!trackC.Busy() && !trackA.Busy()) {
                    Seize(accessToC);
                    Seize(trackC);
                    selectedTrack = &trackC;
                    success = true;
                }
                break;
                
            case 'D':
                // D vyzaduje volne B
                if (!trackD.Busy() && !trackB.Busy()) {
                    Seize(accessToD);
                    Seize(trackD);
                    selectedTrack = &trackD;
                    success = true;
                }
                break;
        }
        
        // Ak neuspech, hladaj alternativu
        if (!success) {
            reroutedTrams++;
            
            if (!trackA.Busy()) {
                Seize(trackA);
                selectedTrack = &trackA;
            } else if (!trackB.Busy()) {
                Seize(trackB);
                selectedTrack = &trackB;
            } else if (!trackC.Busy() && !trackA.Busy()) {
                Seize(accessToC);
                Seize(trackC);
                selectedTrack = &trackC;
            } else if (!trackD.Busy() && !trackB.Busy()) {
                Seize(accessToD);
                Seize(trackD);
                selectedTrack = &trackD;
            } else {
                // Cakaj na prvu volnu kolaj
                Seize(trackA);
                selectedTrack = &trackA;
            }
        }
        
        double waitTime = Time - waitStart - delay;
        if (waitTime < 0) waitTime = 0.0;
        tramWaitTime(waitTime);
        waitTimeHist(waitTime);
        
        // Otocenie elektricky
        double turnaround = Normal(TURNAROUND_TIME, 0.3);
        if (turnaround < 1.0) turnaround = 1.0;
        Wait(turnaround);
        tramTurnaroundTime(turnaround);
        
        // Uvolnenie kolaje
        if (selectedTrack == &trackC) {
            Release(trackC);
            Release(accessToC);
        } else if (selectedTrack == &trackD) {
            Release(trackD);
            Release(accessToD);
        } else {
            Release(*selectedTrack);
        }
    }
};

/**
 * @brief Generator elektriciek podla harmonogramu
 */
class TramGenerator : public Event {
private:
    bool useWeekend;
    
public:
    TramGenerator(bool weekend = false) : useWeekend(weekend) {}
    
    void Behavior() {
        // Prejdi vsetky zaznamy v harmonograme
        for (const auto& entry : tramSchedule) {
            // Filtruj podla typu dna
            if (entry.isWeekend != useWeekend) {
                continue;
            }
            
            // Vypocitaj cas prichodu v minutach od zaciatku simulacie
            double arrivalTime = entry.hour * 60.0 + entry.minute;
            
            // Naplanuj prichod elektricky na spravny cas
            (new TramArrival(entry.track, arrivalTime))->Activate(arrivalTime);
        }
    }
};

/**
 * @brief Export vysledkov do CSV
 */
void exportResults(const string& filename) {
    ofstream file(filename);
    
    file << "Metrika,Hodnota\n";
    file << "Celkovy pocet elektricky," << totalTrams << "\n";
    file << "Meskajuce elektricky," << delayedTrams << "\n";
    file << "Presmerovane elektricky," << reroutedTrams << "\n";
    file << "Poruchy tunela," << tunnelBreakdowns << "\n";
    file << "Poruchy elektricky," << tramBreakdowns << "\n";
    
    // Casy cakania a otacania
    file << "Priemerna doba cakania," << fixed << setprecision(2) 
         << (tramWaitTime.Number() > 0 ? tramWaitTime.MeanValue() : 0.0) << "\n";
    file << "Max doba cakania," << fixed << setprecision(2)
         << (tramWaitTime.Number() > 0 ? tramWaitTime.Max() : 0.0) << "\n";
    file << "Priemerna doba otacania," << fixed << setprecision(2)
         << (tramTurnaroundTime.Number() > 0 ? tramTurnaroundTime.MeanValue() : 0.0) << "\n";
    
    // Vyuzitie kolaji (v percentach)
    file << "Vyuzitie kolaje A," << fixed << setprecision(1) 
         << (trackAUtil.Number() > 0 ? trackAUtil.MeanValue() * 100.0 : 0.0) << "%\n";
    file << "Vyuzitie kolaje B," << fixed << setprecision(1)
         << (trackBUtil.Number() > 0 ? trackBUtil.MeanValue() * 100.0 : 0.0) << "%\n";
    file << "Vyuzitie kolaje C," << fixed << setprecision(1)
         << (trackCUtil.Number() > 0 ? trackCUtil.MeanValue() * 100.0 : 0.0) << "%\n";
    file << "Vyuzitie kolaje D," << fixed << setprecision(1)
         << (trackDUtil.Number() > 0 ? trackDUtil.MeanValue() * 100.0 : 0.0) << "%\n";
    
    file.close();
    cout << "Vysledky exportovane do " << filename << endl;
}

/**
 * @brief Hlavna funkcia
 */
int main(int argc, char* argv[]) {
    bool isWeekend = false;
    int simulationDays = 1;
    
    // Parsovanie argumentov
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--weekend" || arg == "-w") {
            isWeekend = true;
        } else if (arg == "--days" || arg == "-d") {
            if (i + 1 < argc) {
                simulationDays = atoi(argv[++i]);
            }
        } else if (arg == "--help" || arg == "-h") {
            cout << "Pouzitie: " << argv[0] << " [MOZNOSTI]\n";
            cout << "Moznosti:\n";
            cout << "  -w, --weekend     Simuluj vikendovy harmonogram\n";
            cout << "  -d, --days N      Pocet dni simulacie (default: 1)\n";
            cout << "  -h, --help        Zobrazi tuto napovedu\n";
            return 0;
        }
    }
    
    // Inicializacia
    RandomSeed(time(NULL));
    loadSchedule();
    
    // Spocitaj kolko elektriciek je pre kazdy typ dna
    int workdayCount = 0;
    int weekendCount = 0;
    for (const auto& entry : tramSchedule) {
        if (entry.isWeekend) {
            weekendCount++;
        } else {
            workdayCount++;
        }
    }
    
    cout << "==============================================\n";
    cout << "  SIMULACIA KONECNEJ KAMPUS BOHUNICE\n";
    cout << "==============================================\n";
    cout << "Typ dna: " << (isWeekend ? "Vikend" : "Pracovny den") << "\n";
    cout << "Pocet dni: " << simulationDays << "\n";
    cout << "Elektricky v harmonograme:\n";
    cout << "  - Pracovne dni: " << workdayCount << "\n";
    cout << "  - Vikendy: " << weekendCount << "\n";
    cout << "Simulovane: " << (isWeekend ? weekendCount : workdayCount) << " elektricky\n";
    cout << "==============================================\n\n";
    
    // Inicializacia simulacie
    Init(0, simulationDays * 24 * 60);  // Simulacia N dni
    
    // Spustenie procesov
    (new TunnelBreakdown)->Activate();
    (new TramGenerator(isWeekend))->Activate();
    (new UtilizationSampler)->Activate();  // Spusti vzorkovanie vyuzitia
    
    // Spustenie simulacie
    Run();
    
    // Vystup vysledkov
    cout << "\n==============================================\n";
    cout << "  VYSLEDKY SIMULACIE\n";
    cout << "==============================================\n\n";
    
    trackA.Output();
    trackB.Output();
    trackC.Output();
    trackD.Output();
    
    cout << "\n--- STATISTIKY VYUZITIA (TStat) ---\n";
    trackAUtil.Output();
    trackBUtil.Output();
    trackCUtil.Output();
    trackDUtil.Output();
    
    cout << "\n--- STATISTIKY CASOV ---\n";
    tramWaitTime.Output();
    tramTurnaroundTime.Output();
    waitTimeHist.Output();
    
    cout << "\n--- SUHRN ---\n";
    cout << "Celkovy pocet elektricky: " << totalTrams << "\n";
    cout << "Meskajuce elektricky (>1min): " << delayedTrams 
         << " (" << fixed << setprecision(1) << (100.0 * delayedTrams / totalTrams) << "%)\n";
    cout << "Presmerovane elektricky: " << reroutedTrams 
         << " (" << fixed << setprecision(1) << (100.0 * reroutedTrams / totalTrams) << "%)\n";
    cout << "Poruchy tunela: " << tunnelBreakdowns << "\n";
    cout << "Poruchy elektricky: " << tramBreakdowns << "\n";
    cout << "\nPriemerne vyuzitie kolaji:\n";
    cout << "  Kolaj A: " << fixed << setprecision(1) << (trackAUtil.MeanValue() * 100.0) << "%\n";
    cout << "  Kolaj B: " << fixed << setprecision(1) << (trackBUtil.MeanValue() * 100.0) << "%\n";
    cout << "  Kolaj C: " << fixed << setprecision(1) << (trackCUtil.MeanValue() * 100.0) << "%\n";
    cout << "  Kolaj D: " << fixed << setprecision(1) << (trackDUtil.MeanValue() * 100.0) << "%\n";
    
    // Export do CSV
    string csvFilename = isWeekend ? "vysledky_vikend.csv" : "vysledky_pracovny_den.csv";
    exportResults(csvFilename);
    
    return 0;
}
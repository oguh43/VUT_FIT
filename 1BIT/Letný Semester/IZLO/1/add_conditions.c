#include <stddef.h>
#include "cnf.h"

//
// LOGIN: XBOHACH00
//

// Tato funkce by mela do formule pridat klauzule predstavujici podminku 1)
// Křižovatky jsou reprezentovany cisly 0, 1, ..., num_of_crossroads-1
// Cislo num_of_streets predstavuje pocet ulic a proto i pocet kroku cesty
// Pole streets ma velikost num_of_streets a obsahuje vsechny existujuci ulice
//    - pro 0 <= i < num_of_streets predstavuje streets[i] jednu existujici
//      ulici od krizovatky streets[i].crossroad_from ke krizovatce streets[i].crossroad_to
void at_least_one_valid_street_for_each_step(CNF* formula, unsigned num_of_crossroads, unsigned num_of_streets, const Street* streets) {
	assert(formula != NULL);
	assert(num_of_crossroads > 0);
	assert(num_of_streets > 0);
	assert(streets != NULL);
	
	
	
	
	Clause *c;// = create_new_clause(formula);
	for (int step = 0; step < num_of_streets; step++){
		c= create_new_clause(formula);
		for (int i = 0; i < num_of_streets;i++){
			add_literal_to_clause(c, true, step, streets[i].crossroad_from, streets[i].crossroad_to);
		}
		
		continue;
		Clause *c = create_new_clause(formula);
		for (int z = 0; z < num_of_crossroads; z++){
			for (int k = 0; k < num_of_crossroads; k++){
				if (streets[step].crossroad_from == z && streets[step].crossroad_to == k){
					add_literal_to_clause(c, true, step, z, k);
				}
			}
		}
	}
	/*../tests/sat/complex.in: Nesprávný model: neexistující ulice 0 -> 5 v kroku 0
../tests/sat/loop.in: Nesprávný model: neexistující ulice 0 -> 4 v kroku 0 +++
../tests/sat/multiple_paths.in: Nesprávný model: neexistující ulice 0 -> 4 v kroku 0
../tests/sat/one_edge.in: Nesprávný model: neexistující ulice 0 -> 0 v kroku 0 +++
../tests/sat/path_with_self_loops.in: Nesprávný model: neexistující ulice 0 -> 4 v kroku 0
../tests/sat/self_loop.in: Chybný výsledek (UNSAT, očekávaný SAT) ???
../tests/sat/simple_path.in: Nesprávný model: neexistující ulice 0 -> 3 v kroku 0 +++
../tests/unsat/diamond.in: Chybný výsledek (SAT, očekávaný UNSAT)
../tests/unsat/loop_in_middle.in: Chybný výsledek (SAT, očekávaný UNSAT)
../tests/unsat/many_random.in: Chybný výsledek (SAT, očekávaný UNSAT)
../tests/unsat/same_dir.in: Chybný výsledek (SAT, očekávaný UNSAT)
../tests/unsat/simple_path_disconnected.in: Chybný výsledek (SAT, očekávaný UNSAT)
../tests/unsat/two_disconnected.in: Chybný výsledek (SAT, očekávaný UNSAT)
*/
}

// Tato funkce by mela do formule pridat klauzule predstavujici podminku 2)
// Křižovatky jsou reprezentovany cisly 0, 1, ..., num_of_crossroads-1
// Cislo num_of_streets predstavuje pocet ulic a proto i pocet kroku cesty
void at_most_one_street_for_each_step(CNF* formula, unsigned num_of_crossroads, unsigned num_of_streets) {
	assert(formula != NULL);
	assert(num_of_crossroads > 0);
	assert(num_of_streets > 0);
	//return;
	Clause *c;
	for(int i = 0; i < num_of_streets; i++){
		for (int z = 0; z < num_of_crossroads; z++){
			for (int k = 0; k < num_of_crossroads; k++){
				for (int zz = 0; zz < num_of_crossroads; zz++){
					for (int kk = 0; kk < num_of_crossroads; kk++){
						if (!(z==zz && k==kk)){
							c = create_new_clause(formula);
							add_literal_to_clause(c, false, i, z, k);
							add_literal_to_clause(c, false, i, zz, kk);
						}
						
					}
					
				}
			}
		}
	}
/*../tests/sat/complex.in: Nesprávný model: ulice 0 -> 3 v kroku 1 nenavazuje na ulici 0 -> 1 z kroku 0.
../tests/sat/loop.in: OK
../tests/sat/multiple_paths.in: Nesprávný model: ulice 3 -> 4 v kroku 3 nenavazuje na ulici 2 -> 1 z kroku 2.
../tests/sat/one_edge.in: OK
../tests/sat/path_with_self_loops.in: Nesprávný model: ulice 0 -> 0 v kroku 4 nenavazuje na ulici 3 -> 4 z kroku 3.
../tests/sat/self_loop.in: OK
../tests/sat/simple_path.in: OK
../tests/unsat/diamond.in: Chybný výsledek (SAT, očekávaný UNSAT)
../tests/unsat/loop_in_middle.in: Chybný výsledek (SAT, očekávaný UNSAT)
../tests/unsat/many_random.in: Chybný výsledek (SAT, očekávaný UNSAT)
../tests/unsat/same_dir.in: Chybný výsledek (SAT, očekávaný UNSAT)
../tests/unsat/simple_path_disconnected.in: Chybný výsledek (SAT, očekávaný UNSAT)
../tests/unsat/two_disconnected.in: Chybný výsledek (SAT, očekávaný UNSAT)
*/
	
	
	
	// ZDE PRIDAT KOD
}

// Tato funkce by mela do formule pridat klauzule predstavujici podminku 3)
// Křižovatky jsou reprezentovany cisly 0, 1, ..., num_of_crossroads-1
// Cislo num_of_streets predstavuje pocet ulic a proto i pocet kroku cesty
void streets_connected(CNF* formula, unsigned num_of_crossroads, unsigned num_of_streets) {
	assert(formula != NULL);
	assert(num_of_crossroads > 0);
	assert(num_of_streets > 0);
	//return;
	Clause *c;
	for (int i = 0; i < num_of_streets; i++){
		for (int z = 0; z < num_of_crossroads; z++){
			for (int k = 0; k < num_of_crossroads; k++){
				/*c = create_new_clause(formula);
				add_literal_to_clause(c, false, i, z, k);
				add_literal_to_clause(c, true, i+1, k, 0);
				c = create_new_clause(formula);
				add_literal_to_clause(c, false, i, z, k);
				add_literal_to_clause(c, true, i+1, k, 1);
				c = create_new_clause(formula);
				add_literal_to_clause(c, false, i, z, k);
				add_literal_to_clause(c, true, i+1, k, 2);*/
				c = create_new_clause(formula);
				add_literal_to_clause(c,false,i,z,k);
				for (int kk=0; kk<num_of_crossroads;kk++){
					add_literal_to_clause(c,true,i+1,k,kk);
				}
				
			}
		}
	}
}

// Tato funkce by mela do formule pridat klauzule predstavujici podminku 4)
// Křižovatky jsou reprezentovany cisly 0, 1, ..., num_of_crossroads-1
// Cislo num_of_streets predstavuje pocet ulic a proto i pocet kroku cesty
void streets_do_not_repeat(CNF* formula, unsigned num_of_crossroads, unsigned num_of_streets) {
	assert(formula != NULL);
	assert(num_of_crossroads > 0);
	assert(num_of_streets > 0);
	
	for (unsigned i = 0; i < num_of_streets; ++i) {
		// pro kazdy krok i
		for (unsigned j = 0; j < num_of_streets; ++j) {
			if (i != j) {
				// pro kazdy jiny krok j
				for (unsigned z = 0; z < num_of_crossroads; ++z) {
					for (unsigned k = 0; k < num_of_crossroads; ++k) {
						// pro kazdu dvojici krizovatek (z, k)
						Clause* cl = create_new_clause(formula);
						add_literal_to_clause(cl, false, i, z, k);
						add_literal_to_clause(cl, false, j, z, k);
					}
				}
			}
		}
	}
}
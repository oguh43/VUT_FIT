//======= Copyright (c) 2024, FIT VUT Brno, All rights reserved. ============//
//
// Purpose:     White Box - test suite
//
// $NoKeywords: $ivs_project_1 $white_box_tests.cpp
// $Author:     HUGO BOHÁCSEK <xbohach00@stud.fit.vutbr.cz>
// $Date:       $2024-02-14
//============================================================================//
/**
 * @file white_box_tests.cpp
 * @author HUGO BOHÁCSEK
 * 
 * @brief Implementace testu hasovaci tabulky.
 */

#include <vector>

#include "gtest/gtest.h"

#include "white_box_code.h"

//============================================================================//
// ** ZDE DOPLNTE TESTY **
//
// Zde doplnte testy hasovaci tabulky, testujte nasledujici:
// 1. Verejne rozhrani hasovaci tabulky
//     - Vsechny funkce z white_box_code.h
//     - Chovani techto metod testuje pro prazdnou i neprazdnou tabulku.
// 2. Chovani tabulky v hranicnich pripadech
//     - Otestujte chovani pri kolizich ruznych klicu se stejnym hashem 
//     - Otestujte chovani pri kolizich hashu namapovane na stejne misto v 
//       indexu
//============================================================================//
class hashMapTest : public ::testing::Test{
	protected:
		hash_map_t *map;
		
		virtual void SetUp(){
			map = hash_map_ctor();
		}
		virtual void TearDown(){
			hash_map_dtor(map);
		}
};

//https://stackoverflow.com/questions/38023749/make-malloc-fail-voluntarily-in-order-to-test-the-cases-when-malloc-fails
	/*TEST_F(hashMapTest,sample){
		EXPECT_NE(1,5);
	}*/

TEST_F(hashMapTest, reservation){
	EXPECT_EQ(hash_map_reserve(map, -1), MEMORY_ERROR);
	EXPECT_EQ(hash_map_reserve(map, map->allocated), OK);
	EXPECT_EQ(hash_map_reserve(map, 25), OK);
	EXPECT_EQ(hash_map_reserve(map, SIZE_MAX), MEMORY_ERROR);
	EXPECT_EQ(hash_map_put(map, "abc", 1), OK);
	EXPECT_EQ(hash_map_reserve(map, 30), OK);
	EXPECT_EQ(hash_map_reserve(map, 0), VALUE_ERROR);
}

TEST_F(hashMapTest, size_capacity_contains){
	EXPECT_EQ(hash_map_size(map), 0);
	EXPECT_EQ(hash_map_capacity(map), 8);
	EXPECT_EQ(hash_map_contains(map, "abc"), false);
	EXPECT_EQ(hash_map_put(map, "abc", 1), OK);
	EXPECT_EQ(hash_map_contains(map, "abc"), true);
}

TEST_F(hashMapTest, put){
	for (int i = 0; i < 1000; i++){
		EXPECT_EQ(hash_map_put(map, std::to_string(i).c_str(), i+55), OK);
	}
	EXPECT_EQ(hash_map_put(map, "1", 56), KEY_ALREADY_EXISTS);
}

TEST_F(hashMapTest, get){
	int dest = 0;
	EXPECT_EQ(hash_map_get(map, "55", &dest), KEY_ERROR);
	EXPECT_EQ(dest, 0);
	EXPECT_EQ(hash_map_put(map, "55", 42), OK);
	EXPECT_EQ(hash_map_get(map, "55", &dest), OK);
	EXPECT_EQ(dest, 42);
}

TEST_F(hashMapTest, remove){
	for (int i = 0; i < 1000; i++){
		EXPECT_EQ(hash_map_put(map, std::to_string(i).c_str(), i+55), OK);
	}
	hash_map_remove(map, "0");
	hash_map_remove(map, "999");
	hash_map_remove(map, "132");
	hash_map_remove(map, "abc");
	
	hash_map_clear(map);
	hash_map_put(map, "55", 55);
	hash_map_remove(map, "55");
	
	hash_map_clear(map);
	hash_map_put(map, "55", 55);
	hash_map_put(map, "56", 56);
	hash_map_put(map, "57", 57);
	hash_map_remove(map, "56");
}

TEST_F(hashMapTest, same_hash_collision){
	EXPECT_EQ(hash_map_put(map, "abc", 66), OK);
	EXPECT_EQ(hash_map_put(map, "cba", 66), OK);
}

TEST_F(hashMapTest, index_hash_collision){
	EXPECT_EQ(hash_map_put(map, "abc", 66), OK);
	EXPECT_EQ(hash_map_put(map, "cba", 67), OK);
	EXPECT_EQ(hash_map_put(map, "bca", 68), OK);
	int res = 0;
	EXPECT_EQ(hash_map_get(map, "cba", &res), OK);
	EXPECT_EQ(res, 67);
}



/*** Konec souboru white_box_tests.cpp ***/

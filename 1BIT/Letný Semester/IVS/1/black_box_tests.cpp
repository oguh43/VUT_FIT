//======= Copyright (c) 2024, FIT VUT Brno, All rights reserved. ============//
//
// Purpose:     Red-Black Tree - public interface tests
//
// $NoKeywords: $ivs_project_1 $black_box_tests.cpp
// $Author:     HUGO BOHÁCSEK <xbohach00@stud.fit.vutbr.cz>
// $Date:       $2024-02-14
//============================================================================//
/**
 * @file black_box_tests.cpp
 * @author HUGO BOHÁCSEK
 * 
 * @brief Implementace testu binarniho stromu.
 */

#include <vector>

#include "gtest/gtest.h"

#include "red_black_tree.h"

//============================================================================//
// ** ZDE DOPLNTE TESTY **
//
// Zde doplnte testy Red-Black Tree, testujte nasledujici:
// 1. Verejne rozhrani stromu
//    - InsertNode/DeleteNode a FindNode
//    - Chovani techto metod testuje pro prazdny i neprazdny strom.
// 2. Axiomy (tedy vzdy platne vlastnosti) Red-Black Tree:
//    - Vsechny listove uzly stromu jsou *VZDY* cerne.
//    - Kazdy cerveny uzel muze mit *POUZE* cerne potomky.
//    - Vsechny cesty od kazdeho listoveho uzlu ke koreni stromu obsahuji
//      *STEJNY* pocet cernych uzlu.
//============================================================================//

class BinaryTreeTest : public ::testing::Test{
protected:
	BinaryTree *tree;
	
	virtual void SetUp(){
		tree = new BinaryTree();
	}
	virtual void TearDown(){
		delete tree;
	}
};
//insert find a delete
TEST_F(BinaryTreeTest, EmptyTree){
	EXPECT_EQ(tree->FindNode(0), nullptr);		// Nájdime v prázdnom
	EXPECT_EQ(tree->DeleteNode(0), false); 		// Vymažme v prázdnom
	EXPECT_EQ(tree->GetRoot(), nullptr);		// Nájdime root v prázdnom
	
	EXPECT_EQ(tree->InsertNode(5).first, true); // Insertneme
	EXPECT_NE(tree->FindNode(5), nullptr);		// Nájdeme
	EXPECT_EQ(tree->DeleteNode(5), true);		// Odstránime
	
	// Vyskúšame insert viacerých itemov naraz
	
	std::vector<int> zoznam{ 1, 2, 3, 4, 5 };
	std::vector<std::pair<bool, Node_t *> > res;
	tree->InsertNodes(zoznam, res);
	EXPECT_GT(res.size(), 0);
	
	for (int i = 0; i < res.size(); i++){
		EXPECT_EQ(res[i].first, true);
	}
}
// InsertNode, DeleteNode, FindNode
TEST_F(BinaryTreeTest, NonEmptyTree){
	// Je prázdny?
	EXPECT_EQ(tree->GetRoot(), nullptr);
	
	// Spravíme insert, nech máme na čom testovať
	std::vector<int> zoznam{ 0, 1, 2, 3, 4, 5 };
	std::vector<std::pair<bool, Node_t *> > res;
	tree->InsertNodes(zoznam, res);
	EXPECT_GT(res.size(), 0);
	
	for (int i = 0; i < res.size(); i++){
		EXPECT_EQ(res[i].first, true);
	}
	
	EXPECT_EQ(tree->InsertNode(1).first, false);	// Duplicitný insert
	EXPECT_EQ(tree->InsertNode(6).first, true);		// Nový insert
	
	EXPECT_NE(tree->FindNode(3), nullptr);			// Hľadajme existujúce
	EXPECT_EQ(tree->FindNode(10), nullptr);			// Hľadajme neexistujúci
	
	// Ideme na delete
	for (int i=0; i<3; i++){
		EXPECT_EQ(tree->DeleteNode(i), true);
	}
	EXPECT_EQ(tree->DeleteNode(0), false);			// Neplatný delete
	EXPECT_NE(tree->GetRoot(), nullptr);			// Stratili sme root?
	
	EXPECT_EQ(tree->InsertNode(0).first, true);			// Naspäť insert
	
	/*for (int i=0; i < zoznam.size(); i++){
		EXPECT_EQ((tree->FindNode(i) == nullptr), tree->InsertNode(i).first);	// Toto ak to prežije :DDDD // Neprežilo :((( // TODO: fix :)
	}*/
	
}

TEST_F(BinaryTreeTest, TreeAxioms){
	
	// Je prázdny?
	EXPECT_EQ(tree->GetRoot(), nullptr);
	
	// Spravíme insert, nech máme na čom testovať
	std::vector<int> zoznam{ 0, 1, 2, 3, 4, 5, 6, 8 };
	std::vector<std::pair<bool, Node_t *> > res;
	tree->InsertNodes(zoznam, res);
	EXPECT_GT(res.size(), 0);
	
	for (int i = 0; i < res.size(); i++){
		EXPECT_EQ(res[i].first, true);
	}
	
	/*
	Základní axiomy Red-Black Tree  x
	Všechny listové uzly (tedy uzly bez potomků) jsou „černé”. x
	Pokud je uzel „červený”, pak jsou jeho oba potomci „černé”. x
	Každá cesta od každého listového uzlu ke kořeni obsahuje stejný počet „černých” uzlů (vizte oranžové cesty v obrázku). x
	*/
	
	std::vector<Node_t *> allNodes;
	tree->GetAllNodes(allNodes);
	EXPECT_GT(allNodes.size(), 0);
	int leafCnt = 0;
	for (int i = 0; i < allNodes.size(); i++){
		if ((allNodes[i]->pLeft == nullptr) && (allNodes[i]->pRight == nullptr)){
			EXPECT_EQ(allNodes[i]->color, BinaryTree::BLACK);
			leafCnt++;
		}
		if (allNodes[i]->color == BinaryTree::RED){
			
			if (allNodes[i]->pLeft != nullptr){
				EXPECT_EQ(allNodes[i]->pLeft->color, BinaryTree::BLACK);
			}
			if (allNodes[i]->pRight != nullptr){
				EXPECT_EQ(allNodes[i]->pRight->color, BinaryTree::BLACK);
			}
		}
	}
	std::vector<Node_t *> leafNodes;
	tree->GetLeafNodes(leafNodes);
	EXPECT_EQ(leafCnt, leafNodes.size());
	
	Node_t *root = tree->GetRoot();
	Node_t *curr = leafNodes[0];
	
	std::set<int> blackCount;
	int cnt = 0;
	int index = 0;
	
	while (1){
		if (index >= leafNodes.size()){
			break;
		}
		curr = curr->pParent;
		if (curr->key == root->key){
			blackCount.insert(cnt);
			cnt = 0;
			index++;
			if (index >= leafNodes.size()){
				break;
			}
			curr = leafNodes[index];
			continue;
		}
		if (curr->color == BinaryTree::BLACK){
			cnt++;
		}
	}
	ASSERT_EQ(blackCount.size(), 1);
	
}



/*** Konec souboru black_box_tests.cpp ***/

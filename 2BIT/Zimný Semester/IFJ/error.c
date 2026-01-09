/**
 * FIT VUT - IFJ project 2024
 *
 * @file error.c
 *
 * @brief Error exit handling
 *
 * @author Hugo Bohácsek (xbohach00)
 * @author Filip Jenis (xjenisf00)
 */
#include "error.h"
#include "generator.h"
#include <stdlib.h>

void error(int num){
    generator_dispose();
    exit(num);
}

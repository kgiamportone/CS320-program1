#include <vector>
#include <string.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <stdio.h>
#include <cstdlib>
#include <math.h>

using namespace std;

void single_bit_bimodal(char * input_file, FILE * output_file, unsigned long long size) {
    // 0 = not taken, 1 = taken
    unsigned long long table_size = pow(2, size); // size of prediction table
    printf("Single bit bimodal size %llu\n", table_size);
    // index = address AND size
    int prediction_table[table_size];
    for (int i = 0; i <(int)table_size; i++) {
        prediction_table[i] = 1;
    }

    FILE * input = fopen(input_file, "r");
    // Temporary variables
    unsigned long long addr;
    char behavior[10];
    unsigned long long target;

    int correct_total = 0;
    int total = 0;

    while(fscanf(input, "%llx %10s %llx\n", &addr, behavior, &target) != EOF) {
        unsigned long long index = addr & (table_size - 1); // get last size bits
        // printf("Index = %llu\n", index);
        if (strncmp(behavior, "NT", 2) == 0) { // not taken
            if (prediction_table[index] == 0) { // not taken (hit)
                correct_total++;
            }
            else { // expected taken (miss)
                prediction_table[index] = 0;
            }
        }
        else { // taken
            if (prediction_table[index] == 0) { // not taken (miss)
                prediction_table[index] = 1;
            }
            else { // expected taken (miss)
                correct_total++;
            }
        }
        total++;
    }
    fprintf(output_file, "%d,%d; ", correct_total, total);
}

void two_bit_bimodal(char * input_file, FILE * output_file, int size) {
    // 0 = strongly not taken, 1 = weakly not taken
    // 2 = weakly taken, 3 = strongly taken
    unsigned long long table_size = pow(2, size); // size of prediction table
    printf("Two bit bimodal size %llu\n", table_size);
    // index = address AND size
    int prediction_table[table_size];
    for (int i = 0; i < (int)table_size; i++) {
        prediction_table[i] = 3; // strongly taken
    }

    FILE * input = fopen(input_file, "r");
    // Temporary variables
    unsigned long long addr;
    char behavior[10];
    unsigned long long target;

    int correct_total = 0;
    int total = 0;

    while(fscanf(input, "%llx %10s %llx\n", &addr, behavior, &target) != EOF) {
        unsigned long long index = addr & (table_size - 1); // get last size bits
        // printf("Index = %llu\n", index);
        if (strncmp(behavior, "NT", 2) == 0) { // not taken
            if (prediction_table[index] == 0) { // strongly not taken (hit)
                correct_total++;
            }
            else if (prediction_table[index] == 1) { // weakly not taken
                correct_total++;
                prediction_table[index] = 0;
            }
            else if (prediction_table[index] == 2) { // weakly taken
                prediction_table[index] = 1;
            }
            else { // strongly taken
                prediction_table[index] = 2;
            }
        }
        else { // taken
            if (prediction_table[index] == 0) { // strongly not taken (hit)
                prediction_table[index] = 1;
            }
            else if (prediction_table[index] == 1) { // weakly not taken
                prediction_table[index] = 2;
            }
            else if (prediction_table[index] == 2) { // weakly taken
                correct_total++;
                prediction_table[index] = 3;
            }
            else { // strongly taken
                correct_total++;
            }
        }
        total++;
    }
    fprintf(output_file, "%d,%d; ", correct_total, total);
}

// GHSARE HELPER FUNCTIONS
unsigned long long binary_decimal_convert(unsigned long long binaryArray[], int size) {
    unsigned long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum += binaryArray[i] * pow(2, size - i - 1);
    }
    return sum;
}

void gshare(char * input_file, FILE * output_file, int ghr_size) {
    unsigned long long table_size = 2048; // size of prediction table
    printf("Gshare with ghr size %d\n", ghr_size);
    int prediction_table[table_size];
    for (int i = 0; i < (int)table_size; i++) {
        prediction_table[i] = 3; // strongly taken
    }

    unsigned long long ghr[ghr_size];
    memset(ghr, 0, ghr_size);
    for (int i = 0; i < ghr_size; i++) {
        ghr[i] = 0;
    }
    //printf("1\n");

    FILE * input = fopen(input_file, "r");
    // Temporary variables
    unsigned long long addr;
    char behavior[10];
    unsigned long long target;

    int correct_total = 0;
    int total = 0;
    //printf("2: right before loop\n");
    while(fscanf(input, "%llx %10s %llx\n", &addr, behavior, &target) != EOF) {
        unsigned long long index = (addr ^ binary_decimal_convert(ghr, ghr_size)) & (2047); // get last size bits
        //printf("Index = %llu\n", index);
        // printf("Index = %llu\n", index);
        if (strncmp(behavior, "NT", 2) == 0) { // not taken
            if (prediction_table[index] == 0) { // strongly not taken (hit)
                correct_total++;
            }
            else if (prediction_table[index] == 1) { // weakly not taken
                correct_total++;
                prediction_table[index] = 0;
            }
            else if (prediction_table[index] == 2) { // weakly taken
                prediction_table[index] = 1;
            }
            else { // strongly taken
                prediction_table[index] = 2;
            }
        }
        else { // taken
            if (prediction_table[index] == 0) { // strongly not taken (hit)
                prediction_table[index] = 1;
            }
            else if (prediction_table[index] == 1) { // weakly not taken
                prediction_table[index] = 2;
            }
            else if (prediction_table[index] == 2) { // weakly taken
                correct_total++;
                prediction_table[index] = 3;
            }
            else { // strongly taken
                correct_total++;
            }
        }
        total++;
        // update ghr
        for (int i = 0; i < ghr_size - 1; i++) {
            ghr[i] = ghr[i+1];
        }
        if (strncmp(behavior, "NT", 2) == 0) {
            ghr[ghr_size-1] = 0;
        }
        else {
            ghr[ghr_size-1] = 1;
        }
    }
    fprintf(output_file, "%d,%d; ", correct_total, total);
}

void tournament_predictor(char * input_file, FILE * output_file) {
    // GHR = 11 bits, 2048 entries
    // bimodal table = 2048 entries
    int ghr_size = 11;
    int table_size = 2048;
    // initialize bimodal table
    int prediction_table_bimodal[table_size];
    for (int i = 0; i < table_size; i++) {
        prediction_table_bimodal[i] = 1; // taken
    }
    // initialize ghshare table
    int prediction_table_gshare[table_size];
    for (int i = 0; i < table_size; i++) {
        prediction_table_gshare[i] = 3; // strongly taken
    }
    // initialize ghr
    unsigned long long ghr[ghr_size];
    memset(ghr, 0, ghr_size);
    for (int i = 0; i < ghr_size; i++) {
        ghr[i] = 0;
    }
    // initialize selector table
    // 0 = strongly gshare, 1 = weakly gshare, 2 = weakly bimodal, 3 = strongly bimodal
    int selector[table_size];
    for (int i = 0; i < table_size; i++) {
        selector[i] = 0;
    }

    FILE * input = fopen(input_file, "r");
    // Temporary variables
    unsigned long long addr;
    char behavior[10];
    unsigned long long target;

    int correct_total = 0;
    int total = 0;

    while(fscanf(input, "%llx %10s %llx\n", &addr, behavior, &target) != EOF) {
        unsigned long long index_bim = addr & (table_size - 1);
        unsigned long long index_ghr = (addr ^ binary_decimal_convert(ghr, ghr_size)) & (2047); // get last size bits
        // printf("Index bimodal = %llu\n", index_bim);
        // printf("Index gshare = %llu\n", index_ghr);
        if (strncmp(behavior, "NT", 2) == 0) { // not taken
            // GSHARE 00
            if (prediction_table_gshare[index_ghr] == 0) { // strongly not taken (hit)
                if (prediction_table_bimodal[index_bim] == 0) { // bimodal not taken (1/1)
                    if (selector[index_bim] == 0) { // strongly gshare
                        correct_total++;
                    }
                    else if (selector[index_bim] == 1) { // weakly gshare
                        correct_total++;
                    }
                    else if (selector[index_bim] == 2) { // weakly bimodal
                        correct_total++;
                    }
                    else { // strongly bimodal
                        correct_total++;
                    }
                }
                else { // bimodal taken (1/0)
                    if (selector[index_bim] == 0) { // strongly gshare
                        correct_total++;
                        prediction_table_bimodal[index_bim] = 0;
                    }
                    else if (selector[index_bim] == 1) { // weakly gshare
                        correct_total++;
                        selector[index_bim] = 0;
                        prediction_table_bimodal[index_bim] = 0;
                    }
                    else if (selector[index_bim] == 2) { // weakly bimodal
                        selector[index_bim] = 1;
                        prediction_table_bimodal[index_bim] = 0;
                    }
                    else { // strongly bimodal
                        selector[index_bim] = 2;
                        prediction_table_bimodal[index_bim] = 0;
                    }
                }
            }
            // GSHARE 01
            else if (prediction_table_gshare[index_ghr] == 1) { // gshare weakly not taken
                if (prediction_table_bimodal[index_bim] == 0) { // bimodal not taken (1/1)
                    if (selector[index_bim] == 0) { // strongly gshare
                        correct_total++;
                        prediction_table_gshare[index_ghr] = 0;
                    }
                    else if (selector[index_bim] == 1) { // weakly gshare
                        correct_total++;
                        prediction_table_gshare[index_ghr] = 0;
                    }
                    else if (selector[index_bim] == 2) { // weakly bimodal
                        correct_total++;
                        prediction_table_gshare[index_ghr] = 0;
                    }
                    else { // strongly bimodal
                        correct_total++;
                        prediction_table_gshare[index_ghr] = 0;
                    }
                }
                else { // bimodal taken (1/0)
                    if (selector[index_bim] == 0) { // strongly gshare
                        correct_total++;
                        prediction_table_gshare[index_ghr] = 0;
                        prediction_table_bimodal[index_bim] = 0;
                    }
                    else if (selector[index_bim] == 1) { // weakly gshare
                        correct_total++;
                        selector[index_bim] = 0;
                        prediction_table_gshare[index_ghr] = 0;
                        prediction_table_bimodal[index_bim] = 0;
                    }
                    else if (selector[index_bim] == 2) { // weakly bimodal
                        selector[index_bim] = 1;
                        prediction_table_gshare[index_ghr] = 0;
                        prediction_table_bimodal[index_bim] = 0;
                    }
                    else { // strongly bimodal
                        selector[index_bim] = 2;
                        prediction_table_gshare[index_ghr] = 0;
                        prediction_table_bimodal[index_bim] = 0;
                    }
                }
            }
            // GSHARE 10
            else if (prediction_table_gshare[index_ghr] == 2) { // weakly taken
                if (prediction_table_bimodal[index_bim] == 0) { // bimodal not taken (0/1)
                    if (selector[index_bim] == 0) { // strongly gshare
                        prediction_table_gshare[index_ghr] = 1;
                        selector[index_bim] = 1;
                    }
                    else if (selector[index_bim] == 1) { // weakly gshare
                        prediction_table_gshare[index_ghr] = 1;
                        selector[index_bim] = 2;
                    }
                    else if (selector[index_bim] == 2) { // weakly bimodal
                        correct_total++;
                        prediction_table_gshare[index_ghr] = 1;
                        selector[index_bim] = 3;
                    }
                    else { // strongly bimodal
                        correct_total++;
                        prediction_table_gshare[index_ghr] = 1;
                    }
                }
                else { // bimodal taken (0/0)
                    if (selector[index_bim] == 0) { // strongly gshare
                        prediction_table_gshare[index_ghr] = 1;
                        prediction_table_bimodal[index_bim] = 0;
                    }
                    else if (selector[index_bim] == 1) { // weakly gshare
                        prediction_table_gshare[index_ghr] = 1;
                        prediction_table_bimodal[index_bim] = 0;
                    }
                    else if (selector[index_bim] == 2) { // weakly bimodal
                        prediction_table_gshare[index_ghr] = 1;
                        prediction_table_bimodal[index_bim] = 0;
                    }
                    else { // strongly bimodal
                        prediction_table_gshare[index_ghr] = 1;
                        prediction_table_bimodal[index_bim] = 0;
                    }
                }
            }
            // GSHARE 11
            else { // GSHARE strongly taken
                if (prediction_table_bimodal[index_bim] == 0) { // bimodal not taken (0/1)
                    if (selector[index_bim] == 0) { // strongly gshare
                        prediction_table_gshare[index_ghr] = 2;
                        selector[index_bim] = 1;
                    }
                    else if (selector[index_bim] == 1) { // weakly gshare
                        prediction_table_gshare[index_ghr] = 2;
                        selector[index_bim] = 2;
                    }
                    else if (selector[index_bim] == 2) { // weakly bimodal
                        correct_total++;
                        prediction_table_gshare[index_ghr] = 2;
                        selector[index_bim] = 3;
                    }
                    else { // strongly bimodal
                        correct_total++;
                        prediction_table_gshare[index_ghr] = 2;
                    }
                }
                else { // bimodal taken (0/0)
                    if (selector[index_bim] == 0) { // strongly gshare
                        prediction_table_gshare[index_ghr] = 2;
                        prediction_table_bimodal[index_bim] = 0;
                    }
                    else if (selector[index_bim] == 1) { // weakly gshare
                        prediction_table_gshare[index_ghr] = 2;
                        prediction_table_bimodal[index_bim] = 0;
                    }
                    else if (selector[index_bim] == 2) { // weakly bimodal
                        prediction_table_gshare[index_ghr] = 2;
                        prediction_table_bimodal[index_bim] = 0;
                    }
                    else { // strongly bimodal
                        prediction_table_gshare[index_ghr] = 2;
                        prediction_table_bimodal[index_bim] = 0;
                    }
                }
            }
        }

        // ACTUALLY TAKEN
        else { // taken
            // GSHARE 00
            if (prediction_table_gshare[index_ghr] == 0) { // strongly not taken (hit)
                if (prediction_table_bimodal[index_bim] == 0) { // bimodal not taken (0/0)
                    if (selector[index_bim] == 0) { // strongly gshare
                        prediction_table_gshare[index_ghr] = 1;
                        prediction_table_bimodal[index_bim] = 1;
                    }
                    else if (selector[index_bim] == 1) { // weakly gshare
                        prediction_table_gshare[index_ghr] = 1;
                        prediction_table_bimodal[index_bim] = 1;
                    }
                    else if (selector[index_bim] == 2) { // weakly bimodal
                        prediction_table_gshare[index_ghr] = 1;
                        prediction_table_bimodal[index_bim] = 1;
                    }
                    else { // strongly bimodal
                        prediction_table_gshare[index_ghr] = 1;
                        prediction_table_bimodal[index_bim] = 1;
                    }
                }
                else { // bimodal taken (0/1)
                    if (selector[index_bim] == 0) { // strongly gshare
                        selector[index_bim] = 1;
                        prediction_table_gshare[index_ghr] = 1;
                    }
                    else if (selector[index_bim] == 1) { // weakly gshare
                        selector[index_bim] = 2;
                        prediction_table_gshare[index_ghr] = 1;
                    }
                    else if (selector[index_bim] == 2) { // weakly bimodal
                        selector[index_bim] = 3;
                        correct_total++;
                        prediction_table_gshare[index_ghr] = 1;
                    }
                    else { // strongly bimodal
                        correct_total++;
                        prediction_table_gshare[index_ghr] = 1;
                    }
                }
            }
            // GSHARE 01
            else if (prediction_table_gshare[index_ghr] == 1) { // weakly not taken
                if (prediction_table_bimodal[index_bim] == 0) { // bimodal not taken (0/0)
                    if (selector[index_bim] == 0) { // strongly gshare
                        prediction_table_gshare[index_ghr] = 2;
                        prediction_table_bimodal[index_bim] = 1;
                    }
                    else if (selector[index_bim] == 1) { // weakly gshare
                        prediction_table_gshare[index_ghr] = 2;
                        prediction_table_bimodal[index_bim] = 1;
                    }
                    else if (selector[index_bim] == 2) { // weakly bimodal
                        prediction_table_gshare[index_ghr] = 2;
                        prediction_table_bimodal[index_bim] = 1;
                    }
                    else { // strongly bimodal
                        prediction_table_gshare[index_ghr] = 2;
                        prediction_table_bimodal[index_bim] = 1;
                    }
                }
                else { // bimodal taken (0/1)
                    if (selector[index_bim] == 0) { // strongly gshare
                        prediction_table_gshare[index_ghr] = 2;
                        selector[index_bim] = 1;
                    }
                    else if (selector[index_bim] == 1) { // weakly gshare
                        prediction_table_gshare[index_ghr] = 2;
                        selector[index_bim] = 2;
                    }
                    else if (selector[index_bim] == 2) { // weakly bimodal
                        prediction_table_gshare[index_ghr] = 2;
                        selector[index_bim] = 3;
                        correct_total++;
                    }
                    else { // strongly bimodal
                        prediction_table_gshare[index_ghr] = 2;
                        correct_total++;
                    }
                }
            }
            // GSHARE 10
            else if (prediction_table_gshare[index_ghr] == 2) { // weakly taken
                if (prediction_table_bimodal[index_bim] == 0) { // bimodal not taken (1/0)
                    if (selector[index_bim] == 0) { // strongly gshare
                        correct_total++;
                        prediction_table_gshare[index_ghr] = 3;
                        prediction_table_bimodal[index_bim] = 1;
                    }
                    else if (selector[index_bim] == 1) { // weakly gshare
                        correct_total++;
                        prediction_table_gshare[index_ghr] = 3;
                        selector[index_bim] = 0;
                        prediction_table_bimodal[index_bim] = 1;
                    }
                    else if (selector[index_bim] == 2) { // weakly bimodal
                        prediction_table_gshare[index_ghr] = 3;
                        selector[index_bim] = 1;
                        prediction_table_bimodal[index_bim] = 1;
                    }
                    else { // strongly bimodal
                        prediction_table_gshare[index_ghr] = 3;
                        selector[index_bim] = 2;
                        prediction_table_bimodal[index_bim] = 1;
                    }
                }
                else { // bimodal taken (1/1)
                    if (selector[index_bim] == 0) { // strongly gshare
                        correct_total++;
                        prediction_table_gshare[index_ghr] = 3;
                    }
                    else if (selector[index_bim] == 1) { // weakly gshare
                        correct_total++;
                        prediction_table_gshare[index_ghr] = 3;
                    }
                    else if (selector[index_bim] == 2) { // weakly bimodal
                        correct_total++;
                        prediction_table_gshare[index_ghr] = 3;
                    }
                    else { // strongly bimodal
                        correct_total++;
                        prediction_table_gshare[index_ghr] = 3;
                    }
                }
            }
            // GSHARE 11
            else { // strongly taken
                if (prediction_table_bimodal[index_bim] == 0) { // bimodal not taken (1/0)
                    if (selector[index_bim] == 0) { // strongly gshare
                        correct_total++;
                        prediction_table_bimodal[index_bim] = 1;
                    }
                    else if (selector[index_bim] == 1) { // weakly gshare
                        correct_total++;
                        prediction_table_bimodal[index_bim] = 1;
                        selector[index_bim] = 0;
                    }
                    else if (selector[index_bim] == 2) { // weakly bimodal
                        prediction_table_bimodal[index_bim] = 1;
                        selector[index_bim] = 1;
                    }
                    else { // strongly bimodal
                        prediction_table_bimodal[index_bim] = 1;
                        selector[index_bim] = 2;
                    }
                }
                else { // bimodal taken (1/1)
                    if (selector[index_bim] == 0) { // strongly gshare
                        correct_total++;
                    }
                    else if (selector[index_bim] == 1) { // weakly gshare
                        correct_total++;
                    }
                    else if (selector[index_bim] == 2) { // weakly bimodal
                        correct_total++;
                    }
                    else { // strongly bimodal
                        correct_total++;
                    }
                }
            }
        }
        total++;
        // update ghr
        for (int i = 0; i < ghr_size - 1; i++) {
            ghr[i] = ghr[i+1];
        }
        if (strncmp(behavior, "NT", 2) == 0) {
            ghr[ghr_size-1] = 0;
        }
        else {
            ghr[ghr_size-1] = 1;
        }
    }
    fprintf(output_file, "%d,%d; ", correct_total, total);
}

void btb_predictor(char * input_file, FILE * output_file) {
    int table_size = 512;
    int prediction_table[table_size];
    for (int i = 0; i < table_size; i++) {
        prediction_table[i] = 1;
    }
    unsigned long long btb_table[table_size];
    for (int i = 0; i < table_size; i++) {
        btb_table[i] = 0;
    }

    FILE * input = fopen(input_file, "r");
    // Temporary variables
    unsigned long long addr;
    char behavior[10];
    unsigned long long target;

    int correct_total = 0;
    int total = 0;

    while(fscanf(input, "%llx %10s %llx\n", &addr, behavior, &target) != EOF) {
        unsigned long long index = addr & (table_size - 1); // get last size bits
        // printf("Index = %llu\n", index);
        if (strncmp(behavior, "NT", 2) == 0) { // not taken
            if (prediction_table[index] == 1) { // predict taken
                if (btb_table[index] == target) {
                    correct_total++;
                }
                prediction_table[index] = 0;
            }
        }
        else { // taken
            if (prediction_table[index] == 0) { // not taken
                btb_table[index] = target;
                prediction_table[index] = 1;
            }
            else { // predict taken
                if (btb_table[index] == target) {
                    correct_total++;
                }
                btb_table[index] = target;
            }
        }
        total++;
    }
    fprintf(output_file, "%d,%d; ", correct_total, total);

}

int main(int argc, char *argv[]) {
    // argv[1] = input file name
    char * input_name = argv[1];
    // argv[2] = output file name
    char * output_name = argv[2];

    // Temporary variables
    unsigned long long addr;
    char behavior[10];
    unsigned long long target;

    // open output FILE
    FILE *output;
    output = fopen(output_name, "w");

    // ALL TAKEN AND ALL NOT TAKEN TEST
    // Open file for reading for all taken, all not taken
    FILE *input;
    input = fopen(input_name, "r");

    int taken = 0;
    int not_taken = 0;
    int total = 0;
    while(fscanf(input, "%llx %10s %llx\n", &addr, behavior, &target) != EOF) {
      // printf("%s\n", behavior);

      // if (behavior.find("NT") != string::npos) {
      if (strncmp(behavior, "NT", 2) == 0) {
          // printf("Taken\n");
          not_taken++;
      }
      else {
          // printf("Not Taken\n");
          taken++;
      }
      total++;

    }
    fclose(input);
    // printf("Number of branches taken: %d\n", taken);
    // printf("Number of branches not taken: %d\n", not_taken);
    fprintf(output, "%d,%d;\n", taken, total);
    fprintf(output, "%d,%d;\n", not_taken, total);

    // SINGLE BIT BIMODAL TESTS
    single_bit_bimodal(input_name, output, 4); // table size 16
    single_bit_bimodal(input_name, output, 5); // table size 32
    single_bit_bimodal(input_name, output, 7); // table size 128
    single_bit_bimodal(input_name, output, 8); // table size 256
    single_bit_bimodal(input_name, output, 9); // table size 512
    single_bit_bimodal(input_name, output, 10); // table size 1024
    single_bit_bimodal(input_name, output, 11); // table size 2048
    fprintf(output, "\n");

    // TWO BIT BIMODAL TESTS
    two_bit_bimodal(input_name, output, 4); // table size 16
    two_bit_bimodal(input_name, output, 5); // table size 32
    two_bit_bimodal(input_name, output, 7); // table size 128
    two_bit_bimodal(input_name, output, 8); // table size 256
    two_bit_bimodal(input_name, output, 9); // table size 512
    two_bit_bimodal(input_name, output, 10); // table size 1024
    two_bit_bimodal(input_name, output, 11); // table size 2048
    fprintf(output, "\n");

    gshare(input_name, output, 3);
    gshare(input_name, output, 4);
    gshare(input_name, output, 5);
    gshare(input_name, output, 6);
    gshare(input_name, output, 7);
    gshare(input_name, output, 8);
    gshare(input_name, output, 9);
    gshare(input_name, output, 10);
    gshare(input_name, output, 11);
    fprintf(output, "\n");

    tournament_predictor(input_name, output);
    fprintf(output, "\n");

    btb_predictor(input_name, output);

    fclose(output);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <math.h>
#include "sim_bp.h"
int c=0; int c1=0;
/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim bimodal 6 gcc_trace.txt
    argc = 4
    argv[0] = "sim"
    argv[1] = "bimodal"
    argv[2] = "6"
    ... and so on
*/

void Bimodal(unsigned long int B[], unsigned long int index, unsigned long int sets, char outcome)
{

 for (int i=0; i<sets; i++)
     {
    if ( (i==index && B[i]>=2 && outcome=='n') || (i==index && B[i]<=1 && outcome=='t') )
     		c1++;
     		
        if (i==index && outcome=='t')
        {
            B[i] = ( (B[i]++)>=3) ? 3 : B[i]++ ;
            goto OUT;
        }
        else if (i==index && outcome=='n')
        {
           B[i] = ( (B[i]--)<=0) ? 0 : B[i]-- ;
            goto OUT;
        }
    }
    OUT:c++;
}

int GetIndex (unsigned long int addr, unsigned long int size_of_pc)
{
	unsigned long int n = addr; int binary[32]={0};	unsigned long int sum=0;			// here index and tags are basically the index.size() & tag.size()
	int q=31; 																			// respectively
    while (n!=0 && q>0)
    {
        binary[q]=n%2;                                                                  // 31 is the least signifcant bit
        n=n/2; q--;
    }
     for (int i=(30-size_of_pc); i<(30); i++)
        sum=sum+pow(2,(29-i))*binary[i];
     return sum;
}

int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    bp_params params;       // look at sim_bp.h header file for the the definition of struct bp_params
    char outcome;           // Variable holds branch outcome
    unsigned long int addr; // Variable holds the address read from input file

    if (!(argc == 4 || argc == 5 || argc == 7))
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }

    params.bp_name  = argv[1];

    // strtoul() converts char* to unsigned long. It is included in <stdlib.h>
    if(strcmp(params.bp_name, "bimodal") == 0)              // Bimodal
    {
        if(argc != 4)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M2       = strtoul(argv[2], NULL, 10);
        trace_file      = argv[3];
        printf("COMMAND\n%s %s %lu %s\n", argv[0], params.bp_name, params.M2, trace_file);
    }
    else if(strcmp(params.bp_name, "gshare") == 0)          // Gshare
    {
        if(argc != 5)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M1       = strtoul(argv[2], NULL, 10);
        params.N        = strtoul(argv[3], NULL, 10);
        trace_file      = argv[4];
        printf("COMMAND\n%s %s %lu %lu %s\n", argv[0], params.bp_name, params.M1, params.N, trace_file);

    }
    else if(strcmp(params.bp_name, "hybrid") == 0)          // Hybrid
    {
        if(argc != 7)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.K        = strtoul(argv[2], NULL, 10);
        params.M1       = strtoul(argv[3], NULL, 10);
        params.N        = strtoul(argv[4], NULL, 10);
        params.M2       = strtoul(argv[5], NULL, 10);
        trace_file      = argv[6];
        printf("COMMAND\n%s %s %lu %lu %lu %lu %s\n", argv[0], params.bp_name, params.K, params.M1, params.N, params.M2, trace_file);

    }
    else
    {
        printf("Error: Wrong branch predictor name:%s\n", params.bp_name);
        exit(EXIT_FAILURE);
    }

    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    unsigned long int sets = pow(2,params.M2);
    unsigned long int B[sets];
     for (int i=0; i<sets; i++)
        B[i]=2;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    char str[2];
while(fscanf(FP, "%lx %s", &addr, str) != EOF)
    {
        outcome = str[0];
/********************************************************************************************************************************************************************************************
                                                            Add branch predictor code here
******************************************************************************************************************************************************************************************************/
     unsigned long int index=GetIndex(addr, params.M2);
     Bimodal(B,index,sets,outcome);
	}
	float a = c; float b = c1;
	printf("OUTPUT\n");
	printf("number of predictions: %d\n", c);
    printf("number of mispredictions: %d\n", c1);
    printf("misprediction rate: %0.2f%\n", b*100/a);
    printf("FINAL BIMODAL CONTENTS\n");
    for (int i=0; i<sets; i++)
        {
            printf("%lu\t", i);
            printf("%lu\n", B[i]);
        }
    return 0;
}

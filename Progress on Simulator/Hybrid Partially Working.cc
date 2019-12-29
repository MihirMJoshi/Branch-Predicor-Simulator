#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <math.h>
#include "sim_bp.h"
int c=0; int c1=0;
bp_params params;       														// look at sim_bp.h header file for the the definition of struct bp_params
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

///////////////////////////////////////////////Common Functions///////////////////////////////////////////////////////////////////////////////////////////////
int GetIndex (unsigned long int addr, unsigned long int size_of_pc, int minus)
{
	unsigned long int n = addr; int binary[32]={0};	unsigned long int sum=0;			// here index and tags are basically the index.size() & tag.size()
	int q=31; 																			// respectively
    while (n!=0 && q>0)
     {
        binary[q]=n%2;                                                                  // 31 is the least signifcant bit
        n=n/2; q--;
     }

	 if (minus==0)
     	for (int i=(30-size_of_pc); i<(30); i++)
        	sum=sum+pow(2,(29-i))*binary[i];


    else if(minus==1)																	// to extract the bits from reverse side
    	for (int i=(32-size_of_pc); i<(32); i++)
        	sum=sum+pow(2,(31-i))*binary[i];

    else if (minus==2)																	// to extract the bits from front side
    	for (int i=(32-params.M1); i<( 32-(params.M1-size_of_pc) ); i++)
    		sum=sum+pow(2,(31-i-params.M1+params.N) )*binary[i];

     	return sum;
}

void G_history_counter(char outcome, int G[])
{
	for (int i=0; i<params.N; i++)
	{
		G[params.N-1-i]=0;
		G[params.N-1-i]=G[params.N-2-i];
	}
	G[0] = (outcome=='t') ? 1 : 0;
}

int Global_History_Value(int G[],char outcome)
{
	unsigned long int sum=0;
	for (int i=0; i<params.N; i++)
		sum=sum+pow(2,params.N-i-1)*G[i];

	G_history_counter(outcome,G);														// update the gloabl history only after you are done with the operations
	return sum;
}

int ConcatenateValues(unsigned long int new_n, unsigned long int m_minus_n)
{
	int diff = params.M1-params.N;   int q=params.N-1; 			int q1=(params.M1-params.N)-1;
	unsigned long int x = m_minus_n; int binary1[diff];			int combine[params.M1];
	unsigned long int n = new_n; 	 int binary0[params.N];		unsigned long int sum=0;

	for (int i=0; i<params.M1; i++)														// Initialise everything to zero
		combine[i]=0;
	for (int i=0; i<params.N; i++)														// Initialize to zero
		binary0[i]=0;
	for (int i=0; i<(params.M1-params.N); i++)											// Initialise everything to zero
		binary1[i]=0;

    while (n!=0)
     {
        binary0[q]=n%2;                                                                  // 31 is the least signifcant bit
        n=n/2; q--;
     }
    for (int i=0; i<params.N; i++)
		combine[params.M1-1-i]=binary0[i];

    while (x!=0)
     {
        binary1[q1]=x%2;                                                                 // 31 is the least signifcant bit
        x=x/2; q1--;
     }

	for (int i=0; i<(params.M1-params.N); i++)
		combine[params.M1-params.N-1-i]=binary1[i];

	for (int i=params.M1-1; i>=0; i--)
		sum=sum+pow(2,i)*combine[i];

	return sum;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

void UpdateChooserCounter(unsigned long int C[], unsigned long int index3, char outcome, unsigned long int value0, unsigned long int value1)
{
	// g = value0
	// b =value1
	//case 1 : Gshare correct and Bimodal incorrect
	if ( (outcome=='t' && value0>=2 && value1<=1) || (outcome=='n' &&  value0<=1 && value1>=2  ) )
		C[index3] = ((C[index3]++)>=3) ? 3 : C[index3]++;
	//case 2: Gshare incorrect & Bimodal Correct
	else if ( (outcome=='n' && value0>=2 && value1<=1) || (outcome=='t' &&  value0<=1 && value1>=2  ) )
		C[index3] = ((C[index3]--)<=0) ? 0 : C[index3]--;
	// case 3: Both Correct 
	else if ( (outcome=='n' && value0<=1 && value1<=1) || (outcome=='t' && value0>=2 && value1>=2) ) 
		C[index3]=C[index3];
	// case 4: Both Incorrect
	else if ( (outcome=='t' && value0<=1 && value1<=1) || (outcome=='n' && value0>=2 && value1>=2))
		C[index3]=C[index3];
		
//	if ( (C[index3]>=2 && outcome=='n') || (C[index3]<=1 && outcome=='t') )
//		c1++;
}		

void UpdateCounterValues(unsigned long int array[],unsigned long int index, char c)
{
	if (c=='t')
	array[index] = ((array[index]++)>=3) ? 3 : array[index]++;
	else if (c=='n')
	array[index] = ((array[index]--)<=0) ? 0 : array[index]--;
}

int G_Share(unsigned long int index, unsigned long int sets, char outcome, int G[])
{
	unsigned long int n_for_g_share = GetIndex(index,params.N,2);							// extract n (from front) only first
	unsigned long int m_minus_n_for_g_share = GetIndex(index,(params.M1-params.N),1);		// extract m minus n (from end) only first
	unsigned long int g = Global_History_Value(G,outcome);									// extract the contents of g variable only first
	unsigned long int new_n = (g^n_for_g_share);											// finding out new value for (n previous_n ^ g_share contents)
	int new_index=ConcatenateValues(new_n,m_minus_n_for_g_share);							// extract m bits by concatenating new_n with m_minus_n
	return new_index;
}

int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;

    char outcome;           // Variable holds branch outcome
    unsigned long int addr; // Variable holds the address read from input file
    unsigned long int index;// Find Out the index value
    unsigned long int index2;// Find Out the index value for gshare
    unsigned long int index3; // Find Out the index value for hybrid
    unsigned long int sets; // Total number of entries in the table
    unsigned long int sets_for_bimodal, sets_for_gshare;


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

	if(strcmp(params.bp_name, "bimodal") == 0)              // Bimodal
		sets = pow(2,params.M2);
	
	else if(strcmp(params.bp_name, "gshare") == 0)          // Gshare
		sets = pow(2,params.M1);
		
	else if(strcmp(params.bp_name, "hybrid") == 0)			// Hybrid
		{
			sets = pow(2,params.K);
			sets_for_gshare = pow(2,params.M1);
			sets_for_bimodal = pow(2,params.M2);				
		}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	unsigned long int B[sets];
     for (int i=0; i<sets; i++)								// Making The B counter and initialising its contents to 2
        B[i]=2;	
    
	int G[params.N];
	for (int i=0; i<params.N; i++)							// Making Global Index counter and initialising al its conents to 0 
		G[i]=0;												// global history is common for all
		
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    unsigned long int B_for_hybrid[sets_for_bimodal];
     for (int i=0; i<sets_for_bimodal; i++)					// Making The B counter for hybrid and initialising its contents to 2
        B_for_hybrid[i]=2;	
	
	 unsigned long int G_for_hybrid[sets_for_gshare];
     for (int i=0; i<sets_for_gshare; i++)					// Making The G counter for hybrid and initialising its contents to 2
        G_for_hybrid[i]=2;	
	
	unsigned long int C[sets];
     for (int i=0; i<sets; i++)								// Making The Chooser(C) counter for hybrid and initialising its contents to 2
        C[i]=1;	
	
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
	char str[2];int i=0;
while(fscanf(FP, "%lx %s", &addr, str) != EOF && i!=9999+1)
    {
        outcome = str[0]; 
/********************************************************************************************************************************************************************************************
                                                            Add branch predictor code here
******************************************************************************************************************************************************************************************************/
     if(strcmp(params.bp_name, "bimodal") == 0)                     						// Bimodal
		{
	 		index=GetIndex(addr, params.M2,0);
	 		Bimodal(B,index,sets,outcome);
	 	}

	else if(strcmp(params.bp_name, "gshare") == 0)           								// Gshare
		{
			index=GetIndex(addr, params.M1,0);
			unsigned long int new_index=G_Share(index,sets,outcome,G);
			Bimodal(B,new_index,sets,outcome);
		}
	else if(strcmp(params.bp_name, "hybrid") == 0)											// Hybrid
		{
			c++;
			//	printf("=%d\t%lx %c\n",i,addr,outcome);
			index2=GetIndex(addr, params.M1,0);
			unsigned long int new_index=G_Share(index2,sets_for_gshare,outcome,G);			// Printing Out the Gshare values first
			//	printf("    GP: %lu %d\n", new_index, G_for_hybrid[new_index]);
			unsigned long int value0=G_for_hybrid[new_index];
			
			
			index=GetIndex(addr, params.M2,0);
			//	printf("    BP: %lu %d\n",index, B_for_hybrid[index]);								// Printing out the Bimodal values
			unsigned long int value1=B_for_hybrid[index];
			
			
			index3=GetIndex(addr,params.K,0);												// Printing Out the Chooser table's value
		 	//	printf("    CP: %lu %d\n", index3, C[index3]);	
			unsigned long int x = C[index3];						
			
			if (x>=2)
			{
				UpdateCounterValues(G_for_hybrid,new_index,outcome);
				UpdateChooserCounter(C,index3,outcome,value0,value1);
			//	printf("    GU: %lu %d\n",new_index, G_for_hybrid[new_index]);
			}
			else if (x<=1)
			{
				UpdateCounterValues(B_for_hybrid,index,outcome);							// Update the gloabl history only after 
				//G_history_counter(outcome,G);		
				UpdateChooserCounter(C,index3,outcome,value0,value1);						// you are done with the operations
			//	printf("    BU: %lu %d\n",index, B_for_hybrid[index]);			
			}
			//	printf("    CU: %lu %d\n",index3,C[index3]);
			
		//		printf("\n");i++;
		}
	}
for (int i=0; i<sets; i++)
	if ( (C[i]>=2 && outcome=='n') && i==index3 || (C[i]<=1 && outcome=='t' && outcome=='t') )
			c1++;	
	
float a = c; float b = c1;		
if(strcmp(params.bp_name, "hybrid") == 0)											// Hybrid
	{
		printf("OUTPUT\n");
		printf("number of predictions: %d\n", c);
   		printf("number of mispredictions: %d\n", c1);
   		printf("misprediction rate: %0.2f%\n", b*100/a);
   		/*
   		printf("FINAL CHOOSER CONTENTS\n");												// Chooser 
   		for (int i=0; i<sets; i++)
       		{
           		printf("%lu\t", i);
           		printf("%lu\n", C[i]);
			}
    		
		printf("FINAL GSHARE CONTENTS\n");												// gshare 
   		for (int i=0; i<sets_for_gshare; i++)
       		{
           		printf("%lu\t", i);
           		printf("%lu\n", G_for_hybrid[i]);
			}
			
		printf("FINAL BIMODAL CONTENTS\n");												// bimodal
   		for (int i=0; i<sets_for_bimodal; i++)
       		{
           		printf("%lu\t", i);
           		printf("%lu\n", B_for_hybrid[i]);
			}*/
	}
	else
	{
		printf("OUTPUT\n");
		printf("number of predictions: %d\n", c);
   		printf("number of mispredictions: %d\n", c1);
   		printf("misprediction rate: %0.2f%\n", b*100/a);
   		if(strcmp(params.bp_name, "bimodal") == 0)              		  				// Bimodal
   			printf("FINAL BIMODAL CONTENTS\n");
   		else if(strcmp(params.bp_name, "gshare") == 0)           		  				// Gshare
   			printf("FINAL GSHARE CONTENTS\n");
   		for (int i=0; i<sets; i++)
       		{
           		printf("%lu\t", i);
           		printf("%lu\n", B[i]);
			}
	}
    return 0;
}

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

                                                           /* Constant Values */
#define TRUE  1
#define FALSE 0
#define LINESZ 33
#define MAX_SZ 32

                                                       /* Function Prototypes */
char *removeBlanks(char *string);

int main(int argc, char *argv[]) {

                            /* Used to hold the decimal or hexadecimal string */
   char *lineBuffer = (char *)malloc(sizeof(char) * LINESZ);
   char *hitOrMiss = (char *)malloc(sizeof(char) * 5);

                           /* Used to open the file given on the command line */
   FILE *file;

                    /* All the values we are calculating from the hits/misses */
   float missRatio = 0.0;
   int accesses = 0;
   int hits = 0;
   int misses = 0;
   int oldTagValue;

                     /* This turns on verbose mode. It is basically a boolean */
   int tracing;

                     /* These variables hold the values needed to grab portions
                        of the hexadecimal or decimal string once converted */
   unsigned int blockNumber;
   unsigned int hexAddress;
   unsigned int tagValue;
   unsigned int shiftValue;

                                /* Check for the correct number of parameters */
   if (argc != 5) {
      fprintf(stderr, "Usage: %s <2^Cache Size> <2^Block Size> <Tracing On/Off>"
      	" <Filename>\n", argv[0]);
      return 1;
   }

                            /* Calculate the cache size and block size based on
                            the command line parameters entered from the user */
   if (!(argv[1] > 0)) {
      fprintf(stderr, "Invalid cache size.\n");
      return 1;
   }

   if (!(atoi(argv[2]) <= atoi(argv[1]))) {
      fprintf(stderr, "Block size cannot be larger than the cache size.\n");
      return 1;
   }

                               /* Calculate the tag, index, and offset values */
   int index = atoi(argv[1]) - atoi(argv[2]);
   int offset = atoi(argv[2]);
   //int tag = 32 - index - offset;

                          /* Turn on tracing based on the input from the user */
   if (strcmp(argv[3], "on") == 0) {
      tracing = TRUE;
   } else if (strcmp(argv[3], "off") == 0) {
      tracing = FALSE;
   } else {
      fprintf(stdout, "Enter \"on\" or \"off\". Exiting program.\n");
      return 1;
   }

             /* Determine if the file opens. If not, exit out of the program. */
   if ((file = fopen(argv[4], "r")) == NULL) {
      fprintf(stderr, "No file found.\n");
      return 1;
   }

                       /* Set up the array that holds the tags and init to -1 */
   int cacheSize = pow(2, index);
   int i;
   int cache[cacheSize];

   for(i = 0; i < cacheSize; i++) {
      cache[i] = -1;
   }

                                  /* Create the table header if tracing is on */
   if (tracing) {
      printf("*--------------------------------------------------------------");
      printf("---------------*\n");
      printf("|%8s|%8s|%8s|%8s|", "Address", "Tag", "Block #", "Old Tag");
      printf("%5s|%7s|%7s|%8s|", "Hit?", "Hits", "Misses", "Accesses");
      printf("%8s|\n", "Miss Ratio");
      printf("|==============================================================");
      printf("===============|\n");
   }

   while (fgets(lineBuffer, LINESZ, file) != NULL) {
      lineBuffer = removeBlanks(lineBuffer);
      hexAddress = strtoul(lineBuffer, NULL, 0);

      accesses++;

      shiftValue = pow(2, MAX_SZ) - pow(2, atoi(argv[1]));
      tagValue = hexAddress & shiftValue;
      tagValue = tagValue >> atoi(argv[1]);

      shiftValue = pow(2, atoi(argv[1])) - pow(2, offset);
      blockNumber = hexAddress & shiftValue;
      blockNumber = blockNumber >> offset;

      int indexValue = (int) blockNumber;

      oldTagValue = cache[indexValue];

      if (oldTagValue == -1) {
         hitOrMiss = "Miss";
         cache[indexValue] = tagValue;
         misses++;
      } else {
         if (tagValue == oldTagValue) {
            hitOrMiss = "Hit";
            hits++;
         } else {
            hitOrMiss = "Miss";
            cache[indexValue] = tagValue;
            misses++;
         }
      }

      missRatio = (float) misses / (float) accesses;
      
      if (tracing) {
         if (oldTagValue == -1) {
            printf("|%8x|%8x|%8x|%8s|%5s|%7d|%7d|%8d|%.8f|", 
               hexAddress, tagValue, blockNumber, " ", hitOrMiss, hits, 
               misses, accesses, missRatio);
         } else {
            printf("|%8x|%8x|%8x|%8d|%5s|%7d|%7d|%8d|%.8f|", 
               hexAddress, tagValue, blockNumber, oldTagValue, hitOrMiss, hits, 
               misses, accesses, missRatio);
         }
      
         printf("\n");
      }
   }

                                   /* Print the table footer if tracing is on */
   if (tracing) {
      printf("*--------------------------------------------------------------");
      printf("---------------*\n");
   }

                                     /* Print the final output of the program */
   printf("\nName: Michael A. Mendoza\n");
   printf("Parameters: %s %s %s %s\n", argv[1], argv[2], argv[3], argv[4]);
   printf("Hits: %d\n", hits);
   printf("Misses: %d\n", misses);
   printf("Miss Ratio: %0.8f\n", missRatio);

   return 0;
}

                       /* This function will remove any blanks from the front or
                                       back of the line we get from the file. */
char *removeBlanks(char *string) {
   size_t length = 0;
   char *frontPointer = string - 1;
   char *endPointer = NULL;

                                      /* If string is NULL, return NULL back. */
   if (string == NULL)
      return NULL;

   if (string[0] == '\0')
      return string;

   length = strlen(string);
   endPointer = string + length;

         /* Keep incrementing the pointer over any trailing or leading spaces */
   while (isspace(*(++frontPointer))) ;
   while (isspace(*(--endPointer)) && endPointer != frontPointer) ;

                     /* Null-terminate the string and prep it to be returned. */
   if (string + length - 1 != endPointer)
      *(endPointer + 1) = '\0';
   else if (frontPointer != string && endPointer == frontPointer)
      *string = '\0';

   endPointer = string;

   if (frontPointer != string) {
      while (*frontPointer)
         *endPointer++ = *frontPointer++;
      *endPointer = '\0';
   }

   return string;
}

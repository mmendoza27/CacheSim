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
   int associativity = 0;
   int hits = 0;
   int indexValue = 0;
   int lastIndex = 0;
   int misses = 0;
   int *oldTagValue;

         /* This turns on the various modes. They are basically all booleans. */
   int directMapped = FALSE;
   int fifo = FALSE;
   int fullyAssociative = FALSE;
   int found = FALSE;
   int lru = FALSE;
   int nWayAssociative = FALSE;
   int tracing = FALSE;

                     /* These variables hold the values needed to grab portions
                        of the hexadecimal or decimal string once converted */
   unsigned int blockNumber;
   unsigned int hexAddress;
   unsigned int tagValue;
   unsigned int setNumber;
   unsigned int shiftValue;

                                /* Check for the correct number of parameters */
   if (argc != 7) {
      fprintf(stderr, "Usage: %s <Cache Size> <Block Size> <Associativity> "
        "<Tracing>\n\t<Replacement Policy> <Filename>\n", argv[0]);
      return 1;
   }

                            /* Calculate the cache size and block size based on
                            the command line parameters entered from the user */
   if (!(atoi(argv[1]) > 0)) {
      fprintf(stderr, "Invalid cache size.\n");
      return 1;
   }

   if (!(atoi(argv[2]) > 0)) {
      fprintf(stderr, "Invalid block size.\n");
      return 1;
   }

   if (!(atoi(argv[2]) <= atoi(argv[1]))) {
      fprintf(stderr, "Block size cannot be larger than the cache size.\n");
      return 1;
   }

                               /* Calculate the tag, index, and offset values */
   int index = atoi(argv[1]) - atoi(argv[2]);
   int offset = atoi(argv[2]);

                       /* Set the associativity (0 = direct, 1 = 2-way, etc.) */

   int p = atoi(argv[3]);

   if (p < 0 || p > index) {
      p = index;
   }

   if (p == 0) {
      directMapped = TRUE;
      associativity = 0;
   } else if ((p > 0) && (p <= index)) {
         if (p == index) {
            fullyAssociative = TRUE;
            associativity = 0;
         } else {
            nWayAssociative = TRUE;
            associativity = atoi(argv[3]);            
         }
   } else {
      fullyAssociative = TRUE;
      associativity = 0;      
   }

                                   /* Set the replacement policy (fifo | lru) */
   if (strcmp(argv[4], "fifo") == 0) {
      fifo = TRUE;
   } else if (strcmp(argv[4], "lru") == 0) {
      lru = TRUE;
   } else {
      fprintf(stdout, "Please choose either \"FIFO\" or \"LRU\". "
         "Exiting program\n");
      return 1;
   }

                          /* Turn on tracing based on the input from the user */
   if (strcmp(argv[5], "on") == 0) {
      tracing = TRUE;
   } else if (strcmp(argv[5], "off") == 0) {
      tracing = FALSE;
   } else {
      fprintf(stdout, "Enter \"on\" or \"off\". Exiting program.\n");
      return 1;
   }

             /* Determine if the file opens. If not, exit out of the program. */
   if ((file = fopen(argv[6], "r")) == NULL) {
      fprintf(stderr, "No file found.\n");
      return 1;
   }

                  /* Initializing the dynamic array that simulates the cache. */
   int **cache;
   int cacheSize;
   int i;
   int j;

   if (directMapped || fullyAssociative) {
      cacheSize = pow(2, index);      
      cache = (int **)malloc(cacheSize * sizeof(int *));

      for (i = 0; i < cacheSize; i++) {
        cache[i] = (int *)malloc(1 * sizeof(int));
        oldTagValue = (int *)malloc(1 * sizeof(int));
      }

   } else {
      cacheSize = pow(2, (index - associativity));
      cache = (int **)malloc(cacheSize * sizeof(int *));

      for (i = 0; i < cacheSize; i++) {
        cache[i] = (int *)malloc((int) pow(2, associativity) * sizeof(int));
        oldTagValue = (int *)malloc((int) pow(2, associativity) * sizeof(int));
      }

   }

   for(i = 0; i < cacheSize; i++) {
      for (j = 0; j < pow(2, associativity); j++) {
         //fprintf(stdout, "[%d][%d]: ", i, j);
         cache[i][j] = -1;
         //fprintf(stdout, "[%d] ", cache[i][j]);
      }
      //fprintf(stdout, "\n");
   }
   //fprintf(stdout, "\n");

                                  /* Create the table header if tracing is on */
   if (tracing) {
      printf("*--------------------------------------------------------------");
      printf("------*\n");
      printf("|%8s|%8s|%8s|%5s|", "Address", "Tag", "Set #", "Hit?");
      printf("%7s|%7s|%8s|%8s|", "Hits", "Misses", "Accesses", "Miss Ratio");
      printf("%8s\n", "Tag(s)");
      printf("|==============================================================");
      printf("======|\n");
   }

   while (fgets(lineBuffer, LINESZ, file) != NULL) {
      lineBuffer = removeBlanks(lineBuffer);
      hexAddress = strtoul(lineBuffer, NULL, 0);

      accesses++;
      found = FALSE;
                                         /* Meaning this is fully associative */
      if (fullyAssociative) {
         shiftValue = pow(2, MAX_SZ) - pow(2, (atoi(argv[1]) - index));
         tagValue = hexAddress & shiftValue;
         tagValue = tagValue >> (atoi(argv[1]) - index);
      } else {
         shiftValue = pow(2, MAX_SZ) - pow(2, (atoi(argv[1]) - associativity));
         tagValue = hexAddress & shiftValue;
         tagValue = tagValue >> (atoi(argv[1]) - associativity);
      }

      if (directMapped) {
         shiftValue = pow(2, atoi(argv[1])) - pow(2, offset);
         blockNumber = hexAddress & shiftValue;
         blockNumber = blockNumber >> offset;
         indexValue = (int) blockNumber;
         setNumber = 0;
      } else {
         setNumber = hexAddress >> (atoi(argv[2]));
         setNumber = setNumber % (int)(pow(2, (index - associativity)));
      }

      if (directMapped) {
         //oldTagValue[0] = cache[indexValue][0];

         if (tagValue == cache[indexValue][0]) {
            // If tag is found inside set, you have a hit.
            hitOrMiss = "Hit";
            hits++;
         } else if (cache[indexValue][0] == -1) {
            // If the field contains -1, it's an "empty" entry.
            hitOrMiss = "Miss";
            cache[indexValue][0] = tagValue;
            misses++;            
         } else {
            // If tag is not found anywhere inside set, you have a miss.
            hitOrMiss = "Miss";
            cache[indexValue][0] = tagValue;
            misses++;            
         }

         missRatio = (float) misses / (float) accesses;
         
         if (tracing) {
            printf("|%8x|%8x|%8d|%5s|%7d|%7d|%8d|%.8f| %x", hexAddress, 
               tagValue, setNumber, hitOrMiss, hits, misses, accesses, 
               missRatio, tagValue);
         
            printf("\n");
         }

      } else if(nWayAssociative) {
         if (fifo) {
            for(i = 0; i < pow(2, associativity); i++) {
               if (cache[setNumber][i] == tagValue) {
                  // Tag is found inside the set somewhere, we have a hit.
                  hitOrMiss = "Hit";
                  hits++;
                  found = TRUE;
                  break;
               } else if (cache[setNumber][i] == -1) {
                  hitOrMiss = "Miss";
                  cache[setNumber][i] = tagValue;
                  misses++;
                  found = TRUE;
                  break;                  
               }
            }

            if (!found) {
               // This means the array is full with values.
               hitOrMiss = "Miss";
               // Shift everything down 1 and insert at leftmost index.
               for(i = pow(2, associativity); i > 0; i--) {
                  cache[setNumber][i] = cache[setNumber][(i - 1)];
               }

               cache[setNumber][i] = tagValue;
               misses++;
            }
         }

         if (lru) {
            for(i = pow(2, associativity); i > 0; i--) {
               if (cache[setNumber][i] == tagValue) {
                  hitOrMiss = "Hit";
                  hits++;
                  found = TRUE;

                  for(j = i; j < pow(2, associativity); j++) {
                     cache[setNumber][j] = cache[setNumber][(j + 1)];
                  }

                  cache[setNumber][j] = tagValue;
                  break;
               } else if (cache[setNumber][i] == - 1) {
                  hitOrMiss = "Miss";
                  misses++;
                  found = TRUE;

                  for(j = 0; j < pow(2, associativity); j++) {
                     cache[setNumber][j] = cache[setNumber][(j + 1)];
                  }

                  cache[setNumber][j] = tagValue;
                  break;
               }
            }

            if (!found) {
               // This means the array is full with values.
               hitOrMiss = "Miss";
               misses++;

               for(j = 0; j < pow(2, associativity); j++) {
                  cache[setNumber][j] = cache[setNumber][(j + 1)];
               }

               cache[setNumber][j] = tagValue;
            }
         }

         missRatio = (float) misses / (float) accesses;
         
         if (tracing) {
            printf("|%8x|%8x|%8d|%5s|%7d|%7d|%8d|%.8f| ", hexAddress, 
               tagValue, setNumber, hitOrMiss, hits, misses, accesses, 
               missRatio);
         
            for(i = 0; i < pow(2, associativity); i++) {
               if(cache[setNumber][i] != -1) {
                  printf("%x,", cache[setNumber][i]);
               }
            }

            printf("\n");
         }

      } else if (fullyAssociative) {
         if (fifo) {
            for(i = 0; i < cacheSize; i++) {
               if (cache[i][0] == tagValue) {
                  // Tag is found inside the set somewhere, we have a hit.
                  hitOrMiss = "Hit";
                  hits++;
                  found = TRUE;
                  break;
               } else if (cache[i][0] == -1) {
                  hitOrMiss = "Miss";
                  cache[i][0] = tagValue;
                  misses++;
                  found = TRUE;
                  break;                  
               }
            }

            if (!found) {
               hitOrMiss = "Miss";

               for(i = 0; i < (cacheSize - 1); i++) {
                  cache[i][0] = cache[(i + 1)][0];
               }

               cache[i][0] = tagValue;
               misses++;
            }

         }

         if (lru) {
            for(i = 0; i < cacheSize; i++) {
               if (cache[i][0] == tagValue) {
                  hitOrMiss = "Hit";
                  hits++;
                  found = TRUE;

                  for(j = i; j < (lastIndex - 1); j++) {
                     cache[j][0] = cache[(j + 1)][0];
                  }

                  cache[(lastIndex - 1)][0] = tagValue;
                  break;
               } else if (cache[i][0] == -1) {
                  hitOrMiss = "Miss";
                  misses++;
                  found = TRUE;

                  // for(j = 0; j < (cacheSize - 1); j++) {
                  //    cache[j][0] = cache[(j + 1)][0];
                  // }

                  cache[i][0] = tagValue;
                  lastIndex++;
                  break;
               }
            }

            if (!found) {
               // This means the array is full with values.
               hitOrMiss = "Miss";
               misses++;

               for(j = 0; j < (cacheSize - 1); j++) {
                  cache[j][0] = cache[(j + 1)][0];
               }

               cache[j][0] = tagValue;
            }
         }

         missRatio = (float) misses / (float) accesses;
         
         if (tracing) {

            printf("|%8x|%8x|%8d|%5s|%7d|%7d|%8d|%.8f| ", hexAddress, 
               tagValue, setNumber, hitOrMiss, hits, misses, accesses, 
               missRatio);
         
            for(i = 0; i < cacheSize; i++) {
               if(cache[i][0] != -1) {
                  printf("%x,", cache[i][0]);
               }
            }

            printf("\n");
         }


      }

   }

                                   /* Print the table footer if tracing is on */
   if (tracing) {
      printf("*--------------------------------------------------------------");
      printf("-------*\n");
   }

                                     /* Print the final output of the program */
   printf("\nName: Michael A. Mendoza\n");
   printf("Parameters: %s %s %s %s %s %s\n", argv[1], argv[2], argv[3], 
      argv[4], argv[5], argv[6]);
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

#include <stdio.h>

// Define constants directly from op_pool.h to test compilation values
#define SMALL_OP_SIZE 256 
#define MEDIUM_OP_SIZE 512 
#define BIG_OP_SIZE 8192 

#define MAX_SMALL_OPS 500  // Increased for beekeep
#define MAX_MEDIUM_OPS 200  // Increased for beekeep
#define MAX_BIG_OPS 50     // Increased for beekeep

int main() {
    printf("MAX_SMALL_OPS: %d\n", MAX_SMALL_OPS);
    printf("MAX_MEDIUM_OPS: %d\n", MAX_MEDIUM_OPS); 
    printf("MAX_BIG_OPS: %d\n", MAX_BIG_OPS);
    printf("SMALL_OP_SIZE: %d\n", SMALL_OP_SIZE);
    printf("MEDIUM_OP_SIZE: %d\n", MEDIUM_OP_SIZE);
    printf("BIG_OP_SIZE: %d\n", BIG_OP_SIZE);
    
    printf("Total medium pool memory: %d bytes\n", MAX_MEDIUM_OPS * MEDIUM_OP_SIZE);
    
    return 0;
}
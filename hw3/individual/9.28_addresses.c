/**
 * Exercise 9.28 - Virtual Address Translation
 * 
 * Given a 32-bit virtual address and 4-KB page size,
 * extract the page number and offset.
 * 
 * Page size = 4 KB = 2^12 bytes
 * Offset bits: 12 (lower bits)
 * Page number bits: 20 (upper bits)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

// Page size is 4 KB = 4096 bytes
#define PAGE_SIZE 4096
#define PAGE_BITS 12          // log2(4096) = 12
#define PAGE_MASK 0xFFF       // Mask for 12 bits (offset)
#define PAGE_NUM_MASK 0xFFFFF // Mask for 20 bits (page number)

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <virtual_address>\n", argv[0]);
        fprintf(stderr, "Example: %s 19986\n", argv[0]);
        return 1;
    }
    
    // Parse the virtual address
    uint32_t virtual_address = (uint32_t)strtoul(argv[1], NULL, 10);
    
    // Extract page number and offset
    uint32_t offset = virtual_address & PAGE_MASK;           // Lower 12 bits
    uint32_t page_number = (virtual_address >> PAGE_BITS);   // Upper 20 bits
    
    // Print results
    printf("The address %u contains:\n", virtual_address);
    printf("page number=%u\n", page_number);
    printf("offset=%u\n", offset);
    
    return 0;
}

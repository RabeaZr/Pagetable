#include <stdio.h>

#include "os.h"

/*
 * in order to decide how many levels are there, we look at the page size, since it's 4096 bytes
 * and the PTE's size is 32 bits which means 4 bytes, that means that in each page we can fit
 * 4096/4 = 1024 = 2^10 PTEs, which means that we need 10 bits to represent every level, therefore
 * we get 2 levels of 1024 children for each node.
 * we start by making a pointer(which is also an array) to the root of the trie and from there
 * we start traversing the tree until we get to the leaves
 */

// a function that checks if the given entry is valid
int check_validity(uint32_t entry){
    return entry & 1;
}

void page_table_update(uint32_t pt, uint32_t vpn, uint32_t ppn) {
    uint32_t shifted_pt = pt << 12; // we put 12 zeros in order to send it to phys_to_virt and get a pointer to the beginning of the root
    uint32_t *head = phys_to_virt(shifted_pt);
    int index = vpn >> 10; // start by looking on the 10 left bits in order to start traversing the trie
    if (check_validity(head[index]) == 0) {
        /* if it's not valid and we want to destroy the mapping, then we just return because it's not valid
         * otherwise we create a new page for it to continue the traverse*/
        if (ppn != NO_MAPPING) {
            head[index] = (alloc_page_frame() << 12) + 1;//we create a PTE that is valid
        } else {
            return;
        }
    }
    uint32_t address = head[index] - 1; // now we get offset = 0 and we can have a pointer to a specific child of the root
    head = phys_to_virt(address); // continue to the next level
    index = vpn & 1023;// in order to get the 10 right bits because 1023 = 1111111111 in binary
    if (ppn != NO_MAPPING) {
        head[index] = (ppn << 12) + 1; // in order to make PTE that is valid
    } else {
        head[index] = NO_MAPPING; // destroy
    }
    return;
}

uint32_t page_table_query(uint32_t pt, uint32_t vpn){
    uint32_t shifted_pt = pt << 12; // same as before
    uint32_t* head = phys_to_virt(shifted_pt);
    int index = vpn >> 10;
    if ((check_validity(head[index]) == 0) || (head[index] == NO_MAPPING)){
        return NO_MAPPING; // if it's not valid we return NO MAPPING
    }
    // if we got here that means that it's valid and we can continue traversing the tree
    uint32_t address = head[index] - 1;
    head = phys_to_virt(address); // same as before
    index = vpn & 1023;
    if ((check_validity(head[index]) == 0) || (head[index] == NO_MAPPING)){
        return NO_MAPPING;
    }
    return head[index] >> 12; // we shift in order to get the ppn and not the whole address


}
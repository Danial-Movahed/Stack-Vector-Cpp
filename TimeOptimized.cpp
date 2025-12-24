#include <iostream>
#include <cstdint>

using namespace std;

#define HEAP_SIZE 1000000
#define VAR_METADATA_SIZE 5

uint8_t heap[HEAP_SIZE];

enum type {
    Int,
    Double
};

// Metadata starts from end. Last byte is number of vars. Next five bytes are: capacity, size, type, start index, id
// For ID we could also use last byte as a bitmask for available vars then use the index of bit that is 1 in it and when var is deleted set that bit as 0
// TODO: More time efficient!
int malloc(int neededByteCount) {
    int varCount = heap[HEAP_SIZE-1], counter=0;
    for(int i=0; i<HEAP_SIZE-1-varCount*VAR_METADATA_SIZE; i++) {
        if(counter == neededByteCount)
            return i-neededByteCount;
        for(int j=0; j<varCount; j++) {
            int currVarStart=heap[HEAP_SIZE-1-1-(VAR_METADATA_SIZE-4)-j*VAR_METADATA_SIZE];
            int currVarCapacity=heap[HEAP_SIZE-1-1-(VAR_METADATA_SIZE-1)-j*VAR_METADATA_SIZE];
            if(i>=currVarStart) {
                counter = 0;
                i+=currVarCapacity;
                goto collision;
            }
        }
        counter++;
    collision:
        continue;
    }
    return -1;
}
//TODO: Get init values
int DefineVector(type t) {
    const int varCount = ++heap[HEAP_SIZE-1];
    // ID
    // heap[HEAP_SIZE-1-1-(VAR_METADATA_SIZE-4)-varCount*VAR_METADATA_SIZE] = -1;
    // Start index
    heap[HEAP_SIZE-1-1-(VAR_METADATA_SIZE-4)-varCount*VAR_METADATA_SIZE] = -1;
    // Type
    heap[HEAP_SIZE-1-1-(VAR_METADATA_SIZE-3)-varCount*VAR_METADATA_SIZE] = (type)(t);
    // Size
    heap[HEAP_SIZE-1-1-(VAR_METADATA_SIZE-2)-varCount*VAR_METADATA_SIZE] = 0;
    // Capacity
    heap[HEAP_SIZE-1-1-(VAR_METADATA_SIZE-1)-varCount*VAR_METADATA_SIZE] = 0;
    return varCount;
}

int main() {

}
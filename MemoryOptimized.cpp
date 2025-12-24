#include <cstdint>
#include <iostream>

using namespace std;

#define HEAP_SIZE 1000000
#define VAR_METADATA_SIZE 5
#define STATIC_METADATA_SIZE 1

uint8_t heap[HEAP_SIZE];

enum class Type { Int, Double };

enum class VariablePropery {
    capacity = 1,
    size = 2,
    type = 3,
    startIndex = 4,
    id = 5
};

// Metadata starts from end. Last byte is number of vars. Next five bytes are:
// capacity, size, type, start index, id
int malloc(int neededByteCount) {
    int varCount = heap[HEAP_SIZE - 1], counter = 0;
    for (int i = 0; i < HEAP_SIZE - 1 - varCount * VAR_METADATA_SIZE; i++) {
        if (counter == neededByteCount) return i - neededByteCount;
        for (int j = 0; j < varCount; j++) {
            int currVarStart = GetMetadata(j, VariablePropery::startIndex);
            int currVarCapacity = GetMetadata(j, VariablePropery::capacity);
            if (i >= currVarStart) {
                counter = 0;
                // Minus one becase after continue i gets automatically
                // increamented by one by for loop
                i += currVarCapacity - 1;
                goto collision;
            }
        }
        counter++;
    collision:
        continue;
    }
    return -1;
}

int GetMetadata(int variableNumber, VariablePropery property) {
    return heap[HEAP_SIZE - 1 - STATIC_METADATA_SIZE -
                (variableNumber * VAR_METADATA_SIZE) -
                (VAR_METADATA_SIZE - (int)property)];
}
// TODO: Get init values
int DefineVector(Type t) {
    uint8_t* varCount = &heap[HEAP_SIZE - 1];
    bool varID[*varCount]={0}, id=-1;

    for(int i=0; i<*varCount; i++) {
        if(GetMetadata(i, VariablePropery::id)<*varCount)
            varID[GetMetadata(i, VariablePropery::id)]=true;
    }
    for(int i=0; i<*varCount; i++)
        if(!varID[i])
            id=i;

    // Create variable
    *varCount++;
    // Start index
    heap[HEAP_SIZE - 1 - STATIC_METADATA_SIZE - (VAR_METADATA_SIZE - 4) -
         (id)*VAR_METADATA_SIZE] = -1;
    // Type
    heap[HEAP_SIZE - 1 - STATIC_METADATA_SIZE - (VAR_METADATA_SIZE - 3) -
         (id)*VAR_METADATA_SIZE] = (int)(t);
    // Size
    heap[HEAP_SIZE - 1 - STATIC_METADATA_SIZE - (VAR_METADATA_SIZE - 2) -
         (id)*VAR_METADATA_SIZE] = 0;
    // Capacity
    heap[HEAP_SIZE - 1 - STATIC_METADATA_SIZE - (VAR_METADATA_SIZE - 1) -
         (id)*VAR_METADATA_SIZE] = 0;
    return id;
}

int main() {}
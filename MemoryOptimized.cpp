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

int malloc(int);
uint8_t* GetVectorCountAddress();
uint8_t* GetVectorMetadataAddress(int, VariablePropery);
int GetVectorPlaceByID(uint8_t);
uint8_t DefineVector(Type);
void DeleteVector(uint8_t);

int main() {}

// Metadata starts from end. Last byte is number of vars. Next five bytes are:
// capacity, size, type, start index, id
int malloc(int neededByteCount) {
    uint8_t vectorCount = *GetVectorCountAddress();
    int counter = 0;
    for (int i = 0; i < HEAP_SIZE - 1 - vectorCount * VAR_METADATA_SIZE; i++) {
        if (counter == neededByteCount) return i - neededByteCount;
        for (int j = 0; j < vectorCount; j++) {
            int currVarStart =
                *GetVectorMetadataAddress(j, VariablePropery::startIndex);
            int currVarCapacity =
                *GetVectorMetadataAddress(j, VariablePropery::capacity);
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

uint8_t* GetVectorCountAddress() { return &heap[HEAP_SIZE - 1]; }

uint8_t* GetVectorMetadataAddress(int variablePlace, VariablePropery property) {
    return &heap[HEAP_SIZE - 1 - STATIC_METADATA_SIZE -
                 (variablePlace * VAR_METADATA_SIZE) -
                 (VAR_METADATA_SIZE - (int)property)];
}

int GetVectorPlaceByID(uint8_t id) {
    for (int i = 0; i < *GetVectorCountAddress(); i++) {
        if (*GetVectorMetadataAddress(i, VariablePropery::id) == id) return i;
    }
    return -1;
}

// TODO: Get init values
uint8_t DefineVector(Type t) {
    uint8_t *vectorCount = GetVectorCountAddress(), id = -1;
    bool varID[*vectorCount] = {0};

    for (int i = 0; i < *vectorCount; i++) {
        if (*GetVectorMetadataAddress(i, VariablePropery::id) < *vectorCount)
            varID[*GetVectorMetadataAddress(i, VariablePropery::id)] = true;
    }
    for (int i = 0; i < *vectorCount; i++)
        if (!varID[i]) id = i;

    // Create variable
    *vectorCount++;
    // Start index
    *GetVectorMetadataAddress(*vectorCount, VariablePropery::startIndex) = -1;
    // Type
    *GetVectorMetadataAddress(*vectorCount, VariablePropery::type) = (int)(t);
    // Size
    *GetVectorMetadataAddress(*vectorCount, VariablePropery::size) = 0;
    // Capacity
    *GetVectorMetadataAddress(*vectorCount, VariablePropery::capacity) = 0;
    // ID
    *GetVectorMetadataAddress(*vectorCount, VariablePropery::id) = id;
    return id;
}

void DeleteVector(uint8_t id) {
    int place = GetVectorPlaceByID(id);
    if (place == -1) return;
    for (int i = place; i < *GetVectorCountAddress() - 1; i++) {
        *GetVectorMetadataAddress(i, VariablePropery::capacity)=*GetVectorMetadataAddress(i+1, VariablePropery::capacity);
        *GetVectorMetadataAddress(i, VariablePropery::id)=*GetVectorMetadataAddress(i+1, VariablePropery::id);
        *GetVectorMetadataAddress(i, VariablePropery::size)=*GetVectorMetadataAddress(i+1, VariablePropery::size);
        *GetVectorMetadataAddress(i, VariablePropery::startIndex)=*GetVectorMetadataAddress(i+1, VariablePropery::startIndex);
        *GetVectorMetadataAddress(i, VariablePropery::type)=*GetVectorMetadataAddress(i+1, VariablePropery::type);
    }
}

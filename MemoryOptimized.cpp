#include <cstdint>
#include <iostream>

using namespace std;

// #define HEAP_SIZE 1000000
#define HEAP_SIZE 1000
#define VAR_METADATA_SIZE 11
#define STATIC_METADATA_SIZE 1
#define VECTOR_COUNT_ADDR HEAP_SIZE-1

uint8_t heap[HEAP_SIZE];

enum class Type { Int = 1, Double };
enum class VariablePropery {
    capacity = 1,
    size = 4,
    type = 7,
    startIndex = 8,
    id = 11
};

int malloc(int);
uint8_t* GetVectorMetadataAddress(int, VariablePropery);
int GetVectorPlaceByID(uint8_t);
int DefineVector(Type);
void DeleteVector(uint8_t);
void VectorPushBack(uint8_t id, int value);
void VectorPushBack(uint8_t id, double value);

int main() {
    uint8_t intVector = DefineVector(Type::Int);
    uint8_t doubleVector = DefineVector(Type::Double);
    uint8_t int2Vector = DefineVector(Type::Int);
    DeleteVector(intVector);
}

// Metadata starts from end. Last byte is number of vars. Next five bytes are:
// capacity, size, type, start index, id
int malloc(int neededByteCount) {
    uint8_t vectorCount = heap[VECTOR_COUNT_ADDR];
    int counter = 0;
    for (int i = 0; i < HEAP_SIZE - 1 - vectorCount * VAR_METADATA_SIZE; i++) {
        if (counter == neededByteCount) return i - neededByteCount;
        for (int j = 0; j < vectorCount; j++) {
            int currVarStart =
                GetVectorMetadataAddress(j, VariablePropery::startIndex)[0]
                << 16 + GetVectorMetadataAddress(j,
                                                 VariablePropery::startIndex)[1]
                << 8 + GetVectorMetadataAddress(j,
                                                VariablePropery::startIndex)[2]
                << 0;
            int currVarCapacity =
                GetVectorMetadataAddress(j, VariablePropery::capacity)[0]
                << 16 +
                       GetVectorMetadataAddress(j, VariablePropery::capacity)[1]
                << 8 + GetVectorMetadataAddress(j, VariablePropery::capacity)[2]
                << 0;
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

uint8_t* GetVectorMetadataAddress(int variablePlace, VariablePropery property) {
    return &heap[HEAP_SIZE - 1 - STATIC_METADATA_SIZE -
                 (variablePlace * VAR_METADATA_SIZE) -
                 (VAR_METADATA_SIZE - (int)property)];
}

int GetVectorPlaceByID(uint8_t id) {
    for (int i = 0; i < heap[VECTOR_COUNT_ADDR]; i++) {
        if (*GetVectorMetadataAddress(i, VariablePropery::id) == id) return i;
    }
    return -1;
}

// TODO: Get init values
int DefineVector(Type t) {
    if (heap[VECTOR_COUNT_ADDR] == 255) {
        cout << "No room for other vectors!\n";
        return -1;
    }
    int id = -1;
    bool varID[heap[VECTOR_COUNT_ADDR]] = {0};

    for (int i = 0; i < heap[VECTOR_COUNT_ADDR]; i++) {
        if (*GetVectorMetadataAddress(i, VariablePropery::id) < heap[VECTOR_COUNT_ADDR])
            varID[*GetVectorMetadataAddress(i, VariablePropery::id)] = true;
    }
    for (int i = 0; i < heap[VECTOR_COUNT_ADDR]; i++)
        if (!varID[i]) id = i;
    if (id == -1) id = heap[VECTOR_COUNT_ADDR];

    // Create variable
    // Start index
    // *GetVectorMetadataAddress(*vectorCount, VariablePropery::startIndex) =
    // -1; Type
    *GetVectorMetadataAddress(heap[VECTOR_COUNT_ADDR], VariablePropery::type) = (int)(t);
    // Size
    GetVectorMetadataAddress(heap[VECTOR_COUNT_ADDR], VariablePropery::size)[0] =
        GetVectorMetadataAddress(heap[VECTOR_COUNT_ADDR], VariablePropery::size)[1] =
            GetVectorMetadataAddress(heap[VECTOR_COUNT_ADDR], VariablePropery::size)[2] =
                2;
    // Capacity
    GetVectorMetadataAddress(heap[VECTOR_COUNT_ADDR], VariablePropery::capacity)[0] =
        GetVectorMetadataAddress(heap[VECTOR_COUNT_ADDR], VariablePropery::capacity)[1] =
            GetVectorMetadataAddress(heap[VECTOR_COUNT_ADDR],
                                     VariablePropery::capacity)[2] = 10;
    // ID
    *GetVectorMetadataAddress(heap[VECTOR_COUNT_ADDR], VariablePropery::id) = id;
    heap[VECTOR_COUNT_ADDR]++;
    return id;
}

void DeleteVector(uint8_t id) {
    int place = GetVectorPlaceByID(id);
    if (place == -1) return;
    for (int i = place; i < heap[VECTOR_COUNT_ADDR] - 1; i++) {
        // Capacity
        GetVectorMetadataAddress(i, VariablePropery::capacity)[0] =
            GetVectorMetadataAddress(i + 1, VariablePropery::capacity)[0];
        GetVectorMetadataAddress(i, VariablePropery::capacity)[1] =
            GetVectorMetadataAddress(i + 1, VariablePropery::capacity)[1];
        GetVectorMetadataAddress(i, VariablePropery::capacity)[2] =
            GetVectorMetadataAddress(i + 1, VariablePropery::capacity)[2];
        // // ID
        *GetVectorMetadataAddress(i, VariablePropery::id) =
            *GetVectorMetadataAddress(i + 1, VariablePropery::id);
        // // Size
        GetVectorMetadataAddress(i, VariablePropery::size)[0] =
            GetVectorMetadataAddress(i + 1, VariablePropery::size)[0];
        GetVectorMetadataAddress(i, VariablePropery::size)[1] =
            GetVectorMetadataAddress(i + 1, VariablePropery::size)[1];
        GetVectorMetadataAddress(i, VariablePropery::size)[2] =
            GetVectorMetadataAddress(i + 1, VariablePropery::size)[2];
        // // Start index
        GetVectorMetadataAddress(i, VariablePropery::startIndex)[0] =
            GetVectorMetadataAddress(i + 1, VariablePropery::startIndex)[0];
        GetVectorMetadataAddress(i, VariablePropery::startIndex)[1] =
            GetVectorMetadataAddress(i + 1, VariablePropery::startIndex)[1];
        GetVectorMetadataAddress(i, VariablePropery::startIndex)[2] =
            GetVectorMetadataAddress(i + 1, VariablePropery::startIndex)[2];
        // // Type
        *GetVectorMetadataAddress(i, VariablePropery::type) =
            *GetVectorMetadataAddress(i + 1, VariablePropery::type);
    }
    heap[VECTOR_COUNT_ADDR]--;
}

void VectorPushBack(uint8_t id, int value) {}

void VectorPushBack(uint8_t id, double value) {}
#include <cstdint>
#include <iostream>

using namespace std;

// #define HEAP_SIZE 1000000
#define HEAP_SIZE 1000
#define VAR_METADATA_SIZE 14
#define STATIC_METADATA_SIZE 1
#define VECTOR_COUNT_ADDR HEAP_SIZE - 1

uint8_t heap[HEAP_SIZE];

enum class Type { Int = 1, Double = 2 };
enum class VariablePropery {
    id = 0,  // Incremented 4 times because capacity, size and startIndex
             // are 32bit integers.
    type = 1,
    capacity = 5,
    size = 9,
    startIndex = 13,
};

int malloc(int);
uint8_t* GetVectorMetadataAddress(int, VariablePropery);
int GetVectorPlaceByID(uint8_t);
int GetMultiMetadataValue(uint8_t, VariablePropery);
uint8_t GetSingleMetadataValue(uint8_t, VariablePropery);
void SetMetadataValue(uint8_t, VariablePropery, int);
void SetMetadataValueSingle(uint8_t, VariablePropery, uint8_t);
void SetMetadataValueMulti(uint8_t, VariablePropery, int);
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

uint8_t* GetVectorMetadataAddress(int variablePlace, VariablePropery property) {
    uint8_t* addr = &heap[HEAP_SIZE - 1 - STATIC_METADATA_SIZE -
                 (variablePlace * VAR_METADATA_SIZE) - ((int)property)];
    return addr;
}

int GetMultiMetadataValue(uint8_t variablePlace, VariablePropery property) {
    return ((GetVectorMetadataAddress(variablePlace, property)[0] << 24) +
            (GetVectorMetadataAddress(variablePlace, property)[1] << 16) +
            (GetVectorMetadataAddress(variablePlace, property)[2] << 8) +
            (GetVectorMetadataAddress(variablePlace, property)[3] << 0)
           );
}

uint8_t GetSingleMetadataValue(uint8_t variablePlace, VariablePropery property) {
    return (*GetVectorMetadataAddress(variablePlace, property));
}

void SetMetadataValueMulti(uint8_t variablePlace, VariablePropery property, int value) {
    // cout << ((value & 0xff000000) >> 24) << "\n";
    // cout << ((value & 0x00ff0000) >> 16) << "\n";
    // cout << ((value & 0x0000ff00) >> 8) << "\n";
    // cout << ((value & 0x000000ff) >> 0) << "\n###########\n";
    GetVectorMetadataAddress(variablePlace, property)[0] = (value & 0xff000000) >> 24;
    GetVectorMetadataAddress(variablePlace, property)[1] = (value & 0x00ff0000) >> 16;
    GetVectorMetadataAddress(variablePlace, property)[2] = (value & 0x0000ff00) >> 8;
    GetVectorMetadataAddress(variablePlace, property)[3] = (value & 0x000000ff) >> 0;
}

void SetMetadataValueSingle(uint8_t variablePlace, VariablePropery property, uint8_t value) {
    *GetVectorMetadataAddress(variablePlace, property) = value;
}

void SetMetadataValue(uint8_t variablePlace, VariablePropery property, int value) {
    switch (property) {
        case VariablePropery::capacity:
        case VariablePropery::startIndex:
        case VariablePropery::size:
            SetMetadataValueMulti(variablePlace, property, value);
            break;
        case VariablePropery::id:
        case VariablePropery::type:
            SetMetadataValueSingle(variablePlace, property, value);
        default:
            break;
    }
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
                GetMultiMetadataValue(j, VariablePropery::startIndex);
            int currVarCapacity =
                GetMultiMetadataValue(j, VariablePropery::capacity);

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

int GetVectorPlaceByID(uint8_t id) {
    for (int i = 0; i < heap[VECTOR_COUNT_ADDR]; i++) {
        if (GetSingleMetadataValue(i, VariablePropery::id) == id) return i;
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
    bool varID[heap[VECTOR_COUNT_ADDR]];

    for (int i = 0; i < heap[VECTOR_COUNT_ADDR]; i++) {
        uint8_t currentId = GetSingleMetadataValue(i, VariablePropery::id);
        if (currentId < heap[VECTOR_COUNT_ADDR]) varID[currentId] = true;
    }
    for (int i = 0; i < heap[VECTOR_COUNT_ADDR]; i++)
        if (!varID[i]) id = i;
    if (id == -1) {
        id = heap[VECTOR_COUNT_ADDR];
    }
    // Create variable
    // Start index
    SetMetadataValue(heap[VECTOR_COUNT_ADDR], VariablePropery::startIndex, -1);
    // Type
    SetMetadataValue(heap[VECTOR_COUNT_ADDR], VariablePropery::type, (int)(t));
    // Size
    SetMetadataValue(heap[VECTOR_COUNT_ADDR], VariablePropery::size, 0);
    // Capacity
    SetMetadataValue(heap[VECTOR_COUNT_ADDR], VariablePropery::capacity, 0);
    // ID
    SetMetadataValue(heap[VECTOR_COUNT_ADDR], VariablePropery::id, id);
    heap[VECTOR_COUNT_ADDR]++;
    return id;
}

void DeleteVector(uint8_t id) {
    int place = GetVectorPlaceByID(id);
    if (place == -1) return;
    for (int i = place; i < heap[VECTOR_COUNT_ADDR] - 1; i++) {
        // I know for multibyte metadatas manually assigning is faster but this
        // way is more readable and cleaner. Capacity        
        SetMetadataValue(
            i, VariablePropery::capacity,
            GetMultiMetadataValue(i + 1, VariablePropery::capacity));
        // // ID
        SetMetadataValue(i, VariablePropery::id,
                         GetSingleMetadataValue(i + 1, VariablePropery::id));
        // // Size
        SetMetadataValue(i, VariablePropery::size,
                         GetMultiMetadataValue(i + 1, VariablePropery::size));
        // // Start index
        SetMetadataValue(
            i, VariablePropery::startIndex,
            GetMultiMetadataValue(i + 1, VariablePropery::startIndex));
        // // Type
        SetMetadataValue(i, VariablePropery::type,
                         GetSingleMetadataValue(i + 1, VariablePropery::type));
    }
    heap[VECTOR_COUNT_ADDR]--;
}

void VectorPushBack(uint8_t id, int value) {
    
}

void VectorPushBack(uint8_t id, double value) {}
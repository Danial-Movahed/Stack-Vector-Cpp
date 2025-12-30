#include <cstdint>
#include <iostream>

using namespace std;

// #define HEAP_SIZE 1000000
#define HEAP_SIZE 1000
#define VAR_METADATA_SIZE 14
#define STATIC_METADATA_SIZE 1
#define VECTOR_COUNT_ADDR HEAP_SIZE - 1

uint8_t heap[HEAP_SIZE];

enum class Type { Int = sizeof(int), Double = sizeof(double) };
enum class VariablePropery {
    id = 0,  // Incremented 4 times because capacity, size and startIndex
             // are 32bit integers.
    type = 1,
    capacity = 5,
    size = 9,
    startIndex = 13,
};
// First I made these functions return pointer but I thought that because our heap is defined as an array then address
// of each value will become its index so it makes more sense to use index instead of pointers.
// So derefrencing becomes returning heap[index]
int _GetVectorMetadataIndex(uint8_t, VariablePropery);
uint8_t _GetSingleMetadataValue(uint8_t, VariablePropery);
int _GetMultiMetadataValue(uint8_t, VariablePropery);
void _SetMetadataValueSingle(uint8_t, VariablePropery, uint8_t);

int myMalloc(int, int16_t = -1);
void myMemcpy(int, int, int);
void SetMultiValue(double, int);
void SetMultiValue(int, int);
uint8_t GetVectorPlace(uint8_t);
int GetMetadataValue(uint8_t, VariablePropery);
void SetMetadataValue(uint8_t, VariablePropery, int);
int DefineVector(Type);
void DeleteVector(uint8_t);
void VectorPushBack(uint8_t, int);
void VectorPushBack(uint8_t, double);
void VectorPopBack(uint8_t);

int main() {
    uint8_t intVector = DefineVector(Type::Int);
    uint8_t doubleVector = DefineVector(Type::Double);
    // uint8_t int2Vector = DefineVector(Type::Int);
    // DeleteVector(intVector);
    VectorPushBack(intVector, 12);
    VectorPushBack(intVector, 2);
    VectorPushBack(intVector, 15);
    VectorPushBack(intVector, 100);
    VectorPushBack(intVector, 9);
    VectorPushBack(intVector, 3);
    VectorPushBack(intVector, 1);

    VectorPushBack(doubleVector, 1.1);
    VectorPushBack(doubleVector, 3.2);
    VectorPushBack(doubleVector, 4.5);
    VectorPushBack(doubleVector, 9.1);
    VectorPushBack(doubleVector, 100.0);
    VectorPushBack(doubleVector, 123.2);
    VectorPushBack(doubleVector, 1.6);

    VectorPopBack(intVector);
    VectorPopBack(doubleVector);
    return 0;
}

// Typically these functions should not be used
// Metadata starts from end.
int _GetVectorMetadataIndex(uint8_t vectorPlace, VariablePropery property) {
    return (HEAP_SIZE - 1 - STATIC_METADATA_SIZE -
                          (vectorPlace * VAR_METADATA_SIZE) - ((int)property));
}

uint8_t _GetSingleMetadataValue(uint8_t vectorPlace, VariablePropery property) {
    return (heap[_GetVectorMetadataIndex(vectorPlace, property)]);
}
int _GetMultiMetadataValue(uint8_t vectorPlace, VariablePropery property) {
    int metadataIndex = _GetVectorMetadataIndex(vectorPlace, property);
    return ((heap[metadataIndex+3] << 24) +
            (heap[metadataIndex+2] << 16) +
            (heap[metadataIndex+1] << 8) +
            (heap[metadataIndex+0] << 0));
}

void _SetMetadataValueSingle(uint8_t vectorPlace, VariablePropery property, uint8_t value) {
    heap[_GetVectorMetadataIndex(vectorPlace, property)] = value;
}


// These functions can be used instead
int myMalloc(int neededByteCount, int16_t variablePlace) {
    uint8_t vectorCount = heap[VECTOR_COUNT_ADDR];
    int counter = 0;
    for (int i = 0;
         i < HEAP_SIZE - STATIC_METADATA_SIZE - vectorCount * VAR_METADATA_SIZE; i++) {
        if (counter == neededByteCount) return i - neededByteCount;
        for (int j = 0; j < vectorCount; j++) {
            if (j == variablePlace) continue;
            int currVarStart = GetMetadataValue(j, VariablePropery::startIndex),
                currVarCapacity = GetMetadataValue(j, VariablePropery::capacity),
                currVarType = GetMetadataValue(j, VariablePropery::type);
            if(currVarStart == -1)
                continue;
            if (i >= currVarStart && i<currVarStart+(currVarCapacity*currVarType)) {
                counter = 0;
                // Minus one becase after continue i gets automatically increamented by one by for loop
                i += (currVarCapacity*currVarType) - 1;
                goto collision;
            }
        }
        counter++;
    collision:
        continue;
    }
    return -1;
}
void myMemcpy(int from, int to, int size) {
    for (int i = 0; i < size; i++) {
        heap[to + i] = heap[from + i];
    }
}

void SetMultiValue(int value, int startIndex) {
    uint8_t* ptr = (uint8_t*)(&value);
    for (int i = 0; i < sizeof(int); i++) heap[startIndex + i] = ptr[i];
}
void SetMultiValue(double value, int startIndex) {
    uint8_t* ptr = (uint8_t*)(&value);
    for (int i = 0; i < sizeof(double); i++) heap[startIndex + i] = ptr[i];
}

uint8_t GetVectorPlace(uint8_t id) {
    for (int i = 0; i < heap[VECTOR_COUNT_ADDR]; i++) {
        if (GetMetadataValue(i, VariablePropery::id) == id) return i;
    }
    return -1;
}
int GetMetadataValue(uint8_t vectorPlace, VariablePropery property) {
    switch (property) {
        case VariablePropery::capacity:
        case VariablePropery::startIndex:
        case VariablePropery::size:
            return _GetMultiMetadataValue(vectorPlace, property);
            break;
        case VariablePropery::id:
        case VariablePropery::type:
            return _GetSingleMetadataValue(vectorPlace, property);
        default:
            break;
    }
    return -1;
}
void SetMetadataValue(uint8_t vectorPlace, VariablePropery property, int value) {
    switch (property) {
        case VariablePropery::capacity:
        case VariablePropery::startIndex:
        case VariablePropery::size:
            SetMultiValue(value,
                          _GetVectorMetadataIndex(vectorPlace, property));
            break;
        case VariablePropery::id:
        case VariablePropery::type:
            _SetMetadataValueSingle(vectorPlace, property, value);
        default:
            break;
    }
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
        uint8_t currentId = GetMetadataValue(i, VariablePropery::id);
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
    int place = GetVectorPlace(id);
    if (place == -1) return;
    for (int i = place; i < heap[VECTOR_COUNT_ADDR] - 1; i++) {
        // I know for multibyte metadatas manually assigning is faster but this
        // way is more readable and cleaner. Capacity
        SetMetadataValue(
            i, VariablePropery::capacity,
            GetMetadataValue(i + 1, VariablePropery::capacity));
        // // ID
        SetMetadataValue(i, VariablePropery::id,
                         GetMetadataValue(i + 1, VariablePropery::id));
        // // Size
        SetMetadataValue(i, VariablePropery::size,
                         GetMetadataValue(i + 1, VariablePropery::size));
        // // Start index
        SetMetadataValue(
            i, VariablePropery::startIndex,
            GetMetadataValue(i + 1, VariablePropery::startIndex));
        // // Type
        SetMetadataValue(i, VariablePropery::type,
                         GetMetadataValue(i + 1, VariablePropery::type));
    }
    heap[VECTOR_COUNT_ADDR]--;
}

void VectorPushBack(uint8_t id, int value) {
    uint8_t vectorPlace = GetVectorPlace(id);
    int vectorSize = GetMetadataValue(vectorPlace, VariablePropery::size),
        vectorCapacity = GetMetadataValue(vectorPlace, VariablePropery::capacity),
        vectorStart = GetMetadataValue(vectorPlace, VariablePropery::startIndex);

    if (vectorSize >= vectorCapacity) {
        printf("No more capacity available! start index: %d capacity: %d size: %d Trying to malloc...\n", vectorStart, vectorCapacity, vectorSize);
        int newVectorCapacity = vectorCapacity * 1.5 + 1;
        int newVectorStart = myMalloc(newVectorCapacity*sizeof(int), vectorPlace);
        if (newVectorStart == -1) {
            cout << "No more memory available!\n";
            return;
        }
        printf("malloc succeded! new start index: %d new capacity: %d\n",newVectorStart, newVectorCapacity);
        SetMetadataValue(vectorPlace, VariablePropery::capacity, newVectorCapacity);
        SetMetadataValue(vectorPlace, VariablePropery::startIndex, newVectorStart);
        if(vectorStart>0)
            myMemcpy(vectorStart, newVectorStart, vectorSize*sizeof(int));
        vectorStart = newVectorStart;
    }
    SetMultiValue(value, vectorStart+(vectorSize*sizeof(int)));
    SetMetadataValue(vectorPlace, VariablePropery::size, vectorSize+1);
}
void VectorPushBack(uint8_t id, double value) {
    uint8_t vectorPlace = GetVectorPlace(id);
    int vectorSize = GetMetadataValue(vectorPlace, VariablePropery::size),
        vectorCapacity = GetMetadataValue(vectorPlace, VariablePropery::capacity),
        vectorStart = GetMetadataValue(vectorPlace, VariablePropery::startIndex);

    if (vectorSize >= vectorCapacity) {
        printf("No more capacity available! start index: %d capacity: %d size: %d Trying to malloc...\n", vectorStart, vectorCapacity, vectorSize);
        int newVectorCapacity = vectorCapacity * 1.5 + 1;
        int newVectorStart = myMalloc(newVectorCapacity*sizeof(double), vectorPlace);
        if (newVectorStart == -1) {
            cout << "No more memory available!\n";
            return;
        }
        printf("malloc succeded! new start index: %d new capacity: %d\n",newVectorStart, newVectorCapacity);
        SetMetadataValue(vectorPlace, VariablePropery::capacity, newVectorCapacity);
        SetMetadataValue(vectorPlace, VariablePropery::startIndex, newVectorStart);
        if(vectorStart>0)
            myMemcpy(vectorStart, newVectorStart, vectorSize*sizeof(double));
        vectorStart = newVectorStart;
    }
    SetMultiValue(value, vectorStart+(vectorSize*sizeof(double)));
    SetMetadataValue(vectorPlace, VariablePropery::size, vectorSize+1);
}

void VectorPopBack(uint8_t id) {
    uint8_t vectorPlace = GetVectorPlace(id);
    int vectorSize = GetMetadataValue(vectorPlace, VariablePropery::size);
    if(vectorSize < 1)
        return;
    SetMetadataValue(vectorPlace, VariablePropery::size, vectorSize-1);
}
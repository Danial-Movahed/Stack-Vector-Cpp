#include <cstdint>
#include <iostream>

using namespace std;

// Options!
// Use c++ builtin casting using pointer cast and derefrencing
#define USE_POINTER_CAST_GETTER
#define USE_POINTER_CAST_SETTER
// Use pointer arithmetic instead of manually using *sizeof(type)
#define USE_POINTER_ARITHMETIC

// Static values!
#define HEAP_SIZE 1000
// #define HEAP_SIZE 1000000
#define VAR_METADATA_SIZE 14
#define STATIC_METADATA_SIZE 1
#define VECTOR_COUNT_ADDR HEAP_SIZE - 1
#define EINVALID 1
#define ENOVARIABLE 255
#define MAX_VAR_CNT 255 // 0-254 which counts to 255 because number 255 is reserved for no variable error

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
// Typically these functions should not be used
// Metadata starts from end.
int _GetVectorMetadataIndex(uint8_t, VariablePropery);
uint8_t _GetSingleMetadataValue(uint8_t, VariablePropery);
int _GetMultiMetadataValue(uint8_t, VariablePropery);
void _SetMetadataValueSingle(uint8_t, VariablePropery, uint8_t);

// These functions can be used instead
int myMalloc(int, uint8_t = ENOVARIABLE);
void myMemcpy(int, int, int);
void SetMultiValue(int, int);
void SetMultiValue(double, int);
int GetMultiIntValue(int);
double GetMultiDoubleValue(int);
uint8_t GetVectorPlace(uint8_t);
int GetMetadataValue(uint8_t, VariablePropery);
void SetMetadataValue(uint8_t, VariablePropery, int);
uint8_t VectorDefine(Type);
void VectorDelete(uint8_t);
void VectorPushBack(uint8_t, int);
void VectorPushBack(uint8_t, double);
void VectorPopBack(uint8_t);
int VectorIntAt(uint8_t, int);
double VectorDoubleAt(uint8_t, int);
int VectorSize(uint8_t);
int* VectorIntData(uint8_t);
double* VectorDoubleData(uint8_t);
void VectorClear(uint8_t);
void VectorFree(uint8_t);
void VectorResize(uint8_t, int);
void VectorReserve(uint8_t, int);
void VectorShrink2Fit(uint8_t);

int main() {
    uint8_t intVector = VectorDefine(Type::Int);
    uint8_t doubleVector = VectorDefine(Type::Double);
    // uint8_t int2Vector = DefineVector(Type::Int);
    // VectorDelete(intVector);
    // VectorReserve(doubleVector, 10);
    // VectorReserve(intVector, 10);

    VectorPushBack(intVector, 12);
    VectorPushBack(intVector, 2);
    VectorPushBack(intVector, 15);
    VectorPushBack(intVector, 100);
    VectorPushBack(intVector, 99999999);
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

    cout<<VectorIntAt(intVector, 4)<<"\n";
    cout<<VectorIntAt(intVector, 5)<<"\n";
    cout<<VectorDoubleAt(doubleVector, 1)<<"\n";

    int* intVectorData = VectorIntData(intVector);
    cout<<*(intVectorData+1)<<"\n";
    double* doubleVectorData = VectorDoubleData(doubleVector);
    cout<<*(doubleVectorData+1)<<"\n";

    // for(int i=0; i<255; i++) {
    //     // cout<<i<<endl;
    //     cout<<(int)VectorDefine(Type::Int)<<"\n";
    // }
    return 0;
}

int _GetVectorMetadataIndex(uint8_t vectorPlace, VariablePropery property) {
    return (HEAP_SIZE - 1 - STATIC_METADATA_SIZE -
                          (vectorPlace * VAR_METADATA_SIZE) - ((int)property));
}
uint8_t _GetSingleMetadataValue(uint8_t vectorPlace, VariablePropery property) {
    return (heap[_GetVectorMetadataIndex(vectorPlace, property)]);
}
int _GetMultiMetadataValue(uint8_t vectorPlace, VariablePropery property) {
    int metadataIndex = _GetVectorMetadataIndex(vectorPlace, property);
    return GetMultiIntValue(metadataIndex);
}
void _SetMetadataValueSingle(uint8_t vectorPlace, VariablePropery property, uint8_t value) {
    heap[_GetVectorMetadataIndex(vectorPlace, property)] = value;
}

int myMalloc(int neededByteCount, uint8_t variablePlace) {
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
    return -EINVALID;
}
void myMemcpy(int from, int to, int size) {
    for (int i = 0; i < size; i++) {
        heap[to + i] = heap[from + i];
    }
}

void SetMultiValue(int value, int startIndex) {
    #ifdef USE_POINTER_CAST_SETTER
    int* ptr = (int*)(&heap[startIndex]);
    *ptr=value;
    #else
    uint8_t* ptr = (uint8_t*)(&value);
    for (int i = 0; i < sizeof(int); i++) heap[startIndex + i] = ptr[i];
    #endif
}
void SetMultiValue(double value, int startIndex) {
    #ifdef USE_POINTER_CAST_SETTER
    double* ptr = (double*)(&heap[startIndex]);
    *ptr=value;
    #else
    uint8_t* ptr = (uint8_t*)(&value);
    for (int i = 0; i < sizeof(double); i++) heap[startIndex + i] = ptr[i];
    #endif
}
int GetMultiIntValue(int startIndex) {
    #ifdef USE_POINTER_CAST_GETTER
    return *((int*)(&heap[startIndex]));
    #else
    return ((heap[startIndex+3] << 24) +
            (heap[startIndex+2] << 16) +
            (heap[startIndex+1] << 8)  +
            (heap[startIndex+0] << 0));
    #endif
}
double GetMultiDoubleValue(int startIndex) {
    #ifdef USE_POINTER_CAST_GETTER
    return *((double*)(&heap[startIndex]));
    #else
    double out = (heap[startIndex+7] << 56) +
                 (heap[startIndex+6] << 48) +
                 (heap[startIndex+5] << 40) +
                 (heap[startIndex+4] << 32) +
                 (heap[startIndex+3] << 24) +
                 (heap[startIndex+2] << 16) +
                 (heap[startIndex+1] << 8)  +
                 (heap[startIndex+0] << 0);
    return out;
    #endif
}

uint8_t GetVectorPlace(uint8_t id) {
    for (int i = 0; i < heap[VECTOR_COUNT_ADDR]; i++) {
        if (GetMetadataValue(i, VariablePropery::id) == id) return i;
    }
    return ENOVARIABLE;
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
    return -EINVALID;
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
            if(value>255) {
                cout<<"Metadata set value overflow detected! Aborting metadata set value, expect bugs!\n";
                return;
            }
            _SetMetadataValueSingle(vectorPlace, property, value);
        default:
            break;
    }
}

// TODO: Get init values
uint8_t VectorDefine(Type t) {
    if (heap[VECTOR_COUNT_ADDR] == MAX_VAR_CNT) {
        cout << "No room for other vectors!\n";
        return ENOVARIABLE;
    }
    uint8_t id = ENOVARIABLE;
    bool varID[heap[VECTOR_COUNT_ADDR]];

    for (int i = 0; i < heap[VECTOR_COUNT_ADDR]; i++) {
        uint8_t currentId = GetMetadataValue(i, VariablePropery::id);
        if (currentId < heap[VECTOR_COUNT_ADDR]) varID[currentId] = true;
    }
    for (int i = 0; i < heap[VECTOR_COUNT_ADDR]; i++)
        if (!varID[i]) id = i;
    if (id == ENOVARIABLE) {
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
void VectorDelete(uint8_t id) {
    uint8_t vectorPlace = GetVectorPlace(id);
    if (vectorPlace == ENOVARIABLE) {
        cout<<"Variable doesn't exist!\n";
        return;
    }
    for (int i = vectorPlace; i < heap[VECTOR_COUNT_ADDR] - 1; i++) {
        // Capacity
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
    if (vectorPlace == ENOVARIABLE) {
        cout<<"Variable doesn't exist!\n";
        return;
    }
    int vectorSize = GetMetadataValue(vectorPlace, VariablePropery::size),
        vectorCapacity = GetMetadataValue(vectorPlace, VariablePropery::capacity);

    if (vectorSize >= vectorCapacity) {
        printf("No more capacity available! capacity: %d size: %d Trying to malloc...\n", vectorCapacity, vectorSize);
        VectorResize(vectorPlace, vectorCapacity * 1.5 + 1);
    }
    int vectorStart = GetMetadataValue(vectorPlace, VariablePropery::startIndex);
    SetMultiValue(value, vectorStart+(vectorSize*sizeof(int)));
    SetMetadataValue(vectorPlace, VariablePropery::size, vectorSize+1);
}
void VectorPushBack(uint8_t id, double value) {
    uint8_t vectorPlace = GetVectorPlace(id);
    if (vectorPlace == ENOVARIABLE) {
        cout<<"Variable doesn't exist!\n";
        return;
    }
    int vectorSize = GetMetadataValue(vectorPlace, VariablePropery::size),
        vectorCapacity = GetMetadataValue(vectorPlace, VariablePropery::capacity);

    if (vectorSize >= vectorCapacity) {
        printf("No more capacity available! capacity: %d size: %d Trying to malloc...\n", vectorCapacity, vectorSize);
        VectorResize(vectorPlace, vectorCapacity * 1.5 + 1);
    }
    int vectorStart = GetMetadataValue(vectorPlace, VariablePropery::startIndex);
    SetMultiValue(value, vectorStart+(vectorSize*sizeof(double)));
    SetMetadataValue(vectorPlace, VariablePropery::size, vectorSize+1);
}

void VectorPopBack(uint8_t id) {
    uint8_t vectorPlace = GetVectorPlace(id);
    if (vectorPlace == ENOVARIABLE) {
        cout<<"Variable doesn't exist!\n";
        return;
    }
    int vectorSize = GetMetadataValue(vectorPlace, VariablePropery::size);
    if(vectorSize < 1)
        return;
    SetMetadataValue(vectorPlace, VariablePropery::size, vectorSize-1);
}

int VectorIntAt(uint8_t id, int index) {
    uint8_t vectorPlace = GetVectorPlace(id);
    if (vectorPlace == ENOVARIABLE) {
        cout<<"Variable doesn't exist!\n";
        return -EINVALID;
    }
    int vectorSize = GetMetadataValue(vectorPlace, VariablePropery::size);
    if(index>=vectorSize) {
        cout<<"Out of bounds read!\n";
        return -EINVALID;
    }
    int vectorStart = GetMetadataValue(vectorPlace, VariablePropery::startIndex);
    return GetMultiIntValue(vectorStart+index*sizeof(int));
}
double VectorDoubleAt(uint8_t id, int index) {
    uint8_t vectorPlace = GetVectorPlace(id);
    if (vectorPlace == ENOVARIABLE) {
        cout<<"Variable doesn't exist!\n";
        return -EINVALID;
    }
    int vectorSize = GetMetadataValue(vectorPlace, VariablePropery::size);
    if(index>=vectorSize) {
        cout<<"Out of bounds read!\n";
        return -EINVALID;
    }
    int vectorStart = GetMetadataValue(vectorPlace, VariablePropery::startIndex);
    return GetMultiDoubleValue(vectorStart+index*sizeof(double));
}

int VectorSize(uint8_t id) {
    uint8_t vectorPlace = GetVectorPlace(id);
    if (vectorPlace == ENOVARIABLE) {
        cout<<"Variable doesn't exist!\n";
        return -EINVALID;
    }
    int vectorSize = GetMetadataValue(vectorPlace, VariablePropery::size);
    return vectorSize;
}

int* VectorIntData(uint8_t id) {
    uint8_t vectorPlace = GetVectorPlace(id);
    if (vectorPlace == ENOVARIABLE) {
        cout<<"Variable doesn't exist!\n";
        return nullptr;
    }
    int vectorStart = GetMetadataValue(vectorPlace, VariablePropery::startIndex);
    return (int*)(&heap[vectorStart]);
}
double* VectorDoubleData(uint8_t id) {
    uint8_t vectorPlace = GetVectorPlace(id);
    if (vectorPlace == ENOVARIABLE) {
        cout<<"Variable doesn't exist!\n";
        return nullptr;
    }
    int vectorStart = GetMetadataValue(vectorPlace, VariablePropery::startIndex);
    return (double*)(&heap[vectorStart]);
}

void VectorClear(uint8_t id) {
    uint8_t vectorPlace = GetVectorPlace(id);
    if (vectorPlace == ENOVARIABLE) {
        cout<<"Variable doesn't exist!\n";
        return;
    }
    SetMetadataValue(vectorPlace, VariablePropery::size, 0);
}

void VectorFree(uint8_t id) {
    uint8_t vectorPlace = GetVectorPlace(id);
    if (vectorPlace == ENOVARIABLE) {
        cout<<"Variable doesn't exist!\n";
        return;
    }
    SetMetadataValue(vectorPlace, VariablePropery::size, 0);
    SetMetadataValue(vectorPlace, VariablePropery::capacity, 0);
    SetMetadataValue(vectorPlace, VariablePropery::startIndex, -1);
}

void VectorResize(uint8_t vectorPlace, int newCapacity) {
    int vectorType = GetMetadataValue(vectorPlace, VariablePropery::type),
        vectorStart = GetMetadataValue(vectorPlace, VariablePropery::startIndex),
        vectorSize = GetMetadataValue(vectorPlace, VariablePropery::size);
    if (newCapacity < vectorSize) {
        cout<<"Error: Not resizing less than vector size and this should not happen!! Data loss might occur! If you really want this use pop_back() instead and then shrink2fit()\n";
        return;
    }
    int newVectorStart = myMalloc(newCapacity*vectorType, vectorPlace);
    if (newVectorStart == -EINVALID) {
        cout << "No more memory available!\n";
        return;
    }
    printf("malloc succeded! new start index: %d new capacity: %d\n",newVectorStart, newCapacity);
    SetMetadataValue(vectorPlace, VariablePropery::capacity, newCapacity);
    SetMetadataValue(vectorPlace, VariablePropery::startIndex, newVectorStart);
    if(vectorStart>0 && newVectorStart!=vectorStart)
        myMemcpy(vectorStart, newVectorStart, vectorSize*sizeof(int));
}

void VectorReserve(uint8_t id, int size2Reserve) {
    uint8_t vectorPlace = GetVectorPlace(id);
    if (vectorPlace == ENOVARIABLE) {
        cout<<"Variable doesn't exist!\n";
        return;
    }
    int newVectorCapacity = GetMetadataValue(vectorPlace, VariablePropery::capacity) + size2Reserve;
    VectorResize(vectorPlace, newVectorCapacity);
}

void VectorShrink2Fit(uint8_t id) {
    uint8_t vectorPlace = GetVectorPlace(id);
    if (vectorPlace == ENOVARIABLE) {
        cout<<"Variable doesn't exist!\n";
        return;
    }
    int newVectorCapacity = GetMetadataValue(vectorPlace, VariablePropery::size);
    VectorResize(vectorPlace, newVectorCapacity);
}

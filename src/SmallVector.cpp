#include "SmallVector.h"
#include <cstdint>

SC_NAMESPACE_BEGIN

namespace {
    struct Struct16B {
        uint16_t t;
    };

    struct Struct32B
    {
        uint32_t t;
    };
}

static_assert(alignof(SmallVector<Struct16B, 0>) >= alignof(Struct16B),
    "wrong alignment for 16-byte aligned T");
static_assert(alignof(SmallVector<Struct32B, 0>) >= alignof(Struct32B),
    "wrong alignment for 32-byte aligned T");
static_assert(sizeof(SmallVector<Struct16B, 0>) >= alignof(Struct16B),
    "missing padding for 16-byte aligned T");
static_assert(sizeof(SmallVector<Struct32B, 0>) >= alignof(Struct32B),
    "missing padding for 32-byte aligned T");

// Note: Moving this function into the header may cause performance regression.
static SC_SIZE_TYPE getNewCapacity(SC_SIZE_TYPE MinSize, SC_SIZE_TYPE TSize, SC_SIZE_TYPE OldCapacity) {
    constexpr SC_SIZE_TYPE MaxSize = std::numeric_limits<SC_SIZE_TYPE>::max();

    SC_ASSERT(MaxSize >= MinSize);
    SC_ASSERT(OldCapacity != MaxSize);

    // Overflow
    SC_ASSERT(SC_SIZE_TYPE(double(MaxSize - 1) / SC_CAPACITY_STEP) >= OldCapacity);

    SC_SIZE_TYPE NewCapacity = SC_CAPACITY_STEP * OldCapacity + 1; // Always grow.
    return std::min(std::max(NewCapacity, MinSize), MaxSize);
}

// Note: Moving this function into the header may cause performance regression.
void SmallVectorBase::grow_pod(void* FirstEl, SC_SIZE_TYPE MinSize, SC_SIZE_TYPE TSize) 
{
    SC_SIZE_TYPE NewCapacity = getNewCapacity(MinSize, TSize, this->capacity());
    void* NewElts;
    if (BeginX == FirstEl) {
        NewElts = malloc(NewCapacity * TSize);

        // Copy the elements over.  No need to run dtors on PODs.
        memcpy(NewElts, this->BeginX, size() * TSize);
    }
    else {
        // If this wasn't grown from the inline copy, grow the allocated space.
        NewElts = realloc(this->BeginX, NewCapacity * TSize);
    }

    this->BeginX = NewElts;
    this->Capacity = NewCapacity;
}

SC_NAMESPACE_END
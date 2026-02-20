#pragma once
#include <cstddef>
#include <cstdint>
//this is temporary
class ItemStackBase;

struct ItemStackBaseOpaque {
    alignas(16) std::byte data[0x800]; // a very arbitrary value, more bytes here mean less freezes in game inv 
};

static inline ItemStackBase* asISB(ItemStackBaseOpaque& o) {
    return reinterpret_cast<ItemStackBase*>(o.data);
}

#define SHULKER_CACHE_SIZE 16
#define SHULKER_SLOT_COUNT 27

struct ShulkerSlotCache {
    ItemStackBaseOpaque isb; // itemcache
    uint8_t count;           // stable stack size
    bool valid;
    bool enchanted;
};

extern ShulkerSlotCache ShulkerCache[SHULKER_CACHE_SIZE][SHULKER_SLOT_COUNT];

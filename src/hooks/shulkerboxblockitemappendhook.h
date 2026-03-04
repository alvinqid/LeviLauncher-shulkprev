#pragma once
#include "item/item.h"
#include "shulkerenderer/colors.h"
#include "item/itemstackbase.h"
#include "util/scache.h"
#include "util/config.h"
#include "util/modmenu.h"
#include "nbt/compoundtag.h"
#include "nbt/listtag.h"
#include <string>

class ShulkerBoxBlockItem;

using Shulker_appendHover_t = void (*)(void*, ItemStackBase*, void*, std::string&, bool);

inline Shulker_appendHover_t ShulkerBoxBlockItem_appendFormattedHovertext_orig = nullptr;

namespace {
template <std::size_t N>
inline bool containsTag(void* compound, const char (&key)[N]) {
    return CompoundTag_contains(compound, key, N - 1);
}

inline bool hasEnchantmentData(void* compound) {
    if (!compound || !CompoundTag_contains)
        return false;

    return containsTag(compound, "ench")
        || containsTag(compound, "Enchantments")
        || containsTag(compound, "StoredEnchantments")
        || containsTag(compound, "minecraft:enchantments")
        || containsTag(compound, "minecraft:stored_enchantments");
}
} // namespace

inline void ShulkerBoxBlockItem_appendFormattedHovertext_hook(
    ShulkerBoxBlockItem* self,
    ItemStackBase* stack,
    void* level,
    std::string& out,
    bool flag)
{
    if (ShulkerBoxBlockItem_appendFormattedHovertext_orig)
        ShulkerBoxBlockItem_appendFormattedHovertext_orig(
            self, stack, level, out, flag);

    if (auto pos = out.find('\n'); pos != std::string::npos)
        out.erase(pos);

    if (!stack || !stack->mUserData)
        return;

    if (!CompoundTag_getList ||
        !ListTag_get ||
        !ListTag_size ||
        !ItemStackBase_loadItem ||
        !CompoundTag_getByte)
        return;

    int index = (reinterpret_cast<uintptr_t>(stack) >> 4) & (SHULKER_CACHE_SIZE - 1);

    for (int i = 0; i < SHULKER_SLOT_COUNT; ++i) {
        ShulkerCache[index][i].valid = false;
        ShulkerCache[index][i].enchanted = false;
        ShulkerCache[index][i].count = 0;
    }

    void* list = CompoundTag_getList(stack->mUserData, "Items", 5);
    if (!list)
        return;

    int size = ListTag_size(list);

    for (int i = 0; i < size; ++i) {
        void* tag = ListTag_get(list, i);
        if (!tag)
            continue;

        uint8_t slot  = CompoundTag_getByte(tag, "Slot", 4);
        uint8_t count = CompoundTag_getByte(tag, "Count", 5);

        if (slot >= SHULKER_SLOT_COUNT)
            continue;

        ShulkerSlotCache& sc = ShulkerCache[index][slot];
        ItemStackBase_loadItem(asISB(sc.isb), tag);
        sc.count = count;
        ItemStackBase* loaded = asISB(sc.isb);
        sc.enchanted = hasEnchantmentData(tag)
            || (loaded && loaded->mUserData && hasEnchantmentData(loaded->mUserData));
        sc.valid = true;
    }

    static const char hex[] = "0123456789abcdef";
    out.insert(0, "\xC2\xA7");
    out.insert(2, 1, hex[(index >> 4) & 0xF]);
    out.insert(3, "\xC2\xA7");
    out.insert(5, 1, hex[index & 0xF]);

    if (!ItemStackBase_getItem || !Item_getId)
        return;

    void* rootItem = ItemStackBase_getItem(stack);
    if (!rootItem)
        return;

    char color = getShulkerColorCodeFromItemId(Item_getId(rootItem));

    out += "\n§7Press §e";
    out += SP_keyCodeToString(spPreviewKey);
    out += "§7 to toggle preview";
    out += "\xC2\xA7#";
    out.push_back(color);
    out += "\xC2\xA7v";
}

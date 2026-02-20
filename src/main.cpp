#include <dlfcn.h>
#include <link.h>
#include <span>
#include <string>
#include "main.h"
#include <utility>
#include <optional>
#include <safetyhook.hpp>
#include <libhat.hpp>
#include <libhat/scanner.hpp>

Item_getId_t Item_getId = nullptr;
BaseActorRenderContext_ctor_t BaseActorRenderContext_ctor = nullptr;
ItemRenderer_renderGuiItemNew_t ItemRenderer_renderGuiItemNew = nullptr;

extern "C" [[gnu::visibility("default")]] void mod_preinit() {}
extern "C" [[gnu::visibility("default")]] void mod_init()
{
    using namespace hat::literals::signature_literals;
    void *mcLib = dlopen("libminecraftpe.so", 0);
    if (!mcLib)
    {
        printf("[ShulkerPreview] failed to open libminecraftpe.so\n");
        return;
    }

    std::span<std::byte> range1, range2;
    auto callback = [&](const dl_phdr_info &info)
    {
        if (auto h = dlopen(info.dlpi_name, RTLD_NOLOAD); dlclose(h), h != mcLib)
            return 0;
        range1 = {reinterpret_cast<std::byte *>(info.dlpi_addr + info.dlpi_phdr[1].p_vaddr), info.dlpi_phdr[1].p_memsz};
        range2 = {reinterpret_cast<std::byte *>(info.dlpi_addr + info.dlpi_phdr[2].p_vaddr), info.dlpi_phdr[2].p_memsz};
        return 1;
    };
    dl_iterate_phdr([](dl_phdr_info *info, size_t, void *data)
                    { return (*static_cast<decltype(callback) *>(data))(*info); }, &callback);

    auto scan = [range1](const auto&... sig) {
        void* addr;
        ((addr = hat::find_pattern(range1, sig, hat::scan_alignment::X16).get()) || ...);
        return addr;
    };
    SP_register_keybinds();

    // stoff, add wildcards later this is for test 
    auto CtagGetListaddr = scan(
    "53 48 83 EC 20 48 89 FB 64 48 8B 04 25 28 00 00 00 48 89 44 24 ?? 48 89 74 24 ?? 48 89 54 24 ?? 48 83 C7 08 48 8D 74 24 ?? E8 A2 D1 01 00 48 83"_sig
    );
    CompoundTag_getList =
    reinterpret_cast<CompoundTag_getList_t>(CtagGetListaddr);

    auto CtagContainsaddr = scan(
    "53 48 83 EC 30 48 89 FB 64 48 8B 04 25 28 00 00 00 48 89 44 24 ?? 48 89 74 24 ?? 48 89 54 24 ?? 48 83 C7 08 48 8D 74 24 ?? E8 F2 ?? ?? ?? 48 83 C3 10 48 39 D8 74"_sig
    );
    CompoundTag_contains = 
    reinterpret_cast<CompoundTag_contains_t>(CtagContainsaddr);

    auto ListTagGetaddr = scan(
    "31 C0 85 F6 78 ?? 48 8B 4F 08 48 8B 57 10 48 29 CA 48 C1 EA 03 39 F2 7E ?? 89 F0 48 8B 04 C1 C3"_sig
    );
    ListTag_get = 
    reinterpret_cast<ListTag_get_t>(ListTagGetaddr);

    auto ItemStackbaseloadItemaddr = scan(
    "55 41 57 41 56 41 54 53 48 83 EC 70 48 89 F5 49 89 FE 64 48 8B 04 25 28 00 00 00 48 89 44 24 ?? 48 8D 7C 24 ?? E8 ?? ?? ?? ?? 48 8D 35 ?? ?? ?? ?? BA ?? ?? ?? ?? 48 89 EF E8 ?? ?? ?? ?? 41 89 C4"_sig
    );
    ItemStackBase_loadItem =
    reinterpret_cast<ItemStackBase_loadItem_t>(ItemStackbaseloadItemaddr);

    auto ListTagSizeaddr = scan(
        "48 8B 47 10 48 2B 47 08 48 C1 E8 03"_sig
    );
    ListTag_size = 
    reinterpret_cast<ListTag_size_t>(ListTagSizeaddr);

    auto Isbgetnameaddr = scan(
        "41 56 53 48 83 EC 48 48 89 F3 49 89 FE 64 48 8B 04 25 28 00 00 00 48 89 44 24 40 48 89 F7 E8 0D 10 00 00 84 C0 74 0F 48 8D 7C 24 08 48 89 DE E8"_sig
    );
    ItemStackBase_getName =
    reinterpret_cast<ItemStackBase_getName_t>(Isbgetnameaddr);

    auto Isbctor = scan(
        "41 57 41 56 53 48 83 EC 30 48 89 FB 64 48 8B 04 25 28 00 00 00 48 89 44 24 28 48 8D 05 17 50 49"_sig
    );
    ItemStackBase_ctor =
    reinterpret_cast<ItemStackBase_ctor_t>(Isbctor);

    auto isbgetdmgaddr = scan(
        "48 8B 47 08 48 85 C0 74 14 48 8B 00 48 85 C0 74 0C 48 8B 77 10 48 89 C7 E9 03 73 03 00 31 C0 C3"_sig
    );
    ItemStackBase_getDamageValue = 
    reinterpret_cast<ItemStackBase_getDamageValue_t>(isbgetdmgaddr);

    auto Item_getIdaddr = scan( 
        "0F B7 87 8A 00 00 00 C3"_sig
    ); 
    Item_getId = 
    reinterpret_cast<Item_getId_t>(Item_getIdaddr);

    auto Isbgetitemaddr = scan(
        "48 8B 47 08 48 85 C0 74 04 48 8B 00 C3 31 C0 C3"_sig
    ); 
    ItemStackBase_getItem =  
    reinterpret_cast<ItemStackBase_getItem_t>(Isbgetitemaddr);

    auto CtagGetByteAddr = scan( 
        "53 48 83 EC 20 48 89 FB 64 48 8B 04 25 28 00 00 00 48 89 44 24 18 48 89 74 24 08 48 89 54 24 10 48 83 C7 08 48 8D 74 24 08 E8 62 D8 01 00 48 83"_sig 
    ); 
    CompoundTag_getByte = 
    reinterpret_cast<CompoundTag_getByte_t>(CtagGetByteAddr);

    auto BaseActorRenderContextCtorAddr = scan(
        "41 57 41 56 41 55 41 54 53 49 89 D4 49 89 F5 48 89 FB 48 8D 05 27 14 50 0A 48 89 07 0F 57 C0 0F"_sig
    );
    BaseActorRenderContext_ctor =
    reinterpret_cast<BaseActorRenderContext_ctor_t>(BaseActorRenderContextCtorAddr);

    auto ItemRendererRenderGuiItemNewAddr = scan(
        "55 41 57 41 56 41 55 41 54 53 48 81 EC C8 00 00 00 F3 0F 11 64 24 18 F3 0F 11 5C 24 0C F3 0F 11"_sig
    );
    ItemRenderer_renderGuiItemNew =
    reinterpret_cast<ItemRenderer_renderGuiItemNew_t>(ItemRendererRenderGuiItemNewAddr);

    auto textureGroup_getTexturea_addr = scan(
        "55 41 57 41 56 41 55 41 54 53 48 81 EC F8 00 00 00 45 89 CD 4D 89 C7 89 4C 24 0C 49 89 D6 49 89 F4 48 89 7C 24 10 64 48 8B 04 25 28 00 00 00 48"_sig
    );

    // ShulkerBoxBlockItem
    auto ZTS19ShulkerBoxBlockItem = hat::find_pattern(range1, hat::object_to_signature("19ShulkerBoxBlockItem")).get();
    auto _ZTI19ShulkerBoxBlockItem = hat::find_pattern(range2, hat::object_to_signature(ZTS19ShulkerBoxBlockItem)).get() - sizeof(void *);
    auto _ZTV19ShulkerBoxBlockItem = hat::find_pattern(range2, hat::object_to_signature(_ZTI19ShulkerBoxBlockItem)).get() + sizeof(void *);
    void **vtshulk53 = reinterpret_cast<void **>(_ZTV19ShulkerBoxBlockItem);
    // append slot 53
    ShulkerBoxBlockItem_appendFormattedHovertext_orig =
        reinterpret_cast<Shulker_appendHover_t>(vtshulk53[53]);
    vtshulk53[53] = reinterpret_cast<void *>(&ShulkerBoxBlockItem_appendFormattedHovertext_hook);

    // HovertextRenderer
    auto _ZTS17HoverTextRenderer = hat::find_pattern(range1, hat::object_to_signature("17HoverTextRenderer")).get();
    auto _ZTI17HoverTextRenderer = hat::find_pattern(range2, hat::object_to_signature(_ZTS17HoverTextRenderer)).get() - sizeof(void *);
    auto _ZTV17HoverTextRenderer = hat::find_pattern(range2, hat::object_to_signature(_ZTI17HoverTextRenderer)).get() + sizeof(void *);
    // slot17
    void **vtHR = reinterpret_cast<void **>(_ZTV17HoverTextRenderer);
    HoverRenderer_renderHoverBox_orig =
        reinterpret_cast<RenderHoverBoxFn>(vtHR[17]);
    vtHR[17] = reinterpret_cast<void *>(&HoverRenderer_renderHoverBox_hook);

    auto _ZTSMCUIRC = hat::find_pattern(range1, hat::object_to_signature("24MinecraftUIRenderContext")).get();
    auto _ZTIMCUIRC = hat::find_pattern(range2, hat::object_to_signature(_ZTSMCUIRC)).get() - sizeof(void *);
    auto _ZTVMCUIRC = hat::find_pattern(range2, hat::object_to_signature(_ZTIMCUIRC)).get() + sizeof(void *);
    // slot 6
    void **vtMCUIRC = reinterpret_cast<void **>(_ZTVMCUIRC);
    MinecraftUIRenderContext_drawText_orig =
        reinterpret_cast<MinecraftUIRenderContext_drawText_t>(vtMCUIRC[6]);
    vtMCUIRC[6] = reinterpret_cast<void *>(&MinecraftUIRenderContext_drawText_hook);
}

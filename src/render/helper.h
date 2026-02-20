#pragma once

#include <cstddef>
#include <cstdint>

// BaseActorRenderContext::BaseActorRenderContext(screenContext, clientInstance, minecraftGame)
using BaseActorRenderContext_ctor_t = void (*)(void*, void*, void*, void*);
extern BaseActorRenderContext_ctor_t BaseActorRenderContext_ctor;

// ItemRenderer::renderGuiItemNew(barc, itemStack, aux, x, y, glintOnly, a, b, c, d)
using ItemRenderer_renderGuiItemNew_t =
    std::uint64_t (*)(void*, void*, void*, unsigned int, float, float, bool, float, float, float, int);
extern ItemRenderer_renderGuiItemNew_t ItemRenderer_renderGuiItemNew;

// 1.21.114 offsets from RE.
inline constexpr std::size_t kBarcStorageSize = 0x400;
inline constexpr std::size_t kBarcItemRendererOffset = 0x58;      // BaseActorRenderContext + 88
inline constexpr std::size_t kClientMinecraftGameOffset = 0xA8;   // ClientInstance + 168 fallback
inline constexpr std::size_t kClientGetMinecraftGameVfIndex = 84; // vtable slot for getMinecraftGame

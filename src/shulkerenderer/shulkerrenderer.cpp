#include "shulkerrenderer.h"

#include <cstdio>

namespace {
constexpr int kColumns = 9;
constexpr int kRows = 3;
constexpr float kSlotStride = 18.0f;
constexpr float kSlotDrawSize = 17.5f;
constexpr float kPanelPadding = 6.0f;
constexpr float kItemDrawSize = 16.0f;
constexpr float kItemInset = (kSlotStride - kItemDrawSize) * 0.5f;
constexpr float kCountTextHeight = 6.0f;

const HashedString kFlushMaterial("ui_flush");
const NinesliceHelper kPanelNineSlice(16.0f, 16.0f, 4.0f, 4.0f);
const mce::Color kWhite{1.0f, 1.0f, 1.0f, 1.0f};

struct CachedUiTextures {
    bool loaded = false;
    mce::TexturePtr panel;
    mce::TexturePtr slot;
};

inline bool hasTexture(const mce::TexturePtr& texture) {
    return static_cast<bool>(texture.mClientTexture);
}

template <typename Fn>
void forEachSlot(float originX, float originY, Fn&& fn) {
    for (int row = 0; row < kRows; ++row) {
        for (int column = 0; column < kColumns; ++column) {
            const int slotIndex = row * kColumns + column;
            const float slotX = originX + column * kSlotStride;
            const float slotY = originY + row * kSlotStride;
            fn(slotIndex, slotX, slotY);
        }
    }
}

ItemStackBase* getRenderableStack(ShulkerSlotCache& slotCache) {
    if (!slotCache.valid)
        return nullptr;

    ItemStackBase* stack = asISB(slotCache.isb);
    if (!stack)
        return nullptr;

    if (ItemStackBase_getItem && !ItemStackBase_getItem(stack))
        return nullptr;

    return stack;
}

bool hasEnchantedSlots(int cacheIndex) {
    for (int slotIndex = 0; slotIndex < SHULKER_SLOT_COUNT; ++slotIndex) {
        const ShulkerSlotCache& slotCache = ShulkerCache[cacheIndex][slotIndex];
        if (slotCache.valid && slotCache.enchanted)
            return true;
    }
    return false;
}

void resetImageTintWithSentinel(MinecraftUIRenderContext& ctx, const mce::TexturePtr& sentinelTexture) {
    if (hasTexture(sentinelTexture)) {
        ctx.drawImage(
            sentinelTexture.getClientTexture(),
            {-10000.0f, -10000.0f},
            {1.0f, 1.0f},
            {0.0f, 0.0f},
            {1.0f, 1.0f},
            false
        );
    }

    ctx.flushImages(kWhite, 1.0f, kFlushMaterial);
}

void* getMinecraftGame(void* clientInstance) {
    if (!clientInstance)
        return nullptr;

    auto** vtable = *reinterpret_cast<void***>(clientInstance);
    if (vtable && vtable[kClientGetMinecraftGameVfIndex]) {
        auto fn = reinterpret_cast<void* (*)(void*)>(vtable[kClientGetMinecraftGameVfIndex]);
        if (void* game = fn(clientInstance))
            return game;
    }

    auto* bytes = reinterpret_cast<std::byte*>(clientInstance);
    return *reinterpret_cast<void**>(bytes + kClientMinecraftGameOffset);
}

void destroyBaseActorRenderContext(void* barc) {
    if (!barc)
        return;

    auto** vtable = *reinterpret_cast<void***>(barc);
    if (!vtable || !vtable[0])
        return;

    auto dtor = reinterpret_cast<void (*)(void*)>(vtable[0]);
    dtor(barc);
}

CachedUiTextures& getUiTextures(MinecraftUIRenderContext& ctx) {
    static CachedUiTextures textures;
    if (!textures.loaded) {
        textures.panel = ctx.getTexture(
            ResourceLocation("textures/ui/dialog_background_opaque", ResourceFileSystem::UserPackage),
            false
        );
        textures.slot = ctx.getTexture(
            ResourceLocation("textures/ui/item_cell", ResourceFileSystem::UserPackage),
            false
        );
        textures.loaded = true;
    }
    return textures;
}

void drawPanelTexture(MinecraftUIRenderContext& ctx, const CachedUiTextures& textures, const RectangleArea& panel) {
    if (!hasTexture(textures.panel))
        return;

    kPanelNineSlice.draw(ctx, panel, textures.panel.getClientTexture());
}

void drawSlotTexture(MinecraftUIRenderContext& ctx, const CachedUiTextures& textures, const RectangleArea& slotRect) {
    if (!hasTexture(textures.slot))
        return;

    const glm::vec2 pos{slotRect._x0, slotRect._y0};
    const glm::vec2 size{slotRect._x1 - slotRect._x0, slotRect._y1 - slotRect._y0};
    const glm::vec2 uv{0.0f, 0.0f};
    const glm::vec2 uvSize{1.0f, 1.0f};
    ctx.drawImage(textures.slot.getClientTexture(), pos, size, uv, uvSize, false);
}

void drawSlotIcons(
    MinecraftUIRenderContext& ctx,
    const CachedUiTextures& textures,
    int cacheIndex,
    float slotOriginX,
    float slotOriginY
) {
    if (!BaseActorRenderContext_ctor || !ItemRenderer_renderGuiItemNew)
        return;
    if (!ctx.mClient || !ctx.mScreenContext)
        return;

    void* clientInstance = static_cast<void*>(ctx.mClient);
    void* minecraftGame = getMinecraftGame(clientInstance);
    if (!minecraftGame)
        return;

    alignas(16) std::byte barcStorage[kBarcStorageSize]{};
    void* barc = static_cast<void*>(barcStorage);
    BaseActorRenderContext_ctor(
        barc,
        static_cast<void*>(ctx.mScreenContext),
        clientInstance,
        minecraftGame
    );

    auto* barcBytes = reinterpret_cast<std::byte*>(barc);
    void* itemRenderer = *reinterpret_cast<void**>(barcBytes + kBarcItemRendererOffset);
    if (!itemRenderer) {
        destroyBaseActorRenderContext(barc);
        return;
    }

    const auto renderPass = [&](bool glintOnly, bool enchantedOnly) {
        forEachSlot(slotOriginX, slotOriginY, [&](int slotIndex, float slotX, float slotY) {
            ShulkerSlotCache& slotCache = ShulkerCache[cacheIndex][slotIndex];
            if (enchantedOnly && !slotCache.enchanted)
                return;

            ItemStackBase* stack = getRenderableStack(slotCache);
            if (!stack)
                return;

            ItemRenderer_renderGuiItemNew(
                itemRenderer,
                barc,
                static_cast<void*>(stack),
                0,
                slotX + kItemInset,
                slotY + kItemInset,
                glintOnly,
                1.0f,
                1.0f,
                1.0f,
                0
            );
        });
    };

    resetImageTintWithSentinel(ctx, textures.slot);
    renderPass(false, false);

    if (hasEnchantedSlots(cacheIndex)) {
        resetImageTintWithSentinel(ctx, textures.slot);
        renderPass(true, true);
    }

    destroyBaseActorRenderContext(barc);
    ctx.flushImages(kWhite, 1.0f, kFlushMaterial);
}

void drawStackCountText(
    Font& font,
    float slotX,
    float slotY,
    const char* text,
    const TextMeasureData& measureData,
    const CaretMeasureData& caretData
) {
    const float textWidth = ActiveUIContext->getLineLength(font, text, measureData.fontSize, false);
    const float anchorX = slotX + kSlotDrawSize - 0.5f;
    const float anchorY = slotY + kSlotDrawSize - 1.5f;

    const RectangleArea shadowRect{
        anchorX - textWidth + 1.0f,
        anchorX + 1.0f,
        anchorY - kCountTextHeight + 1.0f,
        anchorY + 1.0f
    };

    const RectangleArea textRect{
        anchorX - textWidth,
        anchorX,
        anchorY - kCountTextHeight,
        anchorY
    };

    ActiveUIContext->drawText(
        font,
        shadowRect,
        text,
        mce::Color{0.0f, 0.0f, 0.0f, 0.75f},
        ui::TextAlignment::Right,
        1.0f,
        measureData,
        caretData
    );

    ActiveUIContext->drawText(
        font,
        textRect,
        text,
        mce::Color{1.0f, 1.0f, 1.0f, 1.0f},
        ui::TextAlignment::Right,
        1.0f,
        measureData,
        caretData
    );
}
} // namespace

void ShulkerRenderer::render(
    MinecraftUIRenderContext* ctx,
    float x,
    float y,
    int index,
    char colorCode
) {
    if (!ctx || !ActiveUIContext || !ActiveUIFont)
        return;

    const mce::Color tint = getShulkerTint(colorCode);
    const CachedUiTextures& textures = getUiTextures(*ctx);

    const RectangleArea panelRect{
        x,
        x + kColumns * kSlotStride + kPanelPadding * 2.0f,
        y,
        y + kRows * kSlotStride + kPanelPadding * 2.0f
    };

    drawPanelTexture(*ctx, textures, panelRect);
    ctx->flushImages(tint, 1.0f, kFlushMaterial);

    const float slotOriginX = x + kPanelPadding;
    const float slotOriginY = y + kPanelPadding;

    Font& font = *ActiveUIFont;

    TextMeasureData measureData{};
    measureData.fontSize = 1.0f;
    measureData.renderShadow = false;

    CaretMeasureData caretData{};
    caretData.position = 0;
    caretData.shouldRender = false;

    forEachSlot(slotOriginX, slotOriginY, [&](int, float slotX, float slotY) {
        const RectangleArea slotRect{
            slotX,
            slotX + kSlotDrawSize,
            slotY,
            slotY + kSlotDrawSize
        };
        drawSlotTexture(*ctx, textures, slotRect);
    });

    ctx->flushImages(tint, 1.0f, kFlushMaterial);
    drawSlotIcons(*ctx, textures, index, slotOriginX, slotOriginY);

    forEachSlot(slotOriginX, slotOriginY, [&](int slotIndex, float slotX, float slotY) {
        ShulkerSlotCache& slotCache = ShulkerCache[index][slotIndex];
        if (!slotCache.valid || slotCache.count <= 1)
            return;

        char countText[8];
        std::snprintf(countText, sizeof(countText), "%u", slotCache.count);
        drawStackCountText(font, slotX, slotY, countText, measureData, caretData);
    });

    ctx->flushText(0.0f, std::nullopt);
}

#include <iostream>
#include <memory>
#include "fuzzer/FuzzedDataProvider.h"
#include "ttf2mesh.h"

class Font {
    public:
    constexpr Font(const uint8_t* data, std::size_t size): font{nullptr} {
        ttf_load_from_mem(data, static_cast<int>(size), &font, false);
    }

    ~Font() {
        ttf_free(font);
    }
    Font(const Font& other) = default;
    Font(Font&& other) = default;
    Font& operator=(const Font& other) = default;
    Font& operator=(Font&& other) = default;
    ttf_t *font{};

    void load_mesh() const {
        ttf_mesh_t *out;
        if (ttf_glyph2mesh(&font->glyphs[0], &out, TTF_QUALITY_NORMAL, TTF_FEATURES_DFLT)
        != TTF_DONE)
            return;
        ttf_free_mesh(out);
    }

    void load_3D_mesh() const {
        ttf_mesh3d_t* out;
        if (ttf_glyph2mesh3d(&font->glyphs[0], &out, TTF_QUALITY_NORMAL, TTF_FEATURES_DFLT,
                             0.1f) != TTF_DONE)
            return;
        ttf_free_mesh3d(out);
    }

    void load_svg() const {
        ttf_glyph2svgpath(&font->glyphs[0], 0.1f, 0.2f);
    }
};


extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {

    FuzzedDataProvider fdp(data, size);

    const auto symbol = static_cast<wchar_t>('A');
    const std::size_t font_size = fdp.remaining_bytes();
    std::unique_ptr<uint8_t[]> data_ptr(new uint8_t[font_size]);
    fdp.ConsumeData(data_ptr.get(), font_size);

    // Load the font
    auto font = Font(data_ptr.get(), font_size);
    if (font.font == nullptr) {
        // Failed to load font
        return -1;
    }

//     Find the symbol in the font
    int index = ttf_find_glyph(font.font, symbol);
    if (index < 0) return -1;
    // Make mesh object from the glyph

    font.load_mesh();
    font.load_3D_mesh();
    font.load_svg();
    return 0;
}

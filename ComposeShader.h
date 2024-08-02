#ifndef ComposeShader_DEFINE
#define ComposeShader_DEFINE

#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GPixel.h"
#include "starter_canvas.h"

class ComposeShader : public GShader {
    GShader* fShaderA;
    GShader* fShaderB;
    GPixel mul(GPixel c0, GPixel c1) {
        return GPixel_PackARGB(
            GRoundToInt(GPixel_GetA(c0) * GPixel_GetA(c1) / 255.f),
            GRoundToInt(GPixel_GetR(c0) * GPixel_GetR(c1) / 255.f),
            GRoundToInt(GPixel_GetG(c0) * GPixel_GetG(c1) / 255.f),
            GRoundToInt(GPixel_GetB(c0) * GPixel_GetB(c1) / 255.f)
        );
    }
public:
    ComposeShader(GShader* shaderA, GShader* shaderB) : 
        fShaderA(shaderA), fShaderB(shaderB) {}

    bool isOpaque() override {
        return fShaderA->isOpaque() && fShaderB->isOpaque();
    }

    bool setContext(const GMatrix& ctm) override {
        return fShaderA->setContext(ctm) && fShaderB->setContext(ctm);
    }

    void shadeRow(int x, int y, int count, GPixel row[]) override {
        GPixel rowA[count];
        GPixel rowB[count];
        fShaderA->shadeRow(x, y, count, rowA);
        fShaderB->shadeRow(x, y, count, rowB);
        for (int i = 0; i < count; i++) {
            row[i] = mul(rowA[i], rowB[i]);
        }
    }
};

#endif
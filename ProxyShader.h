#ifndef ProxyShader_DEFINE
#define ProxyShader_DEFINE

#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GPixel.h"
#include "starter_canvas.h"

class ProxyShader : public GShader {
    GShader* fRealShader;
    GMatrix fExtraTransform;
    
public:
    ProxyShader(GShader* shader, const GMatrix& extraTransform) : 
        fRealShader(shader), fExtraTransform(extraTransform) {}

    bool isOpaque() override {return fRealShader->isOpaque();}

    bool setContext(const GMatrix& ctm) override {
        return fRealShader->setContext(ctm * fExtraTransform);
    }
    
    void shadeRow(int x, int y, int count, GPixel row[]) override {
        fRealShader->shadeRow(x, y, count, row);
    }
};

#endif
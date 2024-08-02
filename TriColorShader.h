#ifndef TriColorShader_DEFINE
#define TriColorShader_DEFINE

#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GPixel.h"
#include "starter_canvas.h"

class TriColorShader : public GShader {
public:
    TriColorShader(const GPoint points[3], const GColor colors[3]) : 
        localMatrix({points[1] - points[0], points[2] - points[0], points[0]}), 
        colors{colors[0], colors[1], colors[2]} {}

    bool isOpaque() {
        return (colors[0].a == 255 && colors[1].a == 255 && colors[2].a == 255);
    }
    bool setContext(const GMatrix &ctm) {
        GMatrix tmp = ctm * localMatrix;
        if (!tmp.invert()) {
            return false;
        }
        localInv = *tmp.invert();
        return true;
    }
    void shadeRow(int x, int y, int count, GPixel row[]) {
        GPoint gp = localInv * GPoint({x + 0.5f, y + 0.5f});
        GColor DC1 = colors[1] - colors[0];
        GColor DC2 = colors[2] - colors[0];
        GColor DC = localInv[0] * DC1 + localInv[1] * DC2;
        GColor c = gp.x * DC1 + gp.y * DC2 + colors[0];
        for (int i = 0; i < count; i++) {
            row[i] = premul(c);
            c += DC;
        }
    }

private:
    const GMatrix localMatrix;
    GMatrix localInv;
    const GColor colors[3];

    GPixel premul(const GColor& color) {  
        float a = color.a * 255;   
        int r = GRoundToInt(color.r * a);
        int g = GRoundToInt(color.g * a);
        int b = GRoundToInt(color.b * a);
        return GPixel_PackARGB(GRoundToInt(a), r, g, b);
    }
};

#endif
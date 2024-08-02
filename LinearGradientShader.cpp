#include "include/GShader.h"
#include "include/GBitmap.h"
#include "include/GMatrix.h"
#include "include/GPixel.h"
#include "starter_canvas.h"

class LinearGradientShader : public GShader {
public:
    LinearGradientShader(GPoint p0, GPoint p1, const GColor color[], int count, const GTileMode mode) : count(count), mode(mode) {
        for (int i = 0; i < count; i++) {
            colors.push_back(color[i]);
        }
        colors.push_back(color[count - 1]);
        for (int i = 0; i < count - 1; i++) {
            colordiffs.push_back(colors[i + 1] - colors[i]);
        }
        colordiffs.push_back(color[count - 1]);
        GVector u = p1 - p0;
        localMatrix = GMatrix( u, {-u.y, u.x}, p0 );
    }

    bool isOpaque() {
        return false;
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
        if (this->count == 1) {
            for (int i = 0; i < count; i++) row[i] = premul(colors[0]);
        }
        else {
            GPoint gp = localInv * GPoint( {x +  0.5f, y + 0.5f} );
            float gx = gp.x;
            float a = localInv[0];
            for (int i = 0; i < count; i++) {
                float fx; 
                
                switch(mode) {
                    case GTileMode::kClamp:
                        fx = std::clamp(gx, 0.0f, 1.0f);
                        break;
                    case GTileMode::kRepeat:
                        fx = gx - GFloorToInt(gx);
                        break;
                    case GTileMode::kMirror:
                        fx = gx * 0.5f;
                        fx = fx - GFloorToInt(fx);
                        if (fx > 0.5f)
                            fx = 1 - fx;
                        fx *= 2;
                        break;
                }

                if (this->count == 2) {
                    GColor c = colors[0] + fx * colordiffs[0];
                    row[i] = premul(c);
                    gx += a;
                } else {
                    fx *= (this->count - 1.0f);
                    int k = GFloorToInt(fx);
                    float t = fx - k;
                    GColor c = colors[k] + t * colordiffs[k];
                    row[i] = premul(c);
                    gx += a;
                }
            }
        }
    }

    GPixel premul(const GColor& color) {  
    float a = color.a * 255;   
    int r = GRoundToInt(color.r * a);
    int g = GRoundToInt(color.g * a);
    int b = GRoundToInt(color.b * a);
    return GPixel_PackARGB(GRoundToInt(a), r, g, b);
}
private:
    const int count;
    std::vector<GColor> colors;
    GMatrix localMatrix;
    GMatrix localInv;
    GTileMode mode;
    std::vector<GColor> colordiffs;

    int modulo(int x, int m) {
        return (x % m + m) % m;
    }
};

std::unique_ptr<GShader> GCreateLinearGradient(GPoint p0, GPoint p1, const GColor color[], int count, const GTileMode mode) {
    if (p0 == p1)
        return nullptr;
    return std::unique_ptr<GShader>(new LinearGradientShader(p0, p1, color, count, mode));
}
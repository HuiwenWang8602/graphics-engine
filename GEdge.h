#ifndef GEdge_DEFINED
#define GEdge_DEFINED

#include "include/GPoint.h"

struct GEdge {
    float m, x;
    int top, bot, w;
    // float b;
    bool isValid(float y){
        return (top <= GRoundToInt(y) && GRoundToInt(y) < bot);
    } 
    /* returns value of x for given y */
    // float eval(float y) {
    //     return m * (y + 0.5f) + b;
    // } 
    bool operator < (const GEdge& edge) const {
        return (top == edge.top) ? x < edge.x : top < edge.top;
    }
    /* x = my + b*/
    static inline GEdge makeGEdge(GPoint p0, GPoint p1, int w) {
        if (p0.y > p1.y) std::swap(p0, p1);
        float m = (p1.x - p0.x) / (p1.y - p0.y);
        float b = p0.x - m * p0.y;
        int top = GRoundToInt(p0.y);
        int bot = GRoundToInt(p1.y);
        float x = m * (top + 0.5f) + b;
        return { m, x, top, bot, w };
    }   

    void updateX() {
        x += m;
    }
};

#endif
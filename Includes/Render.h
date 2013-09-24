
#ifndef _Render_h
#define _Render_h

int AsmRender(int32 *scrBuff, int32 *SkyBmp, int D, int H1, int k1, int dX1, int dY1,int H2, int k2, int dX2, int dY2, int Horiz, int SkyW, int SkyH,int SkrW,int SkrWHalf ) asm("AsmRender");

#endif

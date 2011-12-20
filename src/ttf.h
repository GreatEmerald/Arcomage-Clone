#ifndef _TTF_H_
#define _TTF_H_

void InitTTF();
void RenderLine(char* text, SizeF location);
int FindOptimalFontSize();
//int round(double x);
int nextpoweroftwo(int x);
int Min(int A, int B);

#endif

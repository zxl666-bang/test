#include<stdio.h>
#define P(X,Y) (2/(1/X+1/Y))
int main()
{
    int x,y;
    scanf("%d %d",&x,&y);
    printf("%.2lf",P(x,y));

}
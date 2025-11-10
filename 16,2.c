#include<stdio.h>
#include<math.h>
#define pai 3.1415
int main()
{
    int r;
    double a;
    double A;
    scanf("%d %lf",&r,&a);
    A=a*pai/180.0;
    double x=r*cos(A);
    double y=r*sin(A);
    printf("%.0lf,%.0lf",x,y);


}
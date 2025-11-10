#include <stdio.h>
#include<string.h>
#include<stdlib.h>
#define Size 40
char*s_gets(char*st,int n);
struct film
{
    char title[Size];
    int num;
    struct film*next;
};
int main()
{
    struct film*head=NULL;
    struct film*prev,*currunt;
    char input[Size];
    while(s_gets(input,Size)!=NULL&&input[0]!='\0')
    {
        currunt=(struct film*)malloc(sizeof(struct film));
        if(head==NULL)
            head=currunt;
        else
        {
            prev->next=currunt;
        }
        currunt->next=NULL;
        strcpy(currunt->title,input);
        scanf("%d",&currunt->num);
        while(getchar()!='\n')
            continue;
        prev=currunt;
    }
    if(head=NULL)
        printf("no\n");
    currunt=head;
    while(currunt!=NULL)
    {
        printf("%s %d",currunt->title,currunt->num);
        currunt=currunt->next;
    }
    currunt=head;
    while(currunt!=NULL)
    {
        head=currunt->next;
        free(currunt);
        currunt=head;
    }
    return 0;
}
char*s_gets(char*st,int n)
{
    char*red_val;
    char*find;
    red_val=fgets(st,n,stdin);
    if(red_val)
    {
        find=strchr(st,'\n');
        if(find)
        {
            *find='\0';
        }
        else
            while(getchar()!='\n')
                continue;
    }
    return red_val;
}
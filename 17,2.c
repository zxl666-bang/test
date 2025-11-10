#include <stdio.h>
#include<stdlib.h>
#include<stdbool.h>
typedef struct A
{
    int num;
    char*title;
}Itme;
typedef struct B
{
    Itme itme;
    struct B*next;
}Node;
typedef Node*List;
void InitializeList(List*plist)
{
    plist=NULL;
}
bool Listisempty(List*plist)
{
    if(*plist==NULL)
    return true;
    else
    return false;
}
bool Listisfull(List*plist)
{
    List pr;
    pr=(Node*)malloc(sizeof(Node*));
    if(pr==NULL)
    return true;
    else
    free(pr);
    return false;
}
unsigned int Listitmecount(List*plist)
{
    unsigned int count=0;
    Node*pnode=*plist;
    while(pnode!=NULL)
  {
      ++count;
     pnode=pnode->next;
      }
      return count;
}
 void copy(Itme itme,Node*pnode)
{
    pnode->itme=itme;
}
bool AddItme(Itme itme,List*plist)
{
    Node*pnew;
    Node*scan=*plist;
    pnew=(Node*)malloc(sizeof(Node));
    if(pnew==NULL)
    {
        return false;
    }
    copy(itme,pnew);
    pnew->next=NULL;
    if(scan==NULL)
    {
        scan->next=pnew;
    }
    else
    {
        while(scan->next!=NULL)
        scan=scan->next;
        scan->next=pnew;
    }
    return true;
}
void hanshu(List*plist,void(*pfun)(Itme itme))
{
    Node*pnode=*plist;
    while(pnode!=NULL)
    {
        (*pfun)(pnode->itme);
        pnode=pnode->next;
    }
}
void shifang(List*plist)
{
    Node*psave;
    while(*plist!=NULL)
    {
        psave=(*plist)->next;
        free(*plist);
        *plist=psave;
    }
}
#ifndef PARSER_H
#define PARSER_H
#define typeOpr 3

int expr();
int term();
int factor();
typedef struct node {
    int type;
    union {
        int value;                // TAMSAYI tipi için değer
        char id[100];             // TANIMLAYICI tipi için kimlik
        int nops;                      // Alt düğüm sayısı
        struct node *op[10];      // typeOpr tipi için alt düğümler
    } opr;
    
    char operator;
} nodeType;


extern int nextToken;
#endif

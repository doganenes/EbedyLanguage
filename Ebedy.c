#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "include/parser.h"
#include "include/parseerr.h"
#include "include/front.h"
#include "include/parsetree.h"

/* DEĞİŞKENLER */
int charClass;
char lexeme[100];
char nextChar;
int lexLen;
int nextToken;
FILE *in_fp;

#define MAX_VARIABLES 100

/* Değişkenlerin saklandığı struct */
typedef struct
{
    char name[20];
    int value;
} Variable;

Variable variables[MAX_VARIABLES];
int variableCount = 0;

/* FONKSİYONLAR */
void addChar();
void getChar();
void getNonBlank();
int lex();
int isVariable(char *lexeme);
int getVariableIndex(char *lexeme);
void addVariable(char *lexeme);
void setVariableValue(char *lexeme, int value);
int getVariableValue(char *lexeme);
void printVariables();
void assignment();

/* İŞARETLER */
#define HARF 0
#define RAKAM 1
#define TANIMSIZ 99
#define TAMSAYI 10
#define TANIMLAYICI 11
#define ATAMA 20
#define TOPLAMA_OPERATORU 21
#define CIKARMA_OPERATORU 22
#define CARPMA_OPERATORU 23
#define BOLME_OPERATORU 24
#define SOLPARANTEZ 25
#define SAGPARANTEZ 26
#define SATIR_SONU 27

int main()
{
    if ((in_fp = fopen("include/front.in", "r")) == NULL)
    {
        printf("HATA - front.in dosyası açılamadı. \n");
    }
    else
    {
        getChar();
        do
        {
            lex();
            assignment();
        } while (nextToken != EOF);

        printVariables();
    }

    return 0;
}

// lexer
int lookup(char ch)
{
    switch (ch)
    {
    case '(':
        addChar();
        nextToken = SOLPARANTEZ;
        break;
    case ')':
        addChar();
        nextToken = SAGPARANTEZ;
        break;
    case '+':
        addChar();
        nextToken = TOPLAMA_OPERATORU;
        break;
    case '-':
        addChar();
        nextToken = CIKARMA_OPERATORU;
        break;
    case '*':
        addChar();
        nextToken = CARPMA_OPERATORU;
        break;
    case '/':
        addChar();
        nextToken = BOLME_OPERATORU;
        break;
    case '=':
        addChar();
        nextToken = ATAMA;
        break;
    case ';':
        addChar();
        nextToken = SATIR_SONU;
        break;
    default:
        addChar();
        nextToken = EOF;
        break;
    }
    return nextToken;
}

void addChar()
{
    if (lexLen <= 98)
    {
        lexeme[lexLen++] = nextChar;
        lexeme[lexLen] = 0;
    }
    else
    {
        printf("HATA - Girilen sözcük çok uzun! \n");
    }
}

void getChar()
{
    if ((nextChar = getc(in_fp)) != EOF)
    {
        if (isalpha(nextChar))
            charClass = HARF;
        else if (isdigit(nextChar))
            charClass = RAKAM;
        else
            charClass = TANIMSIZ;
    }
    else
    {
        charClass = EOF;
    }
}

void getNonBlank()
{
    while (isspace(nextChar))
        getChar();
}

int lex()
{
    lexLen = 0;
    getNonBlank();

    switch (charClass)
    {

    case HARF:
        addChar();
        getChar();
        while (charClass == HARF || charClass == RAKAM)
        {
            addChar();
            getChar();
        }
        nextToken = TANIMLAYICI;
        break;

    case RAKAM:
        addChar();
        getChar();
        while (charClass == RAKAM)
        {
            addChar();
            getChar();
        }
        nextToken = TAMSAYI;
        break;

    case TANIMSIZ:
        lookup(nextChar);
        getChar();
        break;

    case EOF:
        nextToken = EOF;
        lexeme[0] = 'E';
        lexeme[1] = 'O';
        lexeme[2] = 'F';
        lexeme[3] = 0;
        break;
    }

    printf("Yeni sembol: %d, Yeni sözcük: %s\n", nextToken, lexeme);
    return nextToken;
}

// parser
static void error();

int expr()
{
    int result = term();

    while (nextToken == TOPLAMA_OPERATORU || nextToken == CIKARMA_OPERATORU)
    {
        if (nextToken == TOPLAMA_OPERATORU)
        {
            lex();
            result += term();
        }
        else if (nextToken == CIKARMA_OPERATORU)
        {
            lex();
            result -= term();
        }
    }

    printf("Sonuç: %d\n", result);
    return result;
}

int factor()
{
    int result;

    if (nextToken == TANIMLAYICI)
    {
        result = getVariableValue(lexeme);
        lex();
    }
    else if (nextToken == TAMSAYI)
    {
        result = atoi(lexeme);
        lex();
    }
    else if (nextToken == SOLPARANTEZ)
    {
        lex();
        result = expr();
        if (nextToken == SAGPARANTEZ)
            lex();
        else
            printf("HATA - ')' bekleniyor.\n");
    }
    else
    {
        printf("HATA - Değişken veya sayı bekleniyor.\n");
        result = 0;
    }

    return result;
}

int term()
{
    int result = factor();

    while (nextToken == CARPMA_OPERATORU || nextToken == BOLME_OPERATORU)
    {
        if (nextToken == CARPMA_OPERATORU)
        {
            lex();
            result *= factor();
        }
        else if (nextToken == BOLME_OPERATORU)
        {
            lex();
            int divisor = factor();
            if (divisor != 0)
                result /= divisor;
            else
                printf("HATA - Sıfıra bölme hatası.\n");
        }
    }

    return result;
}

void assignment()
{
    if (nextToken == TANIMLAYICI)
    {
        char variableName[20];
        strcpy(variableName, lexeme);
        lex();
        if (nextToken == ATAMA)
        {
            lex();
            int value = expr();
            setVariableValue(variableName, value);
        }
        else
        {
            printf("HATA - Atama operatörü bekleniyor.\n");
        }
    }
}

/* Yardımcı Fonksiyonlar */
int isVariable(char *lexeme)
{
    for (int i = 0; i < variableCount; i++)
    {
        if (strcmp(variables[i].name, lexeme) == 0)
            return 1;
    }
    return 0;
}

int getVariableIndex(char *lexeme)
{
    for (int i = 0; i < variableCount; i++)
    {
        if (strcmp(variables[i].name, lexeme) == 0)
            return i;
    }
    return -1;
}

void addVariable(char *lexeme)
{
    Variable variable;
    strcpy(variable.name, lexeme);
    variable.value = 0;
    variables[variableCount++] = variable;
}

void setVariableValue(char *lexeme, int value)
{
    if (isVariable(lexeme))
    {
        int index = getVariableIndex(lexeme);
        variables[index].value = value;
    }
    else
    {
        if (variableCount < MAX_VARIABLES)
        {
            addVariable(lexeme);
            setVariableValue(lexeme, value);
        }
        else
        {
            printf("HATA - Maksimum değişken sayısına ulaşıldı.\n");
        }
    }
}

int getVariableValue(char *lexeme)
{
    if (isVariable(lexeme))
    {
        int index = getVariableIndex(lexeme);
        return variables[index].value;
    }
    else
    {
        printf("HATA - Tanımlanmamış değişken: %s\n", lexeme);
        return 0;
    }
}

void printVariables()
{
    printf("Değişkenler:\n");
    for (int i = 0; i < variableCount; i++)
    {
        printf("%s = %d\n", variables[i].name, variables[i].value);
    }
}
static void error()
{
    printf("HATA - tanımlanamayan ifade içeriyor.\n");
}

// PARSE ERROR
#ifndef AYRISTIRICI_HATASI
#define AYRISTIRICI_HATASI

void yyerror(char *);

#endif

// PARSE TREE
static const int DEL = 1;
static const int EPS = 3;

static void graphInit(void);
static void graphFinish(void);
static void exNode(nodeType *p, int c, int l, int *ce, int *cm);
static void graphDrawBox(char *s, int c, int l);
static void graphBox(char *s, int *w, int *h);
static void graphDrawArrow(int c1, int l1, int c2, int l2);

int ex(nodeType *p)
{
    int rte, rtm;

    graphInit();
    exNode(p, 0, 0, &rte, &rtm);
    graphFinish();

    return 0;
}

#define LMAX 200
#define CMAX 200

static char graph[LMAX][CMAX];
static int graphNumber = 0;

void graphTest(int l, int c)
{
    int ok;
    ok = 1;
    if (l < 0)
        ok = 0;
    if (l >= LMAX)
        ok = 0;
    if (c < 0)
        ok = 0;
    if (c >= CMAX)
        ok = 0;
    if (ok)
        return;
    printf("\n+++HATA: l=%d, c=%d dikdörtgen çizilemedi: 0, 0 ... %d, %d",
           l, c, LMAX, CMAX);
    exit(1);
}

static void graphInit(void)
{
    int i, j;
    for (i = 0; i < LMAX; i++)
    {
        for (j = 0; j < CMAX; j++)
        {
            graph[i][j] = ' ';
        }
    }
}

static void exNode(nodeType *p, int c, int l, int *ce, int *cm)
{
    int w, h;
    char *s;
    int cbar;
    int k;
    int che, chm;
    int cs;
    char word[20];

    if (!p)
        return;

    strcpy(word, "???");
    s = word;
    switch (p->type)
    {
    case typeIntConst:
        sprintf(word, "%d", p->opr.op[0]->opr.value);
        break;
    case TANIMLAYICI:
        strcpy(word, lexeme);
        nextToken = TANIMLAYICI;
        break;
    case typeOpr:
        switch (p->operator)
        {
        case ';':
            s = "[;]";
            break;
        case '=':
            s = "[=]";
            break;
        case '+':
            s = "[+]";
            break;
        case '-':
            s = "[-]";
            break;
        case '*':
            s = "[*]";
            break;
        case '/':
            s = "[/]";
            break;
        case '<':
            s = "[<]";
            break;
        case '>':
            s = "[>]";
            break;
        }
        break;
    }

    graphBox(s, &w, &h);
    cbar = c;
    *ce = c + w;
    *cm = c + w / 2;

    if (p->type == typeIntConst || p->type == TANIMLAYICI || p->opr.nops == 0)
    {
        graphDrawBox(s, cbar, l);
        return;
    }

    cs = c;
    for (k = 0; k < p->opr.nops; k++)
    {
        exNode(p->opr.op[k], cs, l + h + EPS, &che, &chm);
        cs = che;
    }

    /* total node width */
    if (w < che - c)
    {
        cbar += (che - c - w) / 2;
        *ce = che;
        *cm = (c + che) / 2;
    }

    graphDrawBox(s, cbar, l);

    cs = c;
    for (k = 0; k < p->opr.nops; k++)
    {
        exNode(p->opr.op[k], cs, l + h + EPS, &che, &chm);
        graphDrawArrow(*cm, l + h, chm, l + h + EPS - 1);
        cs = che;
    }
}

static void graphBox(char *s, int *w, int *h)
{
    *w = strlen(s) + DEL;
    *h = 1;
}

static void graphDrawBox(char *s, int c, int l)
{
    int i;
    graphTest(l, c + strlen(s) - 1 + DEL);
    for (i = 0; i < strlen(s); i++)
    {
        graph[l][c + i + DEL] = s[i];
    }
}

static void graphDrawArrow(int c1, int l1, int c2, int l2)
{
    int m;
    graphTest(l1, c1);
    graphTest(l2, c2);
    m = (l1 + l2) / 2;
    while (l1 != m)
    {
        graph[l1][c1] = '|';
        if (l1 < l2)
            l1++;
        else
            l1--;
    }
    while (c1 != c2)
    {
        graph[l1][c1] = '-';
        if (c1 < c2)
            c1++;
        else
            c1--;
    }
    while (l1 != l2)
    {
        graph[l1][c1] = '|';
        if (l1 < l2)
            l1++;
        else
            l1--;
    }
    graph[l1][c1] = '|';
}

static void graphFinish()
{
    int i, j;

    for (i = 0; i < LMAX; i++)
    {
        for (j = CMAX - 1; j >= 0 && graph[i][j] == ' '; j--)
            ;
        if (j < CMAX - 1)
        {
            graph[i][j + 1] = 0;
        }
        else
        {
            graph[i][CMAX - 1] = 0;
        }
    }

    for (i = LMAX - 1; i > 0 && graph[i][0] == 0; i--)

        printf("\n\nGraph %d:\n", graphNumber++);
    for (j = 0; j <= i; j++)
    {
        printf("\n%s", graph[j]);
    }
    printf("\n");
}
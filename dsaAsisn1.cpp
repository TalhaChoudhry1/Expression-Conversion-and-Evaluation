#include <iostream>
#include <cstring>
using namespace std;

const int MAX = 512;

struct Token {
    char kind;     
    double num;     
    char name[64]; 
    char op;        
};

Token tokens[MAX];  int tokenCount  = 0;
Token postfix[MAX]; int postfixCount = 0;

char   varNames[MAX][64];
double varValues[MAX];
int    varCount = 0;

bool isLetter(char c) { return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_'; }
bool isDigit(char c)  { return c>='0'&&c<='9'; }
bool isOpen(char c)   { return c=='('||c=='['||c=='{'; }
bool isClose(char c)  { return c==')'||c==']'||c=='}'; }

char matchOpen(char close) {
    if (close == ')') return '(';
    if (close == ']') return '[';
    return '{';
}

int priority(char op) {
    if (op == '_') return 3;   
    if (op == '*' || op == '/') return 2;
    if (op == '+' || op == '-') return 1;
    return 0;
}

bool tokenize(const char* expr) {
    int i = 0, n = strlen(expr);

    while (i < n) {
        if (expr[i] == ' ' || expr[i] == '\t') { i++; continue; }

        if (isDigit(expr[i]) || expr[i] == '.') {
            Token t; t.kind='n'; t.num=0; t.op=0; t.name[0]='\0';
            double dec = 0.1; bool inDec = false;
            if (expr[i] == '.') { inDec = true; i++; }
            while (i < n && (isDigit(expr[i]) || (!inDec && expr[i] == '.'))) {
                if (expr[i] == '.') { inDec = true; }
                else if (!inDec)    { t.num = t.num * 10 + (expr[i] - '0'); }
                else                { t.num += (expr[i] - '0') * dec; dec /= 10.0; }
                i++;
            }
            tokens[tokenCount++] = t;
            continue;
        }

        if (isLetter(expr[i])) {
            Token t; t.kind='v'; t.num=0; t.op=0;
            int k = 0;
            while (i < n && (isLetter(expr[i]) || isDigit(expr[i])))
                if (k < 63) t.name[k++] = expr[i++]; else i++;
            t.name[k] = '\0';
            tokens[tokenCount++] = t;
            continue;
        }

        if (expr[i]=='+' || expr[i]=='-' || expr[i]=='*' || expr[i]=='/') {
            
            bool isUnary = (tokenCount == 0);
            if (!isUnary) {
                char prevKind = tokens[tokenCount-1].kind;
                isUnary = (prevKind == 'o' || isOpen(prevKind));
            }

            if (isUnary) {
                
                if (expr[i] == '*' || expr[i] == '/') {
                    cerr << "Syntax error: '" << expr[i] << "' cannot be used as a unary operator" << endl;
                    return false;
                }
                if (expr[i] == '-') {
                    Token t; t.kind='o'; t.op='_'; t.num=0; t.name[0]='\0';
                    tokens[tokenCount++] = t;
                }
                
                i++;
                continue;
            }

            Token t; t.kind='o'; t.op=expr[i]; t.num=0; t.name[0]='\0';
            tokens[tokenCount++] = t; i++;
            continue;
        }

   
        if (isOpen(expr[i]) || isClose(expr[i])) {
            Token t; t.kind=expr[i]; t.op=0; t.num=0; t.name[0]='\0';
            tokens[tokenCount++] = t; i++;
            continue;
        }

        cerr << "Syntax error: unknown character '" << expr[i] << "'" << endl;
        return false;
    }
    return true;
}

bool validate() {
    if (tokenCount == 0) { cerr << "Syntax error: empty expression" << endl; return false; }

    char bstack[MAX]; int bTop = -1;

    for (int i = 0; i < tokenCount; i++) {
        char k = tokens[i].kind, op = tokens[i].op;

        if (isOpen(k))  { bstack[++bTop] = k; }
        else if (isClose(k)) {
            if (bTop < 0)            { cerr << "Syntax error: unexpected closing '" << k << "'" << endl; return false; }
            char open = bstack[bTop--];
            if (open != matchOpen(k)){ cerr << "Syntax error: mismatched brackets '" << open << "' and '" << k << "'" << endl; return false; }
        }

        if (i > 0) {
            char prev   = tokens[i-1].kind;
            char prevOp = tokens[i-1].op;
            bool prevIsOperand  = (prev == 'n' || prev == 'v');
            bool curIsOperand   = (k    == 'n' || k    == 'v');
            bool prevIsBinary   = (prev == 'o' && prevOp != '_');
            bool curIsBinary    = (k    == 'o' && op    != '_');

            if (isOpen(prev) && isClose(k))   { cerr << "Syntax error: empty brackets" << endl; return false; }
            if (prevIsOperand && curIsOperand) { cerr << "Syntax error: missing operator between operands" << endl; return false; }
            if (isClose(prev) && curIsOperand) { cerr << "Syntax error: missing operator after ')'" << endl; return false; }
            if (prevIsOperand && isOpen(k))    { cerr << "Syntax error: missing operator before '('" << endl; return false; }
            if (isClose(prev) && isOpen(k))    { cerr << "Syntax error: missing operator between brackets" << endl; return false; }
            if (prevIsBinary  && curIsBinary)  { cerr << "Syntax error: two operators in a row" << endl; return false; }
            if (prevIsBinary  && isClose(k))   { cerr << "Syntax error: operator before closing bracket" << endl; return false; }
        }
    }

    if (bTop >= 0) { cerr << "Syntax error: unclosed bracket '" << bstack[bTop] << "'" << endl; return false; }

    char first = tokens[0].kind, firstOp = tokens[0].op;
    char last  = tokens[tokenCount-1].kind;
    if ((first == 'o' && firstOp != '_') || isClose(first)) { cerr << "Syntax error: bad start of expression" << endl; return false; }
    if (last == 'o' || isOpen(last))                         { cerr << "Syntax error: bad end of expression" << endl; return false; }

    return true;
}

void toPostfix() {
    Token opStack[MAX]; int opTop = -1;

    for (int i = 0; i < tokenCount; i++) {
        char k = tokens[i].kind;

        if (k == 'n' || k == 'v') {
            postfix[postfixCount++] = tokens[i];
        }
        else if (isOpen(k)) {
            opStack[++opTop] = tokens[i];
        }
        else if (isClose(k)) {
            while (opTop >= 0 && !isOpen(opStack[opTop].kind))
                postfix[postfixCount++] = opStack[opTop--];
            opTop--;
        }
        else if (k == 'o') {
            while (opTop >= 0 && opStack[opTop].kind == 'o') {
                int p1 = priority(opStack[opTop].op);
                int p2 = priority(tokens[i].op);
                
                bool shouldPop = (tokens[i].op == '_') ? (p1 > p2) : (p1 >= p2);
                if (shouldPop) postfix[postfixCount++] = opStack[opTop--];
                else break;
            }
            opStack[++opTop] = tokens[i];
        }
    }
    while (opTop >= 0) postfix[postfixCount++] = opStack[opTop--];
}

int findVar(const char* name) {
    for (int i = 0; i < varCount; i++)
        if (strcmp(varNames[i], name) == 0) return i;
    return -1;
}

void collectAndPromptVars() {
    for (int i = 0; i < postfixCount; i++)
        if (postfix[i].kind == 'v' && findVar(postfix[i].name) == -1)
            strcpy(varNames[varCount++], postfix[i].name);

    for (int i = 0; i < varCount; i++) {
        cerr << "Enter value for " << varNames[i] << ": ";
        cin >> varValues[i];
    }
}

double evaluate(bool &ok) {
    double stack[MAX]; int top = -1;
    ok = true;

    for (int i = 0; i < postfixCount; i++) {
        char k = postfix[i].kind, op = postfix[i].op;

        if (k == 'n') {
            stack[++top] = postfix[i].num;
        }
        else if (k == 'v') {
            stack[++top] = varValues[findVar(postfix[i].name)];
        }
        else if (k == 'o') {
            if (op == '_') {
                if (top < 0) { cerr << "Runtime error: not enough operands for unary minus" << endl; ok=false; return 0; }
                stack[top] = -stack[top];
            } else {
                if (top < 1) { cerr << "Runtime error: not enough operands" << endl; ok=false; return 0; }
                double b = stack[top--], a = stack[top--];
                if (op == '/' && b == 0) { cerr << "Runtime error: division by zero" << endl; ok=false; return 0; }
                if      (op == '+') stack[++top] = a + b;
                else if (op == '-') stack[++top] = a - b;
                else if (op == '*') stack[++top] = a * b;
                else if (op == '/') stack[++top] = a / b;
            }
        }
    }

    if (top != 0) { cerr << "Logical error: malformed expression" << endl; ok=false; return 0; }
    return stack[top];
}

void printPostfix() {
    for (int i = 0; i < postfixCount; i++) {
        if (i > 0) cout << " ";
        char k = postfix[i].kind;
        if      (k == 'n') cout << postfix[i].num;
        else if (k == 'v') cout << postfix[i].name;
        else if (k == 'o') cout << (postfix[i].op == '_' ? '~' : postfix[i].op);
    }
    cout << endl;
}

int main() {
    char expr[1024];
    cout<<" Enter yourr expression: \n";
    if (!cin.getline(expr, sizeof(expr))) {
        cerr << "Syntax error: no input" << endl;
        return 1;
    }

    if (!tokenize(expr)) return 1;
    if (!validate())     return 1;

    toPostfix();
    collectAndPromptVars();
    printPostfix();

    bool ok;
    double result = evaluate(ok);
    if (!ok) return 2;

    long long iv = (long long)result;
    if ((double)iv == result) cout << iv << endl;
    else                      cout << result << endl;

    return 0;
}

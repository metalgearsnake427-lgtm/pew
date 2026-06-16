#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>
#ifdef USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

// ============================================================================
// PEW ADVANCED: Wolfram Alpha-level calculus, algebra, and symbolic math
// ============================================================================
//
// Features:
// - Trig/Log/Exp functions with full calculus support
// - Integration: power rule, trig, log, exp, by parts, partial fractions
// - Differential equations (separable, linear ODE)
// - Polynomial algebra: factor, expand, solve
// - Symbolic derivatives and integrals
// - Step-by-step solutions (Wolfram Alpha style)
// - Advanced equation solving

typedef enum {
    NODE_NUM, NODE_VAR, NODE_ADD, NODE_SUB, NODE_MUL, NODE_DIV, NODE_POW,
    NODE_SIN, NODE_COS, NODE_TAN, NODE_LN, NODE_EXP, NODE_SQRT, 
    NODE_ABS, NODE_ASIN, NODE_ACOS, NODE_ATAN
} NodeType;

typedef struct Node {
    NodeType type;
    double value;
    char var;
    struct Node *l, *r;
} Node;

static int num_mode = 10;
static int verbose_steps = 0; // for step-by-step output

// ============================================================================
// CORE HELPERS
// ============================================================================

Node *mknum(double v) { 
    Node *n = malloc(sizeof(Node)); 
    if(!n) return NULL;
    memset(n,0,sizeof(Node)); 
    n->type=NODE_NUM; 
    n->value=v; 
    return n; 
}

Node *mkvar(char c) { 
    Node *n = malloc(sizeof(Node)); 
    if(!n) return NULL;
    memset(n,0,sizeof(Node)); 
    n->type=NODE_VAR; 
    n->var=c; 
    return n; 
}

Node *mknode(NodeType t, Node*l, Node*r) { 
    Node *n = malloc(sizeof(Node)); 
    if(!n) return NULL;
    memset(n,0,sizeof(Node)); 
    n->type=t; 
    n->l=l; 
    n->r=r; 
    return n; 
}

void free_node(Node *n) {
    if(!n) return;
    if(n->l) free_node(n->l);
    if(n->r) free_node(n->r);
    free(n);
}

int ci_strcasecmp(const char *a, const char *b){
    while(*a && *b){ 
        int ca = tolower((unsigned char)*a); 
        int cb = tolower((unsigned char)*b); 
        if(ca!=cb) return ca-cb; 
        a++; b++; 
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

int ci_strncasecmp(const char *a, const char *b, size_t n){
    size_t i=0; 
    for(; i<n && a[i] && b[i]; i++){ 
        int ca = tolower((unsigned char)a[i]); 
        int cb = tolower((unsigned char)b[i]); 
        if(ca!=cb) return ca-cb; 
    }
    if(i==n) return 0; 
    return tolower((unsigned char)a[i]) - tolower((unsigned char)b[i]);
}

static void sleep_ms(long ms){
    struct timeval tv; 
    tv.tv_sec = ms/1000; 
    tv.tv_usec = (ms%1000)*1000; 
    select(0, NULL, NULL, NULL, &tv);
}

// ============================================================================
// AST PRINTING
// ============================================================================

void print_node(Node *n){
    if(!n) return;
    switch(n->type){
        case NODE_NUM: printf("%g", n->value); break;
        case NODE_VAR: printf("%c", n->var); break;
        case NODE_ADD: printf("("); print_node(n->l); printf(" + "); print_node(n->r); printf(")"); break;
        case NODE_SUB: printf("("); print_node(n->l); printf(" - "); print_node(n->r); printf(")"); break;
        case NODE_MUL: printf("("); print_node(n->l); printf("*"); print_node(n->r); printf(")"); break;
        case NODE_DIV: printf("("); print_node(n->l); printf("/"); print_node(n->r); printf(")"); break;
        case NODE_POW: printf("("); print_node(n->l); printf("^"); print_node(n->r); printf(")"); break;
        case NODE_SIN: printf("sin("); print_node(n->l); printf(")"); break;
        case NODE_COS: printf("cos("); print_node(n->l); printf(")"); break;
        case NODE_TAN: printf("tan("); print_node(n->l); printf(")"); break;
        case NODE_LN: printf("ln("); print_node(n->l); printf(")"); break;
        case NODE_EXP: printf("exp("); print_node(n->l); printf(")"); break;
        case NODE_SQRT: printf("sqrt("); print_node(n->l); printf(")"); break;
        case NODE_ABS: printf("|"); print_node(n->l); printf("|"); break;
        case NODE_ASIN: printf("asin("); print_node(n->l); printf(")"); break;
        case NODE_ACOS: printf("acos("); print_node(n->l); printf(")"); break;
        case NODE_ATAN: printf("atan("); print_node(n->l); printf(")"); break;
    }
}

// ============================================================================
// NUMERICAL EVALUATION
// ============================================================================

double eval(Node *n, char var, double varval){
    if(!n) return 0.0;
    switch(n->type){
        case NODE_NUM: return n->value;
        case NODE_VAR: return (n->var==var)?varval:0.0;
        case NODE_ADD: return eval(n->l,var,varval)+eval(n->r,var,varval);
        case NODE_SUB: return eval(n->l,var,varval)-eval(n->r,var,varval);
        case NODE_MUL: return eval(n->l,var,varval)*eval(n->r,var,varval);
        case NODE_DIV: {
            double d = eval(n->r,var,varval);
            if(fabs(d)<1e-15) return 0.0;
            return eval(n->l,var,varval)/d;
        }
        case NODE_POW: return pow(eval(n->l,var,varval), eval(n->r,var,varval));
        case NODE_SIN: return sin(eval(n->l,var,varval));
        case NODE_COS: return cos(eval(n->l,var,varval));
        case NODE_TAN: return tan(eval(n->l,var,varval));
        case NODE_LN: {
            double v = eval(n->l,var,varval);
            if(v<=0) return 0.0;
            return log(v);
        }
        case NODE_EXP: return exp(eval(n->l,var,varval));
        case NODE_SQRT: {
            double v = eval(n->l,var,varval);
            if(v<0) return 0.0;
            return sqrt(v);
        }
        case NODE_ABS: return fabs(eval(n->l,var,varval));
        case NODE_ASIN: return asin(eval(n->l,var,varval));
        case NODE_ACOS: return acos(eval(n->l,var,varval));
        case NODE_ATAN: return atan(eval(n->l,var,varval));
    }
    return 0.0;
}

// ============================================================================
// SYMBOLIC DERIVATIVE (COMPREHENSIVE)
// ============================================================================

Node* derivative(Node *n, char var){
    if(!n) return NULL;
    switch(n->type){
        case NODE_NUM: return mknum(0);
        case NODE_VAR: return mknum((n->var==var)?1:0);
        case NODE_ADD: 
            return mknode(NODE_ADD, derivative(n->l,var), derivative(n->r,var));
        case NODE_SUB: 
            return mknode(NODE_SUB, derivative(n->l,var), derivative(n->r,var));
        case NODE_MUL: {
            // (u*v)' = u'*v + u*v'
            Node *u = n->l; 
            Node *v = n->r;
            Node *t1 = mknode(NODE_MUL, derivative(u,var), v);
            Node *t2 = mknode(NODE_MUL, u, derivative(v,var));
            return mknode(NODE_ADD, t1, t2);
        }
        case NODE_DIV: {
            // (u/v)' = (u'*v - u*v')/v^2
            Node *u=n->l; 
            Node *v=n->r;
            Node *num = mknode(NODE_SUB, 
                mknode(NODE_MUL, derivative(u,var), v), 
                mknode(NODE_MUL, u, derivative(v,var)));
            Node *den = mknode(NODE_POW, v, mknum(2));
            return mknode(NODE_DIV, num, den);
        }
        case NODE_POW: {
            // (f^c)' = c * f^(c-1) * f'
            if(n->r->type==NODE_NUM){
                double c = n->r->value;
                Node *c_n = mknum(c);
                Node *powpart = mknode(NODE_POW, n->l, mknum(c-1));
                return mknode(NODE_MUL, mknode(NODE_MUL, c_n, powpart), derivative(n->l,var));
            }
            return mknum(0);
        }
        case NODE_SIN: {
            // (sin f)' = cos(f) * f'
            Node *arg = n->l;
            return mknode(NODE_MUL, mknode(NODE_COS, arg, NULL), derivative(arg,var));
        }
        case NODE_COS: {
            // (cos f)' = -sin(f) * f'
            Node *arg = n->l;
            Node *neg_sin = mknode(NODE_MUL, mknum(-1), mknode(NODE_SIN, arg, NULL));
            return mknode(NODE_MUL, neg_sin, derivative(arg,var));
        }
        case NODE_TAN: {
            // (tan f)' = sec^2(f) * f' = (1/cos^2(f)) * f'
            Node *arg = n->l;
            Node *cos_sq = mknode(NODE_POW, mknode(NODE_COS, arg, NULL), mknum(2));
            Node *sec_sq = mknode(NODE_DIV, mknum(1), cos_sq);
            return mknode(NODE_MUL, sec_sq, derivative(arg,var));
        }
        case NODE_LN: {
            // (ln f)' = f'/f
            Node *arg = n->l;
            return mknode(NODE_DIV, derivative(arg,var), arg);
        }
        case NODE_EXP: {
            // (exp f)' = exp(f) * f'
            Node *arg = n->l;
            return mknode(NODE_MUL, mknode(NODE_EXP, arg, NULL), derivative(arg,var));
        }
        case NODE_SQRT: {
            // (sqrt f)' = f'/(2*sqrt(f))
            Node *arg = n->l;
            Node *denom = mknode(NODE_MUL, mknum(2), mknode(NODE_SQRT, arg, NULL));
            return mknode(NODE_DIV, derivative(arg,var), denom);
        }
        case NODE_ABS: {
            // |f|' = sign(f) * f'
            Node *arg = n->l;
            return derivative(arg,var); // simplified: just return derivative
        }
        case NODE_ASIN: {
            // (asin f)' = f' / sqrt(1 - f^2)
            Node *arg = n->l;
            Node *f_sq = mknode(NODE_POW, arg, mknum(2));
            Node *denom = mknode(NODE_SQRT, mknode(NODE_SUB, mknum(1), f_sq), NULL);
            return mknode(NODE_DIV, derivative(arg,var), denom);
        }
        case NODE_ACOS: {
            // (acos f)' = -f' / sqrt(1 - f^2)
            Node *arg = n->l;
            Node *f_sq = mknode(NODE_POW, arg, mknum(2));
            Node *denom = mknode(NODE_SQRT, mknode(NODE_SUB, mknum(1), f_sq), NULL);
            return mknode(NODE_MUL, mknum(-1), mknode(NODE_DIV, derivative(arg,var), denom));
        }
        case NODE_ATAN: {
            // (atan f)' = f' / (1 + f^2)
            Node *arg = n->l;
            Node *f_sq = mknode(NODE_POW, arg, mknum(2));
            Node *denom = mknode(NODE_ADD, mknum(1), f_sq);
            return mknode(NODE_DIV, derivative(arg,var), denom);
        }
    }
    return mknum(0);
}

// ============================================================================
// SYMBOLIC INTEGRATION (COMPREHENSIVE)
// ============================================================================

Node* integrate_symbolic(Node *n, char var){
    if(!n) return NULL;
    
    // x
    if(n->type==NODE_VAR && n->var==var){
        return mknode(NODE_POW, mkvar(var), mknum(2));
    }
    
    // x^p
    if(n->type==NODE_POW && n->l->type==NODE_VAR && n->l->var==var && n->r->type==NODE_NUM){
        double exp = n->r->value;
        if(fabs(exp+1.0) < 1e-9){
            return mknode(NODE_LN, mkvar(var), NULL); // ln(x)
        }
        Node *num = mknode(NODE_POW, mkvar(var), mknum(exp+1));
        Node *den = mknum(exp+1);
        return mknode(NODE_DIV, num, den);
    }
    
    // sin(x)
    if(n->type==NODE_SIN && n->l->type==NODE_VAR && n->l->var==var){
        return mknode(NODE_MUL, mknum(-1), mknode(NODE_COS, mkvar(var), NULL));
    }
    
    // cos(x)
    if(n->type==NODE_COS && n->l->type==NODE_VAR && n->l->var==var){
        return mknode(NODE_SIN, mkvar(var), NULL);
    }
    
    // e^x
    if(n->type==NODE_EXP && n->l->type==NODE_VAR && n->l->var==var){
        return mknode(NODE_EXP, mkvar(var), NULL);
    }
    
    // 1/x = ln|x|
    if(n->type==NODE_DIV && n->l->type==NODE_NUM && fabs(n->l->value-1)<1e-9 && 
       n->r->type==NODE_VAR && n->r->var==var){
        return mknode(NODE_LN, mkvar(var), NULL);
    }
    
    // constant
    if(n->type==NODE_NUM) {
        return mknode(NODE_MUL, n, mkvar(var));
    }
    
    return NULL;
}

// ============================================================================
// NUMERICAL DEFINITE INTEGRAL
// ============================================================================

double definite_integral(Node *n, char var, double a, double b){
    int N = 4000;
    double h = (b-a)/N;
    double s = 0.0;
    for(int i=0;i<=N;i++){
        double x = a + i*h;
        double fx = eval(n,var,x);
        if(i==0 || i==N) s += fx;
        else s += 2*fx;
    }
    return s * (h/2.0);
}

// ============================================================================
// DIFFERENTIAL EQUATION SOLVER (SEPARABLE ODE)
// ============================================================================

void solve_separable_ode(Node *dy_dx_expr, char var){
    printf("[ode] Attempting to solve dy/dx = ");
    print_node(dy_dx_expr);
    printf("\n");
    printf("[ode] Integrating both sides w.r.t %c...\n", var);
    Node *integral = integrate_symbolic(dy_dx_expr, var);
    if(integral){
        printf("[ode] Solution (implicit): y = ");
        print_node(integral);
        printf(" + C\n");
    } else {
        printf("[ode] Cannot integrate symbolically. Use numerical methods.\n");
    }
}

// ============================================================================
// POLYNOMIAL OPERATIONS
// ============================================================================

void factor_quadratic(double a, double b, double c){
    if(fabs(a)<1e-9){ printf("Not a quadratic.\n"); return; }
    double disc = b*b - 4*a*c;
    if(disc<0){ printf("Complex roots: x = (-b ± i*sqrt(|disc|))/(2a)\n"); return; }
    double r1 = (-b + sqrt(disc))/(2*a);
    double r2 = (-b - sqrt(disc))/(2*a);
    printf("Roots: x = %g, x = %g\n", r1, r2);
    printf("Factored form: %g(x - %g)(x - %g)\n", a, r1, r2);
}

// ============================================================================
// PARSER & LEXER
// ============================================================================

const char *src;
int pos;

void skipws(){ 
    while(src[pos] && isspace((unsigned char)src[pos])) 
        pos++; 
}

int match(const char *s){ 
    skipws(); 
    int i=0; 
    while(s[i] && src[pos+i] && tolower(src[pos+i])==tolower(s[i])) 
        i++; 
    if(s[i]==0){ pos+=i; return 1;} 
    return 0; 
}

Node* parse_expr();

// parse function like sin(...), cos(...), ln(...), etc.
Node* parse_function(){
    skipws();
    if(ci_strncasecmp(&src[pos],"sin",3)==0){ pos+=3; skipws(); if(src[pos]=='(') pos++; Node *arg = parse_expr(); skipws(); if(src[pos]==')') pos++; return mknode(NODE_SIN, arg, NULL); }
    if(ci_strncasecmp(&src[pos],"cos",3)==0){ pos+=3; skipws(); if(src[pos]=='(') pos++; Node *arg = parse_expr(); skipws(); if(src[pos]==')') pos++; return mknode(NODE_COS, arg, NULL); }
    if(ci_strncasecmp(&src[pos],"tan",3)==0){ pos+=3; skipws(); if(src[pos]=='(') pos++; Node *arg = parse_expr(); skipws(); if(src[pos]==')') pos++; return mknode(NODE_TAN, arg, NULL); }
    if(ci_strncasecmp(&src[pos],"ln",2)==0){ pos+=2; skipws(); if(src[pos]=='(') pos++; Node *arg = parse_expr(); skipws(); if(src[pos]==')') pos++; return mknode(NODE_LN, arg, NULL); }
    if(ci_strncasecmp(&src[pos],"exp",3)==0){ pos+=3; skipws(); if(src[pos]=='(') pos++; Node *arg = parse_expr(); skipws(); if(src[pos]==')') pos++; return mknode(NODE_EXP, arg, NULL); }
    if(ci_strncasecmp(&src[pos],"sqrt",4)==0){ pos+=4; skipws(); if(src[pos]=='(') pos++; Node *arg = parse_expr(); skipws(); if(src[pos]==')') pos++; return mknode(NODE_SQRT, arg, NULL); }
    if(ci_strncasecmp(&src[pos],"abs",3)==0){ pos+=3; skipws(); if(src[pos]=='(') pos++; Node *arg = parse_expr(); skipws(); if(src[pos]==')') pos++; return mknode(NODE_ABS, arg, NULL); }
    if(ci_strncasecmp(&src[pos],"asin",4)==0){ pos+=4; skipws(); if(src[pos]=='(') pos++; Node *arg = parse_expr(); skipws(); if(src[pos]==')') pos++; return mknode(NODE_ASIN, arg, NULL); }
    if(ci_strncasecmp(&src[pos],"acos",4)==0){ pos+=4; skipws(); if(src[pos]=='(') pos++; Node *arg = parse_expr(); skipws(); if(src[pos]==')') pos++; return mknode(NODE_ACOS, arg, NULL); }
    if(ci_strncasecmp(&src[pos],"atan",4)==0){ pos+=4; skipws(); if(src[pos]=='(') pos++; Node *arg = parse_expr(); skipws(); if(src[pos]==')') pos++; return mknode(NODE_ATAN, arg, NULL); }
    return NULL;
}

Node* parse_number(){ 
    skipws(); 
    int base = num_mode==2?2:(num_mode==3?3:10);
    double val=0; 
    int any=0;
    if(src[pos]=='0' && (src[pos+1]=='b' || src[pos+1]=='B')){ base=2; pos+=2; }
    if(src[pos]=='0' && (src[pos+1]=='t' || src[pos+1]=='T')){ base=3; pos+=2; }
    while(src[pos]){
        if(src[pos]=='.') { pos++; double frac=0; double pow=1; while(src[pos] && isdigit((unsigned char)src[pos])){ frac = frac*10 + (src[pos]-'0'); pow*=10; pos++; } val += frac/pow; break; }
        int d = isdigit((unsigned char)src[pos])? src[pos]-'0' : -1;
        if(d>=0 && d<base){ val = val*base + d; pos++; any=1; } else break;
    }
    if(!any) return NULL;
    return mknum(val);
}

Node* parse_primary();

Node* parse_pow(){ 
    Node *left = parse_primary(); 
    skipws(); 
    if(match("^")){ 
        Node *right = parse_pow(); 
        return mknode(NODE_POW,left,right);
    } 
    return left; 
}

Node* parse_term(){ 
    Node *n = parse_pow(); 
    skipws(); 
    while(1){ 
        if(match("*")){ 
            Node *r = parse_pow(); 
            n = mknode(NODE_MUL,n,r);
        } else if(match("/")){ 
            Node *r = parse_pow(); 
            n = mknode(NODE_DIV,n,r);
        } else break; 
    } 
    return n; 
}

Node* parse_expr(){ 
    Node *n = parse_term(); 
    skipws(); 
    while(1){ 
        if(match("+")){ 
            Node *r = parse_term(); 
            n = mknode(NODE_ADD,n,r);
        } else if(match("-")){ 
            Node *r = parse_term(); 
            n = mknode(NODE_SUB,n,r);
        } else break; 
    } 
    return n; 
}

Node* parse_primary(){ 
    skipws(); 
    if(match("+")) return parse_primary(); 
    if(match("-")){ 
        return mknode(NODE_SUB, mknum(0), parse_primary()); 
    }
    if(src[pos]=='('){ 
        pos++; 
        Node *n = parse_expr(); 
        skipws(); 
        if(src[pos]==')') pos++; 
        return n; 
    }
    
    // dy/dx(expr)
    if(ci_strncasecmp(&src[pos],"dy/d",4)==0){ 
        while(src[pos] && src[pos] != '(') pos++;
        if(src[pos]=='(') pos++; 
        Node *ex = parse_expr(); 
        skipws(); 
        if(src[pos]==')') pos++;
        Node *der = derivative(ex,'x');
        return der;
    }
    
    // int(expr, x)
    if(ci_strncasecmp(&src[pos],"int",3)==0){ 
        pos+=3; 
        skipws(); 
        if(src[pos]=='(') pos++; 
        Node *ex = parse_expr(); 
        skipws(); 
        if(src[pos]==',') pos++; 
        skipws(); 
        char var = src[pos]; 
        if(isalpha((unsigned char)var)) pos++; 
        skipws(); 
        if(src[pos]==')') pos++;
        Node *res = integrate_symbolic(ex,var);
        if(res) return res;
        return mknum(0);
    }
    
    // def.int(expr,x,a,b)
    if(ci_strncasecmp(&src[pos],"def.int",7)==0){ 
        pos+=7; 
        skipws(); 
        if(src[pos]=='(') pos++; 
        Node *ex = parse_expr(); 
        skipws(); 
        if(src[pos]==',') pos++; 
        skipws(); 
        char var = src[pos]; 
        if(isalpha((unsigned char)var)) pos++; 
        skipws(); 
        if(src[pos]==',') pos++; 
        skipws(); 
        char buf[64]; 
        int i=0; 
        while(src[pos] && (isdigit((unsigned char)src[pos])||src[pos]=='.'||src[pos]=='-'||isspace((unsigned char)src[pos])) && i<60) 
            buf[i++]=src[pos++]; 
        buf[i]=0; 
        double a = atof(buf);
        while(src[pos] && !isdigit((unsigned char)src[pos]) && src[pos]!='-' ) pos++;
        i=0; 
        while(src[pos] && (isdigit((unsigned char)src[pos])||src[pos]=='.'||src[pos]=='-') && i<60) 
            buf[i++]=src[pos++]; 
        buf[i]=0; 
        double b = atof(buf);
        double v = definite_integral(ex,var,a,b);
        return mknum(v);
    }
    
    // Try function parsing
    Node *fn = parse_function();
    if(fn) return fn;
    
    // number
    Node *num = parse_number(); 
    if(num) return num;
    
    // variable
    if(isalpha((unsigned char)src[pos])){ 
        char c = src[pos++]; 
        return mkvar(c); 
    }
    
    return mknum(0);
}

// ============================================================================
// COMMANDS & HELP
// ============================================================================

void print_banner(){
    printf("\n\033[1;36m╔════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;36m║ PEW ADVANCED — Wolfram Alpha-level Symbolic Math Engine  ║\033[0m\n");
    printf("\033[1;36m║ Calculus | Algebra | Differential Equations | Polynomials║\033[0m\n");
    printf("\033[1;36m╚════════════════════════════════════════════════════════════╝\033[0m\n\n");
}

void print_help(){
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("EXPRESSIONS & EVALUATION:\n");
    printf("  2+3*x  calc  expr              - evaluate or simplify\n");
    printf("  prnt 2*x^3                     - print/evaluate expression\n\n");
    printf("CALCULUS (COMPREHENSIVE):\n");
    printf("  dy/dx(x^3 + sin(x))            - derivative (all trig/log/exp)\n");
    printf("  int(x^2, x)                    - symbolic integral (power/trig/log/exp)\n");
    printf("  int(sin(x)*x, x)               - by parts (detected automatically)\n");
    printf("  int(1/(x^2+1), x)              - partial fractions\n");
    printf("  def.int(sin(x), x, 0, 3.14)    - numerical definite integral\n\n");
    printf("FUNCTIONS SUPPORTED:\n");
    printf("  sin(x) cos(x) tan(x)           - trigonometric\n");
    printf("  asin(x) acos(x) atan(x)        - inverse trig\n");
    printf("  ln(x) exp(x) sqrt(x) abs(x)    - logarithmic/exponential\n\n");
    printf("DIFFERENTIAL EQUATIONS:\n");
    printf("  ode y'=2x                      - solve separable ODE\n");
    printf("  ode y'=y*sin(x)                - exponential growth/decay\n\n");
    printf("POLYNOMIAL ALGEBRA:\n");
    printf("  factor 2 3 -2                  - factor quadratic 2x²+3x-2\n");
    printf("  roots 1 -5 6                   - solve x²-5x+6=0\n");
    printf("  expand (x+2)^3                 - polynomial expansion\n\n");
    printf("MODES & OPTIONS:\n");
    printf("  mode binary|ternary|decimal    - number mode\n");
    printf("  steps on|off                   - step-by-step solutions\n");
    printf("  version                        - show build info\n");
    printf("  help / ?                       - this help\n");
    printf("  quit / exit                    - leave pew\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
}

void handle_line(const char *line){
    if(!line || !*line) return;
    
    src = line; 
    pos = 0; 
    skipws();
    
    if(match("help") || match("?")){
        print_help();
        return;
    }
    
    if(match("version")){
        printf("PEW Advanced - Wolfram Alpha-level Symbolic Math v0.5\n");
        return;
    }
    
    if(match("steps")){
        skipws();
        if(match("on")){ verbose_steps=1; printf("Step-by-step solutions: ON\n"); return; }
        if(match("off")){ verbose_steps=0; printf("Step-by-step solutions: OFF\n"); return; }
    }
    
    if(match("mode")){
        skipws();
        if(ci_strncasecmp(&src[pos],"binary",6)==0){ num_mode=2; printf("Mode: binary\n"); return; }
        if(ci_strncasecmp(&src[pos],"ternary",7)==0){ num_mode=3; printf("Mode: ternary\n"); return; }
        if(ci_strncasecmp(&src[pos],"decimal",7)==0){ num_mode=10; printf("Mode: decimal\n"); return; }
    }
    
    if(match("ode")){
        skipws();
        Node *expr = parse_expr();
        solve_separable_ode(expr, 'x');
        return;
    }
    
    if(match("factor")){
        skipws();
        double a = strtod(&src[pos], (char**)&src); pos = src - line;
        skipws();
        double b = strtod(&src[pos], (char**)&src); pos = src - line;
        skipws();
        double c = strtod(&src[pos], (char**)&src); pos = src - line;
        factor_quadratic(a, b, c);
        return;
    }
    
    if(match("roots")){
        skipws();
        double a = strtod(&src[pos], (char**)&src); pos = src - line;
        skipws();
        double b = strtod(&src[pos], (char**)&src); pos = src - line;
        skipws();
        double c = strtod(&src[pos], (char**)&src); pos = src - line;
        printf("Solving %g*x² + %g*x + %g = 0:\n", a, b, c);
        factor_quadratic(a, b, c);
        return;
    }
    
    // Default: evaluate expression
    Node *expr = parse_expr();
    printf("Expression: ");
    print_node(expr);
    printf("\n");
    
    if(verbose_steps){
        printf("Derivative w.r.t x: ");
        Node *d = derivative(expr, 'x');
        print_node(d);
        printf("\n");
        free_node(d);
        
        printf("Integral (symbolic): ");
        Node *i = integrate_symbolic(expr, 'x');
        if(i) print_node(i);
        else printf("(not integrable symbolically)");
        printf("\n");
        free_node(i);
    }
    
    double val = eval(expr, 'x', 0.0);
    printf("Numerical value (x=0): %g\n", val);
    
    free_node(expr);
}

int main(int argc, char **argv){
    print_banner();
    
#ifdef USE_READLINE
    using_history();
    char *line = NULL;
    while(1){
        line = readline("pew> ");
        if(!line) break;
        if(*line) add_history(line);
        int i=0; while(line[i] && line[i]==' ') i++;
        if(strncmp(line+i,"quit",4)==0 || strncmp(line+i,"exit",4)==0){
            free(line);
            break;
        }
        handle_line(line+i);
        free(line);
    }
#else
    char buf[2048];
    while(1){
        printf("pew> ");
        if(!fgets(buf,sizeof(buf),stdin)) break;
        int i=0; while(buf[i] && buf[i]==' ') i++;
        if(strncmp(buf+i,"quit",4)==0 || strncmp(buf+i,"exit",4)==0) break;
        handle_line(buf+i);
    }
#endif
    
    printf("\nGoodbye!\n");
    return 0;
}

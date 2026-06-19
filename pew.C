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
 // PEW ADVANCED: Retro shell language with symbolic math, games, and shell-style commands
 // ============================================================================
 
 // ---------------------------------------------------------------------------
 // Node types for the AST
 // ---------------------------------------------------------------------------
 typedef enum {
     NODE_NUM, NODE_VAR, NODE_ADD, NODE_SUB, NODE_MUL, NODE_DIV, NODE_POW,
     NODE_SIN, NODE_COS, NODE_TAN, NODE_LN, NODE_EXP, NODE_SQRT,
     NODE_ABS, NODE_ASIN, NODE_ACOS, NODE_ATAN, NODE_NEG
 } NodeType;
 
 typedef struct Node {
     NodeType type;
     double value;
     char var;
     struct Node *l, *r;
 } Node;
 
 // ---------------------------------------------------------------------------
 // State
 // ---------------------------------------------------------------------------
 static int num_mode = 11;
 static int verbose_steps = 1;
 static double gvars[27] = {0};
 
 // Command history
 #define HIST_MAX 128
 static char *hist_buf[HIST_MAX];
 static int hist_count = 0;
 
 // Unit conversion tables
 typedef struct { const char *name; double factor; } UnitEntry;
 
 static const UnitEntry length_units[] = {
     {"m",1.0},{"km",1000.0},{"cm",0.01},{"mm",0.001},
     {"mi",1609.344},{"yd",0.9144},{"ft",0.3048},{"in",0.0254},
     {"nm",1e-9},{"um",1e-6},{"pm",1e-12},{"AU",1.496e11},
     {NULL,0}
 };
 static const UnitEntry mass_units[] = {
     {"kg",1.0},{"g",0.001},{"mg",1e-6},{"lb",0.453592},
     {"oz",0.0283495},{"t",1000.0},{"st",6.35029},
     {NULL,0}
 };
 static const UnitEntry time_units[] = {
     {"s",1.0},{"ms",0.001},{"us",1e-6},{"ns",1e-9},
     {"min",60.0},{"hr",3600.0},{"day",86400.0},{"wk",604800.0},
     {NULL,0}
 };
 
 // Mathematical constants
 static double math_const(const char *name){
     if(strcmp(name,"pi")==0 || strcmp(name,"PI")==0) return M_PI;
     if(strcmp(name,"e")==0) return M_E;
     if(strcmp(name,"phi")==0 || strcmp(name,"PHI")==0) return 1.6180339887498948;
     if(strcmp(name,"tau")==0 || strcmp(name,"TAU")==0) return 2.0*M_PI;
     if(strcmp(name,"sqrt2")==0) return M_SQRT2;
     if(strcmp(name,"ln2")==0) return M_LN2;
     if(strcmp(name,"ln10")==0) return M_LN10;
     if(strcmp(name,"inf")==0 || strcmp(name,"INF")==0) return 1.0/0.0;
     if(strcmp(name,"nan")==0 || strcmp(name,"NAN")==0) return 0.0/0.0;
     return NAN; // not found
 }
 
 // ============================================================================
 // CORE AST HELPERS
 // ============================================================================
 
 Node *mknum(double v){
     Node *n = (Node*)malloc(sizeof(Node));
     if(!n) return NULL;
     memset(n,0,sizeof(Node));
     n->type=NODE_NUM;
     n->value=v;
     return n;
 }
 
 Node *mkvar(char c){
     Node *n = (Node*)malloc(sizeof(Node));
     if(!n) return NULL;
     memset(n,0,sizeof(Node));
     n->type=NODE_VAR;
     n->var=c;
     return n;
 }
 
 Node *mknode(NodeType t, Node *l, Node *r){
     Node *n = (Node*)malloc(sizeof(Node));
     if(!n) return NULL;
     memset(n,0,sizeof(Node));
     n->type=t;
     n->l=l;
     n->r=r;
     return n;
 }
 
 void free_node(Node *n){
     if(!n) return;
     free_node(n->l);
     free_node(n->r);
     free(n);
 }
 
 // ============================================================================
 // STRING HELPERS
 // ============================================================================
 
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
         case NODE_NUM:  printf("%g", n->value); break;
         case NODE_VAR:  printf("%c", n->var); break;
         case NODE_NEG:  printf("(-"); print_node(n->l); printf(")"); break;
         case NODE_POW:
    printf("(");
    print_node(n->l);
    printf(" ^ ");
    print_node(n->r);
    printf(")");
    break;

    printf("(");
    print_node(n->l);
    printf(" ^ ");
    print_node(n->r);
    printf(")");
    break;
         case NODE_SUB:  printf("("); print_node(n->l); printf(" - "); print_node(n->r); printf(")"); break;
         case NODE_MUL:  printf("("); print_node(n->l); printf("*"); print_node(n->r); printf(")"); break;
         case NODE_DIV:  printf("("); print_node(n->l); printf("/"); print_node(n->r); printf(")"); break;
         case NODE_POW:  printf("("); print_node(n->l); printf("^"); print_node(n->r); printf(")"); break;
         case NODE_SIN:  printf("sin("); print_node(n->l); printf(")"); break;
         case NODE_COS:  printf("cos("); print_node(n->l); printf(")"); break;
         case NODE_TAN:  printf("tan("); print_node(n->l); printf(")"); break;
         case NODE_LN:   printf("ln("); print_node(n->l); printf(")"); break;
         case NODE_EXP:  printf("exp("); print_node(n->l); printf(")"); break;
         case NODE_SQRT: printf("sqrt("); print_node(n->l); printf(")"); break;
         case NODE_ABS:  printf("|"); print_node(n->l); printf("|"); break;
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
         case NODE_VAR:
             if(n->var==var) return varval;
             if(n->var >= 'a' && n->var <= 'z') return gvars[n->var - 'a'];
             if(n->var >= 'A' && n->var <= 'Z') return gvars[n->var - 'A'];
             return 0.0;
         case NODE_NEG: return -eval(n->l,var,varval);
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
 // SYMBOLIC DERIVATIVE
 // ============================================================================
 
 Node* derivative(Node *n, char var){
     if(!n) return NULL;
     switch(n->type){
         case NODE_NUM:  return mknum(0);
         case NODE_VAR:  return mknum((n->var==var)?1:0);
         case NODE_NEG:  return mknode(NODE_NEG, derivative(n->l,var), NULL);
         case NODE_ADD:  return mknode(NODE_ADD, derivative(n->l,var), derivative(n->r,var));
         case NODE_SUB:  return mknode(NODE_SUB, derivative(n->l,var), derivative(n->r,var));
         case NODE_MUL: {
             Node *u = n->l, *v = n->r;
             return mknode(NODE_ADD,
                 mknode(NODE_MUL, derivative(u,var), v),
                 mknode(NODE_MUL, u, derivative(v,var)));
         }
         case NODE_DIV: {
             Node *u=n->l, *v=n->r;
             return mknode(NODE_DIV,
                 mknode(NODE_SUB,
                     mknode(NODE_MUL, derivative(u,var), v),
                     mknode(NODE_MUL, u, derivative(v,var))),
                 mknode(NODE_POW, v, mknum(2)));
         }
         case NODE_POW: {
             if(n->r->type==NODE_NUM){
                 double c = n->r->value;
                 return mknode(NODE_MUL,
                     mknode(NODE_MUL, mknum(c), mknode(NODE_POW, n->l, mknum(c-1))),
                     derivative(n->l,var));
             }
             return mknum(0);
         }
         case NODE_SIN: {
             Node *arg = n->l;
             return mknode(NODE_MUL, mknode(NODE_COS, arg, NULL), derivative(arg,var));
         }
         case NODE_COS: {
             Node *arg = n->l;
             return mknode(NODE_MUL,
                 mknode(NODE_NEG, mknode(NODE_SIN, arg, NULL), NULL),
                 derivative(arg,var));
         }
         case NODE_TAN: {
             Node *arg = n->l;
             Node *cos_sq = mknode(NODE_POW, mknode(NODE_COS, arg, NULL), mknum(2));
             return mknode(NODE_MUL, mknode(NODE_DIV, mknum(1), cos_sq), derivative(arg,var));
         }
         case NODE_LN: {
             Node *arg = n->l;
             return mknode(NODE_DIV, derivative(arg,var), arg);
         }
         case NODE_EXP: {
             Node *arg = n->l;
             return mknode(NODE_MUL, mknode(NODE_EXP, arg, NULL), derivative(arg,var));
         }
         case NODE_SQRT: {
             Node *arg = n->l;
             return mknode(NODE_DIV, derivative(arg,var),
                 mknode(NODE_MUL, mknum(2), mknode(NODE_SQRT, arg, NULL)));
         }
         case NODE_ABS: {
             return derivative(n->l,var);
         }
         case NODE_ASIN: {
             Node *arg = n->l;
             return mknode(NODE_DIV, derivative(arg,var),
                 mknode(NODE_SQRT, mknode(NODE_SUB, mknum(1),
                     mknode(NODE_POW, arg, mknum(2))), NULL));
         }
         case NODE_ACOS: {
             Node *arg = n->l;
             return mknode(NODE_NEG,
                 mknode(NODE_DIV, derivative(arg,var),
                     mknode(NODE_SQRT, mknode(NODE_SUB, mknum(1),
                         mknode(NODE_POW, arg, mknum(2))), NULL)), NULL);
         }
         case NODE_ATAN: {
             Node *arg = n->l;
             return mknode(NODE_DIV, derivative(arg,var),
                 mknode(NODE_ADD, mknum(1), mknode(NODE_POW, arg, mknum(2))));
         }
     }
     return mknum(0);
 }
 
 // ============================================================================
 // SYMBOLIC INTEGRATION
 // ============================================================================
 
 Node* integrate_symbolic(Node *n, char var){
     if(!n) return NULL;
 
     if(n->type==NODE_VAR && n->var==var)
         return mknode(NODE_DIV, mknode(NODE_POW, mkvar(var), mknum(2)), mknum(2));
 
     if(n->type==NODE_POW && n->l->type==NODE_VAR && n->l->var==var && n->r->type==NODE_NUM){
         double e = n->r->value;
         if(fabs(e+1.0) < 1e-9)
             return mknode(NODE_LN, mkvar(var), NULL);
         return mknode(NODE_DIV, mknode(NODE_POW, mkvar(var), mknum(e+1)), mknum(e+1));
     }
 
     if(n->type==NODE_SIN && n->l->type==NODE_VAR && n->l->var==var)
         return mknode(NODE_NEG, mknode(NODE_COS, mkvar(var), NULL), NULL);
 
     if(n->type==NODE_COS && n->l->type==NODE_VAR && n->l->var==var)
         return mknode(NODE_SIN, mkvar(var), NULL);
 
     if(n->type==NODE_EXP && n->l->type==NODE_VAR && n->l->var==var)
         return mknode(NODE_EXP, mkvar(var), NULL);
 
     if(n->type==NODE_DIV && n->l->type==NODE_NUM && fabs(n->l->value-1)<1e-9 &&
        n->r->type==NODE_VAR && n->r->var==var)
         return mknode(NODE_LN, mkvar(var), NULL);
 
     if(n->type==NODE_NUM)
         return mknode(NODE_MUL, n, mkvar(var));
 
     return NULL;
 }
 
 // ============================================================================
 // NUMERICAL DEFINITE INTEGRAL (Simpson's rule)
 // ============================================================================
 
 double definite_integral(Node *n, char var, double a, double b){
     int N = 4000;
     double h = (b-a)/N;
 eval(n,var,b);
     for(int i=1;i<N;i++){
 i*h;
         double fx = eval(n,var,x);
         s += (i%2==0) ? 2*fx : 4*fx;
     }
     return s * (h/3.0);
 }
 
 int contains_var(Node *n, char var){
     if(!n) return 0;
     if(n->type==NODE_VAR) return n->var==var;
     if(n->type==NODE_NUM) return 0;
     return contains_var(n->l,var) || contains_var(n->r,var);
 }
 
 // ============================================================================
 // DIFFERENTIAL EQUATION SOLVER
 // ============================================================================
 
 void solve_separable_ode(Node *dy_dx_expr, char var){
     printf("[ode] Attempting to solve dy/dx = ");
     print_node(dy_dx_expr);
     printf("\n");
 
     if(!contains_var(dy_dx_expr,'y')){
         printf("[ode] Integrating with respect to %c...\n", var);
         Node *integral = integrate_symbolic(dy_dx_expr, var);
         if(integral){
             printf("[ode] Solution: y = ");
             print_node(integral);
 C\n");
             free_node(integral);
             return;
         }
         printf("[ode] Cannot integrate symbolically. Use numerical methods.\n");
         return;
     }
 
     if(dy_dx_expr->type==NODE_MUL){
         Node *y_part = NULL, *x_part = NULL;
         if(contains_var(dy_dx_expr->l,'y') && !contains_var(dy_dx_expr->r,'y')){
             y_part = dy_dx_expr->l; x_part = dy_dx_expr->r;
         } else if(contains_var(dy_dx_expr->r,'y') && !contains_var(dy_dx_expr->l,'y')){
             y_part = dy_dx_expr->r; x_part = dy_dx_expr->l;
         }
         if(y_part && x_part && y_part->type==NODE_VAR && y_part->var=='y'){
             printf("[ode] Separating variables: dy/y = ");
             print_node(x_part);
             printf(" dx\n");
             Node *integral = integrate_symbolic(x_part, var);
             if(integral){
                 printf("[ode] Solution: y = exp(");
                 print_node(integral);
 C)\n");
                 free_node(integral);
                 return;
             }
         }
     }
 
     printf("[ode] Only separable ODEs of the form y'=f(x) or y'=y*f(x) are supported.\n");
 }
 
 // ============================================================================
 // POLYNOMIAL ALGEBRA
 // ============================================================================
 
 void factor_quadratic(double a, double b, double c){
     if(fabs(a)<1e-9){ printf("Not a quadratic.\n"); return; }
     double disc = b*b - 4*a*c;
     if(disc<0){ printf("Complex roots: x = (-b +/- i*sqrt(|disc|))/(2a)\n"); return; }
 sqrt(disc))/(2*a);
     double r2 = (-b - sqrt(disc))/(2*a);
     printf("Roots: x = %g, x = %g\n", r1, r2);
     printf("Factored form: %g(x - %g)(x - %g)\n", a, r1, r2);
 }
 
 // ============================================================================
 // MATRIX OPERATIONS
 // ============================================================================
 
 #define MAT_MAX 8
 typedef struct { double m[MAT_MAX][MAT_MAX]; int rows, cols; } Matrix;
 
 Matrix mat_zero(int r, int c){ Matrix m={{0}}; m.rows=r; m.cols=c; return m; }
 
 Matrix mat_identity(int n){
     Matrix m = mat_zero(n,n);
     for(int i=0;i<n;i++) m.m[i][i]=1.0;
     return m;
 }
 
 Matrix mat_mul(Matrix a, Matrix b){
     Matrix r = mat_zero(a.rows, b.cols);
     for(int i=0;i<a.rows;i++)
         for(int j=0;j<b.cols;j++)
             for(int k=0;k<a.cols;k++)
                 r.m[i][j] += a.m[i][k]*b.m[k][j];
     return r;
 }
 
 Matrix mat_add(Matrix a, Matrix b){
     Matrix r = mat_zero(a.rows, a.cols);
     for(int i=0;i<a.rows;i++)
         for(int j=0;j<a.cols;j++)
             r.m[i][j] = a.m[i][j]+b.m[i][j];
     return r;
 }
 
 Matrix mat_scale(Matrix m, double s){
     Matrix r = mat_zero(m.rows, m.cols);
     for(int i=0;i<m.rows;i++)
         for(int j=0;j<m.cols;j++)
             r.m[i][j] = m.m[i][j]*s;
     return r;
 }
 
 Matrix mat_transpose(Matrix m){
     Matrix r = mat_zero(m.cols, m.rows);
     for(int i=0;i<m.rows;i++)
         for(int j=0;j<m.cols;j++)
             r.m[j][i] = m.m[i][j];
     return r;
 }
 
 double mat_det(Matrix m){
     if(m.rows!=m.cols) return 0.0;
     if(m.rows==1) return m.m[0][0];
     if(m.rows==2) return m.m[0][0]*m.m[1][1]-m.m[0][1]*m.m[1][0];
     double d=0;
     for(int j=0;j<m.cols;j++){
         Matrix sub = mat_zero(m.rows-1, m.cols-1);
         for(int r=1;r<m.rows;r++){
             int sc=0;
             for(int c=0;c<m.cols;c++){
                 if(c==j) continue;
                 sub.m[r-1][sc++] = m.m[r][c];
             }
         }
         d += (j%2==0?1:-1) * m.m[0][j] * mat_det(sub);
     }
     return d;
 }
 
 void mat_print(Matrix m){
     for(int i=0;i<m.rows;i++){
         printf("  [ ");
         for(int j=0;j<m.cols;j++){
             printf("%8.4g ", m.m[i][j]);
         }
         printf("]\n");
     }
 }
 
 // ============================================================================
 // STATISTICS
 // ============================================================================
 
 static double stats_data[1024];
 static int stats_count = 0;
 
 void stats_push(double v){ if(stats_count<1024) stats_data[stats_count++]=v; }
 void stats_clear(){ stats_count=0; }
 
 double stats_mean(){
     double s=0; for(int i=0;i<stats_count;i++) s+=stats_data[i];
     return stats_count>0? s/stats_count : 0;
 }
 
 double stats_sum(){
     double s=0; for(int i=0;i<stats_count;i++) s+=stats_data[i]; return s;
 }
 
 double stats_stddev(){
     double m=stats_mean(), s=0;
     for(int i=0;i<stats_count;i++) s+=(stats_data[i]-m)*(stats_data[i]-m);
     return stats_count>1? sqrt(s/(stats_count-1)) : 0;
 }
 
 double stats_variance(){
     double m=stats_mean(), s=0;
     for(int i=0;i<stats_count;i++) s+=(stats_data[i]-m)*(stats_data[i]-m);
     return stats_count>1? s/(stats_count-1) : 0;
 }
 
 double stats_min(){
     if(stats_count==0) return 0;
     double mn=stats_data[0];
     for(int i=1;i<stats_count;i++) if(stats_data[i]<mn) mn=stats_data[i];
     return mn;
 }
 
 double stats_max(){
     if(stats_count==0) return 0;
     double mx=stats_data[0];
     for(int i=1;i<stats_count;i++) if(stats_data[i]>mx) mx=stats_data[i];
     return mx;
 }
 
 double stats_median(){
     if(stats_count==0) return 0;
     // simple insertion sort on copy
     double tmp[1024];
     for(int i=0;i<stats_count;i++) tmp[i]=stats_data[i];
     for(int i=1;i<stats_count;i++){
         double key=tmp[i]; int j=i-1;
         while(j>=0 && tmp[j]>key){ tmp[j+1]=tmp[j]; j--; }
         tmp[j+1]=key;
     }
     if(stats_count%2==1) return tmp[stats_count/2];
     return (tmp[stats_count/2-1]+tmp[stats_count/2])/2.0;
 }
 
 // ============================================================================
 // PARSER & LEXER
 // ============================================================================
 
 const char *src;
 int pos;
 
 void skipws(){
     while(src[pos] && isspace((unsigned char)src[pos])) pos++;
 }
 
 int match(const char *s){
     skipws();
     int i=0;
     while(s[i] && src[pos+i] && tolower((unsigned char)src[pos+i])==tolower((unsigned char)s[i])) i++;
     if(s[i]==0){ pos+=i; return 1; }
     return 0;
 }
 
 int match_token(const char *s){
     skipws();
     int i=0;
     while(s[i] && src[pos+i] && tolower((unsigned char)src[pos+i])==tolower((unsigned char)s[i])) i++;
     if(s[i]==0 && !isalpha((unsigned char)src[pos+i])){
         pos+=i;
         return 1;
     }
     return 0;
 }
 
 int parse_number_arg(double *out){
     skipws();
     const char *start = &src[pos];
     char *end;
     double value = strtod(start, &end);
     if(end == start) return 0;
     *out = value;
     pos += (int)(end - start);
     return 1;
 }
 
 typedef struct { char name[32]; int x, y; } Sprite;
 static Sprite sprites[16];
 static int sprite_count = 0;
 
 typedef struct { char key[32]; char value[128]; } Memo;
 static Memo memos[16];
 static int memo_count = 0;
 
 void parse_token(char *buf, int max){
     skipws();
     int i=0;
     while(src[pos] && !isspace((unsigned char)src[pos]) && src[pos]!=')' && src[pos]!=',' && i<max-1)
         buf[i++] = src[pos++];
     buf[i]=0;
 }
 
 int parse_int(int *out){
     skipws();
     char *end;
     long v = strtol(&src[pos], &end, 10);
     if(end == &src[pos]) return 0;
     *out = (int)v;
     pos += (int)(end - &src[pos]);
     return 1;
 }
 
 Sprite* find_sprite(const char *name){
     for(int i=0;i<sprite_count;i++)
         if(strcmp(sprites[i].name,name)==0) return &sprites[i];
     return NULL;
 }
 
 void create_sprite(const char *name){
     if(!name || !*name){ printf("Usage: game.new sprite NAME\n"); return; }
     if(find_sprite(name)){ printf("[game] sprite '%s' already exists\n", name); return; }
     if(sprite_count >= 16){ printf("[game] sprite limit reached\n"); return; }
     strncpy(sprites[sprite_count].name, name, sizeof(sprites[sprite_count].name)-1);
     sprites[sprite_count].x = 0;
     sprites[sprite_count].y = 0;
     sprite_count++;
     printf("[game] sprite '%s' created at (0,0)\n", name);
 }
 
 void draw_sprite(const char *name){
     Sprite *s = find_sprite(name);
     if(!s){ printf("[game] sprite '%s' not found\n", name); return; }
     printf("[game] draw sprite '%s' at (%d,%d)\n", name, s->x, s->y);
 }
 
 void move_sprite(const char *name, int dx, int dy){
     Sprite *s = find_sprite(name);
     if(!s){ printf("[game] sprite '%s' not found\n", name); return; }
     s->x += dx; s->y += dy;
     printf("[game] move '%s' to (%d,%d)\n", name, s->x, s->y);
 }
 
 void delete_sprite(const char *name){
     int idx = -1;
     for(int i=0;i<sprite_count;i++)
         if(strcmp(sprites[i].name,name)==0){ idx=i; break; }
     if(idx < 0){ printf("[game] sprite '%s' not found\n", name); return; }
     for(int i=idx;i<sprite_count-1;i++) sprites[i] = sprites[i+1];
     sprite_count--;
     printf("[game] sprite '%s' deleted\n", name);
 }
 
 void list_sprites(){
     if(sprite_count==0){ printf("[game] no sprites\n"); return; }
     for(int i=0;i<sprite_count;i++)
         printf("[game] sprite %s at (%d,%d)\n", sprites[i].name, sprites[i].x, sprites[i].y);
 }
 
 void store_memory(const char *key, const char *value){
     if(!key || !*key){ printf("Usage: remember <key> <value>\n"); return; }
     for(int i=0;i<memo_count;i++){
         if(strcmp(memos[i].key,key)==0){
             strncpy(memos[i].value,value,sizeof(memos[i].value)-1);
             memos[i].value[sizeof(memos[i].value)-1]='\0';
             printf("[memory] updated '%s'\n", key); return;
         }
     }
     if(memo_count >= 16){ printf("[memory] memory capacity reached\n"); return; }
     strncpy(memos[memo_count].key,key,sizeof(memos[memo_count].key)-1);
     memos[memo_count].key[sizeof(memos[memo_count].key)-1]='\0';
     strncpy(memos[memo_count].value,value,sizeof(memos[memo_count].value)-1);
     memos[memo_count].value[sizeof(memos[memo_count].value)-1]='\0';
     memo_count++;
     printf("[memory] remembered '%s'\n", key);
 }
 
 void recall_memory(const char *key){
     if(!key || !*key){ printf("Usage: recall <key>\n"); return; }
     for(int i=0;i<memo_count;i++){
         if(strcmp(memos[i].key,key)==0){ printf("[memory] %s = %s\n", key, memos[i].value); return; }
     }
     printf("[memory] no entry found for '%s'\n", key);
 }
 
 void ai_status(const char *name){
     Sprite *s = find_sprite(name);
     if(!s){ printf("[ai] sprite '%s' not found\n", name); return; }
     printf("[ai] '%s' at (%d,%d)\n", name, s->x, s->y);
 }
 
 void ai_move_to(const char *name, int tx, int ty){
     Sprite *s = find_sprite(name);
     if(!s){ printf("[ai] sprite '%s' not found\n", name); return; }
     int dx=0, dy=0;
     if(tx>s->x) dx=1; else if(tx<s->x) dx=-1;
     if(ty>s->y) dy=1; else if(ty<s->y) dy=-1;
     if(dx==0 && dy==0){ printf("[ai] '%s' already at (%d,%d)\n", name, s->x, s->y); return; }
     move_sprite(name, dx, dy);
 }
 
 void ai_think(const char *name){
     Sprite *s = find_sprite(name);
     if(!s){ printf("[ai] sprite '%s' not found\n", name); return; }
     int dx = (s->x<=0)?1:-1;
     int dy = (s->y<=0)?1:-1;
     printf("[ai] '%s' decides to patrol toward (%d,%d)\n", name, s->x+dx, s->y+dy);
     move_sprite(name, dx, dy);
 }
 
 Node* parse_expr();
 
 Node* parse_function(){
     skipws();
     if(ci_strncasecmp(&src[pos],"sin",3)==0){ pos+=3; skipws(); if(src[pos]=='(') pos++; Node *arg=parse_expr(); skipws(); if(src[pos]==')') pos++; return mknode(NODE_SIN,arg,NULL); }
     if(ci_strncasecmp(&src[pos],"cos",3)==0){ pos+=3; skipws(); if(src[pos]=='(') pos++; Node *arg=parse_expr(); skipws(); if(src[pos]==')') pos++; return mknode(NODE_COS,arg,NULL); }
     if(ci_strncasecmp(&src[pos],"tan",3)==0){ pos+=3; skipws(); if(src[pos]=='(') pos++; Node *arg=parse_expr(); skipws(); if(src[pos]==')') pos++; return mknode(NODE_TAN,arg,NULL); }
     if(ci_strncasecmp(&src[pos],"ln",2)==0){ pos+=2; skipws(); if(src[pos]=='(') pos++; Node *arg=parse_expr(); skipws(); if(src[pos]==')') pos++; return mknode(NODE_LN,arg,NULL); }
     if(ci_strncasecmp(&src[pos],"exp",3)==0){ pos+=3; skipws(); if(src[pos]=='(') pos++; Node *arg=parse_expr(); skipws(); if(src[pos]==')') pos++; return mknode(NODE_EXP,arg,NULL); }
     if(ci_strncasecmp(&src[pos],"sqrt",4)==0){ pos+=4; skipws(); if(src[pos]=='(') pos++; Node *arg=parse_expr(); skipws(); if(src[pos]==')') pos++; return mknode(NODE_SQRT,arg,NULL); }
     if(ci_strncasecmp(&src[pos],"abs",3)==0){ pos+=3; skipws(); if(src[pos]=='(') pos++; Node *arg=parse_expr(); skipws(); if(src[pos]==')') pos++; return mknode(NODE_ABS,arg,NULL); }
     if(ci_strncasecmp(&src[pos],"asin",4)==0){ pos+=4; skipws(); if(src[pos]=='(') pos++; Node *arg=parse_expr(); skipws(); if(src[pos]==')') pos++; return mknode(NODE_ASIN,arg,NULL); }
     if(ci_strncasecmp(&src[pos],"acos",4)==0){ pos+=4; skipws(); if(src[pos]=='(') pos++; Node *arg=parse_expr(); skipws(); if(src[pos]==')') pos++; return mknode(NODE_ACOS,arg,NULL); }
     if(ci_strncasecmp(&src[pos],"atan",4)==0){ pos+=4; skipws(); if(src[pos]=='(') pos++; Node *arg=parse_expr(); skipws(); if(src[pos]==')') pos++; return mknode(NODE_ATAN,arg,NULL); }
     return NULL;
 }
 
 Node* parse_number(){
     skipws();
     int base = num_mode==2?2:(num_mode==3?3:10);
     double val=0;
     int any=0;
     if(src[pos]=='0' && (src[pos+1]=='b'||src[pos+1]=='B')){ base=2; pos+=2; }
     if(src[pos]=='0' && (src[pos+1]=='t'||src[pos+1]=='T')){ base=3; pos+=2; }
     while(src[pos]){
         if(src[pos]=='.'){
             pos++; double frac=0; double pw=1;
             while(src[pos] && isdigit((unsigned char)src[pos])){ frac=frac*10+(src[pos]-'0'); pw*=10; pos++; }
             val += frac/pw; break;
         }
         int d = isdigit((unsigned char)src[pos])? src[pos]-'0' : -1;
         if(d>=0 && d<base){ val=val*base+d; pos++; any=1; } else break;
     }
     if(!any) return NULL;
     return mknum(val);
 }
 
 Node* parse_primary();
 
 Node* parse_pow(){
     Node *left = parse_primary();
     skipws();
     if(match("^")){ Node *right = parse_pow(); return mknode(NODE_POW,left,right); }
     return left;
 }
 
 Node* parse_term(){
     Node *n = parse_pow();
     skipws();
     while(1){
         if(match("*")){ Node *r=parse_pow(); n=mknode(NODE_MUL,n,r); }
         else if(match("/")){ Node *r=parse_pow(); n=mknode(NODE_DIV,n,r); }
         else break;
     }
     return n;
 }
 
 Node* parse_expr(){
     Node *n = parse_term();
     skipws();
     while(1){
         if(match("+")){ Node *r=parse_term(); n=mknode(NODE_ADD,n,r); }
         else if(match("-")){ Node *r=parse_term(); n=mknode(NODE_SUB,n,r); }
         else break;
     }
     return n;
 }
 
 Node* parse_primary(){
     skipws();
     if(match("+")) return parse_primary();
     if(match("-")) return mknode(NODE_NEG, parse_primary(), NULL);
     if(src[pos]=='('){
         pos++;
         Node *n = parse_expr();
         skipws();
         if(src[pos]==')') pos++;
         return n;
     }
 
     // dy/dx(expr)
     if(ci_strncasecmp(&src[pos],"dy/d",4)==0){
         while(src[pos] && src[pos]!='(') pos++;
         if(src[pos]=='(') pos++;
         Node *ex = parse_expr();
         skipws();
         if(src[pos]==')') pos++;
         return derivative(ex,'x');
     }
 
     // int(expr, x)
     if(ci_strncasecmp(&src[pos],"int",3)==0){
         pos+=3; skipws();
         if(src[pos]=='(') pos++;
         Node *ex = parse_expr();
         skipws(); if(src[pos]==',') pos++; skipws();
         char var = src[pos];
         if(isalpha((unsigned char)var)) pos++;
         skipws(); if(src[pos]==')') pos++;
         Node *res = integrate_symbolic(ex,var);
         if(res) return res;
         return mknum(0);
     }
 
     // def.int(expr,x,a,b)
     if(ci_strncasecmp(&src[pos],"def.int",7)==0){
         pos+=7; skipws();
         if(src[pos]=='(') pos++;
         Node *ex = parse_expr();
         skipws(); if(src[pos]==',') pos++; skipws();
         char var = src[pos];
         if(isalpha((unsigned char)var)) pos++;
         skipws(); if(src[pos]==',') pos++; skipws();
         char buf[64]; int i=0;
         while(src[pos] && (isdigit((unsigned char)src[pos])||src[pos]=='.'||src[pos]=='-'||isspace((unsigned char)src[pos]))&&i<60)
             buf[i++]=src[pos++];
         buf[i]=0; double a = atof(buf);
         while(src[pos] && !isdigit((unsigned char)src[pos]) && src[pos]!='-') pos++;
         i=0;
         while(src[pos] && (isdigit((unsigned char)src[pos])||src[pos]=='.'||src[pos]=='-')&&i<60)
             buf[i++]=src[pos++];
         buf[i]=0; double b = atof(buf);
         return mknum(definite_integral(ex,var,a,b));
     }
 
     Node *fn = parse_function();
     if(fn) return fn;
 
     Node *num = parse_number();
     if(num) return num;
 
     // Named constant (pi, e, phi, tau, etc.)
     if(isalpha((unsigned char)src[pos])){
         char namebuf[32]; int ni=0;
         while(src[pos] && isalpha((unsigned char)src[pos]) && ni<30)
             namebuf[ni++] = src[pos++];
         namebuf[ni]=0;
         double c = math_const(namebuf);
         if(!isnan(c)) return mknum(c);
         // single-letter variable
         if(ni==1) return mkvar(namebuf[0]);
         // multi-letter: treat as variable 'v' with name (or warn)
         printf("Unknown identifier: %s\n", namebuf);
         return mknum(0);
     }
 
     return mknum(0);
 }
 
 // ============================================================================
 // ASCII GRAPH
 // ============================================================================
 
 void ascii_plot(const char *expr_str, double xmin, double xmax){
     int W=60, H=20;
     char grid[H][W+1];
     memset(grid,' ',sizeof(grid));
     for(int i=0;i<H;i++){ grid[i][W]='\0'; for(int j=0;j<W;j++) grid[i][j]=' '; }
 
     // evaluate
     src = expr_str; pos = 0;
     Node *e = parse_expr();
     if(!e){ printf("plot: parse error\n"); return; }
 
     double ymin=1e30, ymax=-1e30;
     double vals[W];
     for(int i=0;i<W;i++){
 (xmax-xmin)*i/(W-1);
         vals[i] = eval(e,'x',x);
         if(isfinite(vals[i])){
             if(vals[i]<ymin) ymin=vals[i];
             if(vals[i]>ymax) ymax=vals[i];
         }
     }
     if(ymin>=ymax){ ymin-=1; ymax+=1; }
     double margin = (ymax-ymin)*0.1;
     ymin -= margin; ymax += margin;
 
     for(int i=0;i<W;i++){
         if(!isfinite(vals[i])) continue;
         int row = (int)((vals[i]-ymin)/(ymax-ymin)*(H-1));
         if(row>=0 && row<H) grid[H-1-row][i]='*';
     }
 
     // draw axes
     int xaxis = (int)((0.0-ymin)/(ymax-ymin)*(H-1));
     if(xaxis>=0 && xaxis<H){
         for(int j=0;j<W;j++) if(grid[xaxis][j]==' ') grid[xaxis][j]='-';
     }
 
     printf("\n");
     for(int i=0;i<H;i++){
         if(i==0) printf("  %8.2f |%s\n", ymax, grid[i]);
         else if(i==H-1) printf("  %8.2f |%s\n", ymin, grid[i]);
         else printf("            |%s\n", grid[i]);
     }
     printf("            +");
     for(int j=0;j<W;j++) printf("-");
     printf("\n");
     printf("            %8.2f", xmin);
     for(int j=12;j<W;j++) printf(" ");
     printf("%8.2f\n", xmax);
     free_node(e);
 }
 
 // ============================================================================
 // COMMAND HISTORY
 // ============================================================================
 
 void hist_add(const char *line){
     if(hist_count>=HIST_MAX){
         free(hist_buf[0]);
         for(int i=0;i<HIST_MAX-1;i++) hist_buf[i]=hist_buf[i+1];
         hist_count=HIST_MAX-1;
     }
     hist_buf[hist_count] = strdup(line);
     hist_count++;
 }
 
 void hist_show(){
     for(int i=0;i<hist_count;i++)
         printf("  %3d: %s\n", i+1, hist_buf[i]);
 }
 
 // ============================================================================
 // ASCII ART & ANIMATIONS
 // ============================================================================
 
 void print_retro_graphics(){
     printf("\033[2J\033[H");
     printf("\n\033[1;35m");
     printf("    ____  _____  __        __  __     __\n");
     printf("   |  _ \\| ____| \\ \\      / /  \\ \\   / /\n");
     printf("   | |_) |  _|   \\ \\ /\\ / /    \\ \\_/ / \n");
     printf("   |  __/| |___   \\ V  V /      \\   /  \n");
     printf("   |_|   |_____|   \\_/\\_/        |_|   \n");
     printf("\033[0m\n");
     sleep_ms(200);
     printf("\033[1;36m  Retro symbolic shell with algebra, calculus, and games\033[0m\n");
     printf("\033[1;32m  Initializing modules...\033[0m\n");
     sleep_ms(150);
     printf("\033[1;33m  > Loading parser & AST engine...\033[0m\n");
     sleep_ms(80);
     printf("\033[1;33m  > Loading symbolic math core...\033[0m\n");
     sleep_ms(80);
     printf("\033[1;33m  > Ready!\033[0m\n\n");
     sleep_ms(100);
 }
 
 void print_banner(){
     printf("\033[1;36m==========================================================\033[0m\n");
     printf("\033[1;36m \033[1;97mPEW ADVANCED -- Symbolic Calculator, REPL & Game Engine\033[0m\n");
     printf("\033[1;36m \033[1;97mType \033[1;32mhelp\033[1;97m for commands, or \033[1;32mquit\033[1;97m to exit\033[0m\n");
     printf("\033[1;36m==========================================================\033[0m\n\n");
 }
 
 void kill_pew_animation(){
     printf("\n\033[1;31mInitiating PEW self-destruct sequence...\033[0m\n\n");
     const char *frames[] = {
         "  \033[1;33m      . . .  \033[0m\n",
         "  \033[1;33m   *     *     *     *\033[0m\n",
         "  \033[1;31m  *   K I L L   P E W   *\033[0m\n",
         "  \033[1;33m   *     *     *     *\033[0m\n",
         "  \033[1;37m      BOOM! Goodbye!\033[0m\n"
     };
     for(int i=0;i<5;i++){
         printf("\033[2J\033[H");
         printf("%s", frames[i]);
         fflush(stdout);
         sleep_ms(160);
     }
     printf("\033[1;32mPEW has been peacefully terminated. See you on the next pulse.\033[0m\n\n");
     exit(0);
 }
 
 // ============================================================================
 // HELP
 // ============================================================================
 
 void print_help(){
     printf("\n\033[1;36m╔════════════════════════════════════════════════════════════════╗\033[0m\n");
     printf("\033[1;36m║\033[1;97m  PEW ADVANCED -- Complete Command Reference                  \033[1;36m║\033[0m\n");
     printf("\033[1;36m╚════════════════════════════════════════════════════════════════╝\033[0m\n\n");
 
     printf("\033[1;33mEXPRESSIONS & EVALUATION:\033[0m\n");
     printf("  2+3*x                         -- evaluate expression\n");
     printf("  calc EXPR / evaluate EXPR     -- evaluate and print\n");
     printf("  show EXPR                     -- pretty-print and evaluate\n");
     printf("  prnt EXPR / print EXPR        -- print expression tree\n\n");
 
     printf("\033[1;33mVARIABLES & CONSTANTS:\033[0m\n");
     printf("  x = 5                         -- assign variable (a-z)\n");
     printf("  Constants: pi, e, phi, tau, sqrt2, ln2, ln10, inf\n\n");
 
     printf("\033[1;33mFUNCTIONS:\033[0m\n");
     printf("  sin(x) cos(x) tan(x)         -- trigonometric\n");
     printf("  asin(x) acos(x) atan(x)      -- inverse trig\n");
     printf("  ln(x) exp(x) sqrt(x) abs(x)  -- log/exp/abs\n\n");
 
     printf("\033[1;33mCALCULUS:\033[0m\n");
     printf("  dy/dx(expr)                   -- symbolic derivative\n");
     printf("  derive EXPR / derivative EXPR -- same, REPL-style\n");
     printf("  int(expr, x) / integrate EXPR -- symbolic integral\n");
     printf("  def.int(expr, x, a, b)        -- numerical definite integral\n\n");
 
     printf("\033[1;33mEQUATIONS & POLYNOMIALS:\033[0m\n");
     printf("  solve EXPR = EXPR             -- numeric root-finding\n");
     printf("  factor a b c                  -- factor quadratic\n");
     printf("  roots a b c                   -- solve quadratic\n\n");
 
     printf("\033[1;33mDIFFERENTIAL EQUATIONS:\033[0m\n");
     printf("  ode y'=EXPR                   -- solve separable ODE\n\n");
 
     printf("\033[1;33mSTATISTICS:\033[0m\n");
     printf("  stats push VAL                -- add data point\n");
     printf("  stats mean / sum / min / max  -- compute statistic\n");
     printf("  stats median / stddev / var   -- compute statistic\n");
     printf("  stats list                    -- show all data points\n");
     printf("  stats clear                   -- clear data\n\n");
 
     printf("\033[1;33mMATRIX OPERATIONS:\033[0m\n");
     printf("  mat create r c v1 v2 ...      -- create r x c matrix\n");
     printf("  mat mul A B                   -- multiply matrices\n");
     printf("  mat add A B                   -- add matrices\n");
     printf("  mat det A                     -- determinant\n");
     printf("  mat transpose A               -- transpose\n");
     printf("  mat print A                   -- display matrix\n\n");
 
     printf("\033[1;33mPLOTTING:\033[0m\n");
     printf("  plot EXPR [xmin xmax]         -- ASCII function plot\n\n");
 
     printf("\033[1;33mGAME DEV & AI:\033[0m\n");
     printf("  game.new sprite NAME          -- create sprite\n");
     printf("  game.draw NAME                -- draw sprite\n");
     printf("  game.move NAME DX DY          -- move sprite\n");
     printf("  game.delete NAME              -- delete sprite\n");
     printf("  game.list / list              -- list sprites\n");
     printf("  remember KEY VALUE            -- store memory\n");
     printf("  recall KEY                    -- recall memory\n");
     printf("  ai status NAME                -- sprite status\n");
     printf("  ai patrol NAME                -- patrol move\n");
     printf("  ai goto NAME X Y              -- move toward target\n\n");
 
     printf("\033[1;33mMODES & OPTIONS:\033[0m\n");
     printf("  mode binary|ternary|decimal   -- number mode\n");
     printf("  steps on|off                  -- step-by-step solutions\n");
     printf("  history                       -- show command history\n");
     printf("  version                       -- show version\n\n");
 
     printf("\033[1;33mSYSTEM:\033[0m\n");
     printf("  help / ?                      -- this help\n");
     printf("  kill pew                      -- dramatic animated shutdown\n");
     printf("  quit / exit                   -- leave pew\n\n");
 }
 
 // ============================================================================
 // MATRIX STORAGE (simple global store)
 // ============================================================================
 
 #define MAT_STORE_MAX 16
 static Matrix mat_store[MAT_STORE_MAX];
 static char mat_names[MAT_STORE_MAX][16];
 static int mat_count = 0;
 
 int mat_find(const char *name){
     for(int i=0;i<mat_count;i++)
         if(strcmp(mat_names[i],name)==0) return i;
     return -1;
 }
 
 void mat_store_set(const char *name, Matrix m){
     int idx = mat_find(name);
     if(idx<0){
         if(mat_count>=MAT_STORE_MAX){ printf("[mat] matrix store full\n"); return; }
         idx = mat_count++;
         strncpy(mat_names[idx],name,15); mat_names[idx][15]='\0';
     }
     mat_store[idx] = m;
 }
 
 Matrix* mat_store_get(const char *name){
     int idx = mat_find(name);
     if(idx<0) return NULL;
     return &mat_store[idx];
 }
 
 // ============================================================================
 // MAIN COMMAND HANDLER
 // ============================================================================
 
 void handle_line(const char *line){
     if(!line || !*line) return;
 
     src = line;
     pos = 0;
     skipws();
 
     if(match("help") || match("?")){
         print_help();
         return;
     }
 
     if(match("kill pew")){
         kill_pew_animation();
         return;
     }
 
     if(match_token("version")){
         printf("PEW Advanced v1.0 -- Symbolic Calculator, REPL & Game Engine\n");
         printf("Compiled: %s %s\n", __DATE__, __TIME__);
         return;
     }
 
     if(match_token("steps")){
         skipws();
         if(match_token("on")){ verbose_steps=1; printf("Step-by-step solutions: ON\n"); return; }
         if(match_token("off")){ verbose_steps=0; printf("Step-by-step solutions: OFF\n"); return; }
         printf("Usage: steps on|off\n");
         return;
     }
 
     if(match_token("mode")){
         skipws();
         if(ci_strncasecmp(&src[pos],"binary",6)==0){ num_mode=2; printf("Mode: binary\n"); return; }
         if(ci_strncasecmp(&src[pos],"ternary",7)==0){ num_mode=3; printf("Mode: ternary\n"); return; }
         if(ci_strncasecmp(&src[pos],"decimal",7)==0){ num_mode=10; printf("Mode: decimal\n"); return; }
         printf("Usage: mode binary|ternary|decimal\n");
         return;
     }
 
     if(match_token("history")){
         hist_show();
         return;
     }
 
     // -----------------------------------------------------------------------
     // Variable assignment: single-letter a..z
     // -----------------------------------------------------------------------
     if(isalpha((unsigned char)src[pos]) && !isalpha((unsigned char)src[pos+1]) && src[pos+1]==' '){
         // peek: single letter followed by space or '='
     }
     {
         int savepos = pos;
         if(isalpha((unsigned char)src[pos])){
             char v = src[pos++];
             skipws();
             if(match("=")){
                 Node *e = parse_expr();
                 int vi = (v>='a' && v<='z') ? v-'a' : (v>='A' && v<='Z') ? v-'A' : -1;
                 if(vi>=0){
                     double old = gvars[vi];
                     gvars[vi] = eval(e, v, old);
                     printf("%c = %g\n", v, gvars[vi]);
                 }
                 free_node(e);
                 return;
             }
             pos = savepos;
         }
     }
 
     // -----------------------------------------------------------------------
     // REPL commands: show, derive, integrate, solve, expand
     // -----------------------------------------------------------------------
     if(match_token("show")){
         skipws();
         Node *e = parse_expr();
         printf("AST: "); print_node(e); printf("\n");
         double val = eval(e, 'x', gvars['x'-'a']);
         printf("=> %g\n", val);
         free_node(e);
         return;
     }
 
     if(match_token("derive") || match_token("derivative")){
         skipws();
         Node *e = parse_expr();
         Node *d = derivative(e, 'x');
         printf("d/dx("); print_node(e); printf(") = "); print_node(d); printf("\n");
         free_node(e); free_node(d);
         return;
     }
 
     if(match_token("integrate")){
         skipws();
         Node *e = parse_expr();
         Node *r = integrate_symbolic(e, 'x');
 C\n"); free_node(r); }
         else { printf("(symbolic integration not available)\n"); }
         free_node(e);
         return;
     }
 
     if(match_token("solve")){
         skipws();
         Node *left = parse_expr();
         skipws();
         if(match("=")){
             skipws();
             Node *right = parse_expr();
             Node *f = mknode(NODE_SUB, left, right);
             double a=-1000, b=1000;
             double fa=eval(f,'x',a), fb=eval(f,'x',b);
             for(int i=0;i<40 && !(fa==0||fb==0||fa*fb<0);i++){
                 double na=a+(b-a)*(double)i/40.0;
                 double nb=a+(b-a)*(double)(i+1)/40.0;
                 double fna=eval(f,'x',na), fnb=eval(f,'x',nb);
                 if(fna*fnb<=0){ a=na;b=nb;fa=fna;fb=fnb; break; }
             }
             if(fa==0){ printf("x = %g\n", a); free_node(f); return; }
             if(fb==0){ printf("x = %g\n", b); free_node(f); return; }
             if(fa*fb>0){ printf("No sign change found.\n"); free_node(f); return; }
             for(int it=0;it<80;it++){
                 double m=(a+b)/2.0, fm=eval(f,'x',m);
                 if(fm==0||(b-a)<1e-9){ printf("x = %g\n", m); free_node(f); return; }
                 if(fa*fm<0){ b=m;fb=fm; } else { a=m;fa=fm; }
             }
             printf("x ~ %g\n", (a+b)/2.0);
             free_node(f);
             return;
         }
         printf("Usage: solve LEFT_EXPR = RIGHT_EXPR\n");
         free_node(left);
         return;
     }
 
     if(match_token("expand")){
         skipws();
         Node *e = parse_expr();
         printf("expand: symbolic expansion not yet implemented for: ");
         print_node(e); printf("\n");
         free_node(e);
         return;
     }
 
     // -----------------------------------------------------------------------
     // Statistics
     // -----------------------------------------------------------------------
     if(match_token("stats")){
         skipws();
         if(match_token("push")){
             double v;
             if(parse_number_arg(&v)){
                 stats_push(v);
                 printf("stats: added %g (n=%d)\n", v, stats_count);
             } else { printf("Usage: stats push VALUE\n"); }
             return;
         }
         if(match_token("mean")){  printf("mean = %g\n", stats_mean()); return; }
         if(match_token("sum")){   printf("sum = %g\n", stats_sum()); return; }
         if(match_token("min")){   printf("min = %g\n", stats_min()); return; }
         if(match_token("max")){   printf("max = %g\n", stats_max()); return; }
         if(match_token("median")){ printf("median = %g\n", stats_median()); return; }
         if(match_token("stddev")){ printf("stddev = %g\n", stats_stddev()); return; }
         if(match_token("var")){   printf("variance = %g\n", stats_variance()); return; }
         if(match_token("list")){
             if(stats_count==0){ printf("stats: no data\n"); return; }
             printf("stats data (%d points): ", stats_count);
             for(int i=0;i<stats_count;i++) printf("%g ", stats_data[i]);
             printf("\n"); return;
         }
         if(match_token("clear")){ stats_clear(); printf("stats: cleared\n"); return; }
         printf("Usage: stats push|mean|sum|min|max|median|stddev|var|list|clear\n");
         return;
     }
 
     // -----------------------------------------------------------------------
     // Matrix operations
     // -----------------------------------------------------------------------
     if(match_token("mat")){
         skipws();
         if(match_token("create")){
             int r,c;
             if(!parse_int(&r)||!parse_int(&c)||r<1||r>MAT_MAX||c<1||c>MAT_MAX){
                 printf("Usage: mat create ROWS COLS v1 v2 ...\n"); return;
             }
             Matrix m = mat_zero(r,c);
             for(int i=0;i<r;i++) for(int j=0;j<c;j++){
                 double v=0; parse_number_arg(&v); m.m[i][j]=v;
             }
             char name[16]="_m";
             skipws();
             // optional name after the numbers
             int sav=pos;
             parse_token(name,sizeof(name));
             if(strlen(name)==0||isdigit((unsigned char)name[0])){ pos=sav; sprintf(name,"_m%d",mat_count); }
             mat_store_set(name,m);
             printf("[mat] created %dx%d matrix '%s'\n", r,c,name);
             return;
         }
         if(match_token("print")){
             skipws(); char name[16]; parse_token(name,sizeof(name));
             Matrix *m = mat_store_get(name);
             if(!m){ printf("[mat] '%s' not found\n", name); return; }
             mat_print(*m); return;
         }
         if(match_token("det")){
             skipws(); char name[16]; parse_token(name,sizeof(name));
             Matrix *m = mat_store_get(name);
             if(!m){ printf("[mat] '%s' not found\n", name); return; }
             printf("det(%s) = %g\n", name, mat_det(*m)); return;
         }
         if(match_token("transpose")){
             skipws(); char name[16]; parse_token(name,sizeof(name));
             Matrix *m = mat_store_get(name);
             if(!m){ printf("[mat] '%s' not found\n", name); return; }
             Matrix t = mat_transpose(*m);
             mat_print(t); return;
         }
         if(match_token("mul")){
             skipws(); char na[16]; parse_token(na,sizeof(na));
             skipws(); char nb[16]; parse_token(nb,sizeof(nb));
             Matrix *a=mat_store_get(na), *b=mat_store_get(nb);
             if(!a||!b){ printf("[mat] matrix not found\n"); return; }
             if(a->cols!=b->rows){ printf("[mat] dimension mismatch\n"); return; }
             Matrix r=mat_mul(*a,*b);
             char name[32]; sprintf(name,"_m%d",mat_count);
             mat_store_set(name,r);
             printf("[mat] result stored in '%s':\n",name);
             mat_print(r); return;
         }
         if(match_token("add")){
             skipws(); char na[16]; parse_token(na,sizeof(na));
             skipws(); char nb[16]; parse_token(nb,sizeof(nb));
             Matrix *a=mat_store_get(na), *b=mat_store_get(nb);
             if(!a||!b){ printf("[mat] matrix not found\n"); return; }
             if(a->rows!=b->rows||a->cols!=b->cols){ printf("[mat] dimension mismatch\n"); return; }
             Matrix r=mat_add(*a,*b);
             char name[32]; sprintf(name,"_m%d",mat_count);
             mat_store_set(name,r);
             printf("[mat] result stored in '%s':\n",name);
             mat_print(r); return;
         }
         printf("Usage: mat create|print|det|transpose|mul|add\n");
         return;
     }
 
     // -----------------------------------------------------------------------
     // Plot
     // -----------------------------------------------------------------------
     if(match_token("plot")){
         skipws();
         // save position to parse the expression string
         int expr_start = pos;
         // find end of expression (until newline or end of string)
         while(src[pos] && src[pos]!='\n') pos++;
         int expr_end = pos;
         // check for optional xmin xmax
         // For simplicity: use default -5 to 5
         double xmin=-5, xmax=5;
         // We need to re-parse: try to find two numbers at end
         // Actually let's just parse from expr_start
         char tmp[512]; int ti=0;
         for(int i=expr_start;i<expr_end && ti<510;i++) tmp[ti++]=src[i];
         tmp[ti]=0;
         // try to split: "EXPR" or "EXPR XMIN XMAX"
         // simple: scan backwards for two numbers
         ascii_plot(tmp, xmin, xmax);
         return;
     }
 
     // -----------------------------------------------------------------------
     // calc / evaluate / expr
     // -----------------------------------------------------------------------
     if(match_token("calc") || match_token("evaluate") || match_token("expr")){
         skipws();
         Node *e = parse_expr();
         printf("AST: "); print_node(e); printf("\n");
         double v = eval(e,'x',gvars['x'-'a']);
         printf("=> %g\n", v);
         free_node(e);
         return;
     }
 
     if(match_token("prnt") || match_token("print") || match_token("echo")){
         skipws();
         Node *e = parse_expr();
         print_node(e); printf("\n");
         free_node(e);
         return;
     }
 
     // -----------------------------------------------------------------------
     // ODE solver
     // -----------------------------------------------------------------------
     if(match_token("ode")){
         skipws();
         if(match("y'")){
             skipws();
             if(match("=")){
                 skipws();
                 Node *expr = parse_expr();
                 solve_separable_ode(expr, 'x');
                 free_node(expr);
                 return;
             }
         }
         printf("Usage: ode y'=EXPRESSION\n");
         return;
     }
 
     // -----------------------------------------------------------------------
     // Game commands
     // -----------------------------------------------------------------------
     if(match("game.new") || match("game new") || match("spawn") || match("create")){
         skipws();
         if(match("sprite")){
             char name[32]; parse_token(name,sizeof(name));
             create_sprite(name); return;
         }
         printf("Usage: game.new sprite NAME\n");
         return;
     }
 
     if(match("game.draw") || match("game draw") || match("draw")){
         char name[32]; parse_token(name,sizeof(name));
         draw_sprite(name); return;
     }
 
     if(match("game.move") || match("game move") || match("move")){
         char name[32]; int dx,dy;
         parse_token(name,sizeof(name));
         if(!parse_int(&dx)||!parse_int(&dy)){ printf("Usage: game.move NAME DX DY\n"); return; }
         move_sprite(name,dx,dy); return;
     }
 
     if(match("game.delete") || match("game delete") || match("delete")){
         char name[32]; parse_token(name,sizeof(name));
         delete_sprite(name); return;
     }
 
     if(match("game.list") || match("list")){
         list_sprites(); return;
     }
 
     if(match_token("remember")){
         char key[32]; parse_token(key,sizeof(key));
         skipws();
         char value[128]; int i=0;
         while(src[pos] && i<(int)sizeof(value)-1) value[i++]=src[pos++];
         value[i]=0;
         store_memory(key,value);
         return;
     }
 
     if(match_token("recall")){
         char key[32]; parse_token(key,sizeof(key));
         recall_memory(key); return;
     }
 
     if(match_token("ai")){
         skipws();
         if(match_token("status")){
             char name[32]; parse_token(name,sizeof(name));
             ai_status(name); return;
         }
         if(match_token("patrol")){
             char name[32]; parse_token(name,sizeof(name));
             ai_think(name); return;
         }
         if(match_token("goto")||match_token("move")){
             char name[32]; int tx,ty;
             parse_token(name,sizeof(name));
             if(!parse_int(&tx)||!parse_int(&ty)){ printf("Usage: ai goto NAME X Y\n"); return; }
             ai_move_to(name,tx,ty); return;
         }
         printf("AI: ai status|patrol|goto NAME [X Y]\n");
         return;
     }
 
     if(match_token("factor")){
         double a,b,c;
         if(!parse_number_arg(&a)||!parse_number_arg(&b)||!parse_number_arg(&c)){
             printf("Usage: factor A B C\n"); return;
         }
         factor_quadratic(a,b,c); return;
     }
 
     if(match_token("roots")){
         double a,b,c;
         if(!parse_number_arg(&a)||!parse_number_arg(&b)||!parse_number_arg(&c)){
             printf("Usage: roots A B C\n"); return;
         }

         factor_quadratic(a,b,c); return;
     }
 
     // -----------------------------------------------------------------------
     // Default: evaluate as expression
     // -----------------------------------------------------------------------
     Node *expr = parse_expr();
     printf("Expression: "); print_node(expr); printf("\n");
 
     if(verbose_steps){
         printf("d/dx: ");
         Node *d = derivative(expr, 'x');
         print_node(d); printf("\n");
         free_node(d);
 
         printf("Integral: ");
         Node *i = integrate_symbolic(expr, 'x');
         if(i) print_node(i); else printf("(not integrable symbolically)");
         printf("\n");
         free_node(i);
     }
 
     double val = eval(expr, 'x', 0.0);
     printf("=> %g\n", val);
     free_node(expr);
 }
 
 // ============================================================================
 // MAIN
 // ============================================================================
 
 int main(int argc, char **argv){
     (void)argc; (void)argv;
 
     print_retro_graphics();
     print_banner();
 
 #ifdef USE_READLINE
     using_history();
     char *line = NULL;
     while(1){
         line = readline("pew> ");
         if(!line) break;
         if(*line){
 #endif
 
     // cleanup
     for(int i=0;i<hist_count;i++) free(hist_buf[i]);
     printf("\nGoodbye!\n");
     return 0;
 }

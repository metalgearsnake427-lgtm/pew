 typedef struct { double re, im; } Complex;
+ 
+ Complex cx_make(double re, double im){ Complex c; c.re=re; c.im=im; return c; }
+ Complex cx_add(Complex a, Complex b){ return cx_make(a.re+b.re, a.im+b.im); }
+ Complex cx_sub(Complex a, Complex b){ return cx_make(a.re-b.re, a.im-b.im); }
+ Complex cx_mul(Complex a, Complex b){ return cx_make(a.re*b.re - a.im*b.im, a.re*b.im + a.im*b.re); }
+ Complex cx_div(Complex a, Complex b){
+     double d = b.re*b.re + b.im*b.im;
+     if(fabs(d)<1e-30) return cx_make(1e30, 0);
+     return cx_make((a.re*b.re+a.im*b.im)/d, (a.im*b.re-a.re*b.im)/d);
+ }
+ Complex cx_conj(Complex a){ return cx_make(a.re, -a.im); }
+ Complex cx_neg(Complex a){ return cx_make(-a.re, -a.im); }
+ Complex cx_scale(Complex a, double s){ return cx_make(a.re*s, a.im*s); }
+ 
+ Complex cx_sqrt(Complex a){
+     double r = sqrt(a.re*a.re + a.im*a.im);
+     double theta = atan2(a.im, a.re);
+     double sr = sqrt(r);
+     return cx_make(sr*cos(theta/2.0), sr*sin(theta/2.0));
+ }
+ 
+ Complex cx_pow(Complex base, Complex exp){
+     // b^e = exp(e * ln(b))
+     double r = sqrt(base.re*base.re + base.im*base.im);
+     if(r < 1e-30) return cx_make(0,0);
+     double theta = atan2(base.im, base.re);
+     double lnr = log(r);
+     // ln(b) = lnr + i*theta
+     // e * ln(b) = (exp.re + i*exp.im) * (lnr + i*theta)
+     double p_re = exp.re*lnr - exp.im*theta;
+     double p_im = exp.re*theta + exp.im*lnr;
+     return cx_make(exp(p_re)*cos(p_im), exp(p_re)*sin(p_im));
+ }
+ 
+ Complex cx_exp(Complex a){ return cx_make(exp(a.re)*cos(a.im), exp(a.re)*sin(a.im)); }
+ Complex cx_ln(Complex a){
+     double r = sqrt(a.re*a.re + a.im*a.im);
+     if(r < 1e-30) return cx_make(-1e30, 0);
+     return cx_make(log(r), atan2(a.im, a.re));
+ }
+ Complex cx_sin(Complex a){
+     // sin(a) = (e^(ia) - e^(-ia)) / (2i)
+     Complex ei = cx_exp(cx_make(-a.im, a.re));
+     Complex emi = cx_exp(cx_make(a.im, -a.re));
+     return cx_div(cx_sub(ei, emi), cx_make(0, 2));
+ }
+ Complex cx_cos(Complex a){
+     Complex ei = cx_exp(cx_make(-a.im, a.re));
+     Complex emi = cx_exp(cx_make(a.im, -a.re));
+     return cx_scale(cx_add(ei, emi), 0.5);
+ }
+ Complex cx_abs(Complex a){ return cx_make(sqrt(a.re*a.re + a.im*a.im), 0); }
+ Complex cx_polar(double r, double theta){ return cx_make(r*cos(theta), r*sin(theta)); }
+ 
+ void cx_print(Complex a){
+     if(fabs(a.im) < 1e-15) printf("%g", a.re);
+     else if(fabs(a.re) < 1e-15) printf("%gi", a.im);
+     else if(a.im > 0) printf("%g+%gi", a.re, a.im);
+     else printf("%g%gi", a.re, a.im);
+ }
+ 
+ // Global complex variable store (shares names with real vars)
+ static Complex cx_vars[26];
+ static int use_complex = 0;  // toggle complex mode
+ 
+ // Evaluate expression as complex number
+ Complex cx_eval(Node *n, char var, Complex varval){
+     if(!n) return cx_make(0,0);
+     switch(n->type){
+         case NODE_NUM: return cx_make(n->value, 0);
+         case NODE_VAR: {
+             if(n->var==var) return varval;
+             int vi = (n->var>='a' && n->var<='z') ? n->var-'a' : (n->var>='A' && n->var<='Z') ? n->var-'A' : -1;
+             if(vi>=0) return cx_vars[vi];
+             return cx_make(0,0);
+         }
+         case NODE_NEG: return cx_neg(cx_eval(n->l, var, varval));
+         case NODE_ADD: return cx_add(cx_eval(n->l,var,varval), cx_eval(n->r,var,varval));
+         case NODE_SUB: return cx_sub(cx_eval(n->l,var,varval), cx_eval(n->r,var,varval));
+         case NODE_MUL: return cx_mul(cx_eval(n->l,var,varval), cx_eval(n->r,var,varval));
+         case NODE_DIV: return cx_div(cx_eval(n->l,var,varval), cx_eval(n->r,var,varval));
+         case NODE_POW: return cx_pow(cx_eval(n->l,var,varval), cx_eval(n->r,var,varval));
+         case NODE_SIN: return cx_sin(cx_eval(n->l,var,varval));
+         case NODE_COS: return cx_cos(cx_eval(n->l,var,varval));
+         case NODE_TAN: return cx_div(cx_sin(cx_eval(n->l,var,varval)), cx_cos(cx_eval(n->l,var,varval)));
+         case NODE_LN: return cx_ln(cx_eval(n->l,var,varval));
+         case NODE_EXP: return cx_exp(cx_eval(n->l,var,varval));
+         case NODE_SQRT: return cx_sqrt(cx_eval(n->l,var,varval));
+         case NODE_ABS: return cx_abs(cx_eval(n->l,var,varval));
+         default: return cx_make(0,0);
+     }
+ }
+ 
+ // Mathematical constants
+ static double math_const(const char *name){
 
- void free_node(Node *n){
-     if(!n) return;
-     free_node(n->l);
-     free_node(n->r);
-     free(n);
- }
+ void free_node(Node *n){
+     if(!n) return;
+     free_node(n->l);
+     free_node(n->r);
+     free(n);
+ }
+ 
+ // Deep copy an AST node
+ Node* copy_node(Node *n){
+     if(!n) return NULL;
+     Node *c = (Node*)malloc(sizeof(Node));
+     if(!c) return NULL;
+     memcpy(c, n, sizeof(Node));
+     c->l = copy_node(n->l);
+     c->r = copy_node(n->r);
+     return c;
+ }
+ 
+ // ============================================================================
+ // SYMBOLIC SIMPLIFICATION
+ // ============================================================================
+ 
+ Node* simplify(Node *n){
+     if(!n) return NULL;
+ 
+     // Recursively simplify children first
+     Node *l = simplify(n->l);
+     Node *r = simplify(n->r);
+     Node *result = NULL;
+ 
+     switch(n->type){
+         case NODE_NUM:
+         case NODE_VAR:
+             result = mknode(n->type, NULL, NULL);
+             if(result){ result->value = n->value; result->var = n->var; }
+             free_node(l); free_node(r);
+             return result;
+ 
+         case NODE_NEG: {
+             if(!l){ free_node(r); return NULL; }
+             // -(-x) = x
+             if(l->type==NODE_NEG){
+                 Node *inner = l->l; l->l = NULL; free_node(l); free_node(r);
+                 return inner;
+             }
+             // -(num) = -num
+             if(l->type==NODE_NUM){
+                 double v = -l->value; free_node(l); free_node(r);
+                 return mknum(v);
+             }
+             free_node(r);
+             return mknode(NODE_NEG, l, NULL);
+         }
+ 
+         case NODE_ADD: {
+             if(!l || !r){ free_node(l); free_node(r); return l ? l : r; }
+             // x + 0 = x
+             if(r->type==NODE_NUM && fabs(r->value) < 1e-15){ free_node(r); return l; }
+             // 0 + x = x
+             if(l->type==NODE_NUM && fabs(l->value) < 1e-15){ free_node(l); return r; }
+             // x + x = 2*x
+             if(l->type==NODE_VAR && r->type==NODE_VAR && l->var==r->var){
+                 free_node(l); free_node(r);
+                 return mknode(NODE_MUL, mknum(2), mkvar(n->l->var));
+             }
+             // num + num = fold
+             if(l->type==NODE_NUM && r->type==NODE_NUM){
+                 double v = l->value + r->value;
+                 free_node(l); free_node(r);
+                 return mknum(v);
+             }
+             result = mknode(NODE_ADD, l, r);
+             return result;
+         }
+ 
+         case NODE_SUB: {
+             if(!l || !r){ free_node(l); free_node(r); return l ? l : r; }
+             // x - 0 = x
+             if(r->type==NODE_NUM && fabs(r->value) < 1e-15){ free_node(r); return l; }
+             // 0 - x = -x
+             if(l->type==NODE_NUM && fabs(l->value) < 1e-15){ free_node(l); return mknode(NODE_NEG, r, NULL); }
+             // x - x = 0
+             if(l->type==NODE_VAR && r->type==NODE_VAR && l->var==r->var){
+                 free_node(l); free_node(r); return mknum(0);
+             }
+             // num - num = fold
+             if(l->type==NODE_NUM && r->type==NODE_NUM){
+                 double v = l->value - r->value;
+                 free_node(l); free_node(r);
+                 return mknum(v);
+             }
+             result = mknode(NODE_SUB, l, r);
+             return result;
+         }
+ 
+         case NODE_MUL: {
+             if(!l || !r){ free_node(l); free_node(r); return mknum(0); }
+             // x * 0 = 0
+             if((l->type==NODE_NUM && fabs(l->value)<1e-15) ||
+                (r->type==NODE_NUM && fabs(r->value)<1e-15)){
+                 free_node(l); free_node(r); return mknum(0);
+             }
+             // x * 1 = x
+             if(r->type==NODE_NUM && fabs(r->value-1.0)<1e-15){ free_node(r); return l; }
+             // 1 * x = x
+             if(l->type==NODE_NUM && fabs(l->value-1.0)<1e-15){ free_node(l); return r; }
+             // x * (-1) = -x
+             if(r->type==NODE_NUM && fabs(r->value+1.0)<1e-15){ free_node(r); return mknode(NODE_NEG, l, NULL); }
+             // (-1) * x = -x
+             if(l->type==NODE_NUM && fabs(l->value+1.0)<1e-15){ free_node(l); return mknode(NODE_NEG, r, NULL); }
+             // num * num = fold
+             if(l->type==NODE_NUM && r->type==NODE_NUM){
+                 double v = l->value * r->value;
+                 free_node(l); free_node(r);
+                 return mknum(v);
+             }
+             result = mknode(NODE_MUL, l, r);
+             return result;
+         }
+ 
+         case NODE_DIV: {
+             if(!l || !r){ free_node(l); free_node(r); return mknum(0); }
+             // 0 / x = 0
+             if(l->type==NODE_NUM && fabs(l->value)<1e-15){ free_node(l); free_node(r); return mknum(0); }
+             // x / 1 = x
+             if(r->type==NODE_NUM && fabs(r->value-1.0)<1e-15){ free_node(r); return l; }
+             // x / x = 1 (same variable)
+             if(l->type==NODE_VAR && r->type==NODE_VAR && l->var==r->var){
+                 free_node(l); free_node(r); return mknum(1);
+             }
+             // num / num = fold
+             if(l->type==NODE_NUM && r->type==NODE_NUM && fabs(r->value)>1e-15){
+                 double v = l->value / r->value;
+                 free_node(l); free_node(r);
+                 return mknum(v);
+             }
+             result = mknode(NODE_DIV, l, r);
+             return result;
+         }
+ 
+         case NODE_POW: {
+             if(!l || !r){ free_node(l); free_node(r); return mknum(1); }
+             // x^0 = 1
+             if(r->type==NODE_NUM && fabs(r->value)<1e-15){
+                 free_node(l); free_node(r); return mknum(1);
+             }
+             // x^1 = x
+             if(r->type==NODE_NUM && fabs(r->value-1.0)<1e-15){
+                 free_node(r); return l;
+             }
+             // 0^x = 0 (x>0)
+             if(l->type==NODE_NUM && fabs(l->value)<1e-15){
+                 free_node(l); free_node(r); return mknum(0);
+             }
+             // 1^x = 1
+             if(l->type==NODE_NUM && fabs(l->value-1.0)<1e-15){
+                 free_node(l); free_node(r); return mknum(1);
+             }
+             // num^num = fold
+             if(l->type==NODE_NUM && r->type==NODE_NUM){
+                 double v = pow(l->value, r->value);
+                 free_node(l); free_node(r);
+                 return mknum(v);
+             }
+             result = mknode(NODE_POW, l, r);
+             return result;
+         }
+ 
+         // Unary functions with constant args -> fold
+         case NODE_SIN: case NODE_COS: case NODE_TAN:
+         case NODE_LN:  case NODE_EXP: case NODE_SQRT:
+         case NODE_ABS: case NODE_ASIN: case NODE_ACOS: case NODE_ATAN: {
+             if(!l){ free_node(r); return NULL; }
+             if(l->type==NODE_NUM){
+                 double v;
+                 switch(n->type){
+                     case NODE_SIN:  v = sin(l->value); break;
+                     case NODE_COS:  v = cos(l->value); break;
+                     case NODE_TAN:  v = tan(l->value); break;
+                     case NODE_LN:   v = (l->value>0)? log(l->value) : 0; break;
+                     case NODE_EXP:  v = exp(l->value); break;
+                     case NODE_SQRT: v = (l->value>=0)? sqrt(l->value) : 0; break;
+                     case NODE_ABS:  v = fabs(l->value); break;
+                     case NODE_ASIN: v = asin(l->value); break;
+                     case NODE_ACOS: v = acos(l->value); break;
+                     case NODE_ATAN: v = atan(l->value); break;
+                     default: v = 0; break;
+                 }
+                 free_node(l); free_node(r);
+                 return mknum(v);
+             }
+             free_node(r);
+             return mknode(n->type, l, NULL);
+         }
+ 
+         default:
+             free_node(l); free_node(r);
+             return copy_node(n);
+     }
+ }
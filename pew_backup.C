#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#ifdef USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif
#include <time.h>
#include <sys/time.h>
#include <sys/select.h>

// forward declarations for portable helpers
int ci_strcasecmp(const char *a, const char *b);
int ci_strncasecmp(const char *a, const char *b, size_t n);
static void sleep_ms(long ms);


// Minimal interpreter for the `pew` language (basic prototype)
// Features:
// - Modes: binary / ternary (affect integer literal parsing)
// - Simple expressions: + - * / ^ and parentheses
// - `prnt` statement: prints expression value or symbolic form
// - Yhubls: `dy/dx(expr)` for symbolic derivative w.r.t x
// - `int(expr, x)` for symbolic indefinite integration (power rule)
// - `def.int(expr, x, a, b)` numerical definite integral (trapezoid)
// - Tolerant parsing: `prnt expr` or `prnt(expr)` both work

typedef enum {NODE_NUM, NODE_VAR, NODE_ADD, NODE_SUB, NODE_MUL, NODE_DIV, NODE_POW} NodeType;

typedef struct Node {
	NodeType type;
	double value; // for numbers
	char var;     // for variable nodes (single-letter)
	struct Node *l, *r;
} Node;

// Global mode: 2 or 3
static int num_mode = 10; // default decimal; will set to 2 or 3 on mode commands

// --- Dictionary and translation helpers
typedef struct { const char *word; const char *en; const char *es; const char *fr; const char *de; } DictEntry;
static DictEntry dictionary[] = {
    {"hello","hello","hola","bonjour","hallo"},
    {"world","world","mundo","monde","welt"},
    {"game","game","juego","jeu","spiel"},
    {"start","start","comenzar","commencer","starten"},
    {"player","player","jugador","joueur","spieler"},
    {"score","score","puntuación","score","punktestand"},
    {"move","move","mover","déplacer","bewegen"},
    {"jump","jump","saltar","sauter","springen"},
    {"run","run","correr","courir","laufen"},
    {"level","level","nivel","niveau","niveau"},
    {"enemy","enemy","enemigo","ennemi","feind"},
    {"health","health","salud","santé","gesundheit"},
    {"coin","coin","moneda","pièce","münze"},
    {"speed","speed","velocidad","vitesse","geschwindigkeit"},
    {"jump","jump","saltar","sauter","springen"},
    {"attack","attack","ataque","attaque","angriff"},
    {"shield","shield","escudo","bouclier","schild"},
    {"level","level","nivel","niveau","niveau"},
    {"boss","boss","jefe","boss","boss"},
    {"score","score","puntuación","score","punktestand"},
    {"power","power","poder","puissance","kraft"},
    {"win","win","ganar","gagner","gewinnen"},
    {"lose","lose","perder","perdre","verlieren"},
    {"hero","hero","héroe","héros","held"},
    {"world","world","mundo","monde","welt"},
    {"level","level","nivel","niveau","niveau"},
    {"sound","sound","sonido","son","klang"},
    {"music","music","música","musique","musik"}
};
static int dictionary_size = sizeof(dictionary)/sizeof(dictionary[0]);
static const char *language_names[] = {"en", "es", "fr", "de"};
typedef struct { const char *term; const char *explanation; } GlossaryEntry;
static const char *pew_keywords[] = {
    "help", "h", "quit", "exit", "bye", "kill pew", "prnt", "print", "echo", "translate",
    "explain", "define", "study", "mode binary", "mode ternary", "mode decimal", "set mode binary",
    "set mode ternary", "clear", "calc", "evaluate", "expr", "game.new window", "game.new sprite",
    "create sprite", "spawn sprite", "draw", "move", "list", "delete sprite", "game.list",
    "dy/dx", "int", "def.int"
};
static GlossaryEntry glossary[] = {
    {"pew", "A compact shell language mixing simple math, text play, and tiny game objects."},
    {"binary", "Read numbers in base 2: only 0 and 1 digits are valid unless prefixes change the mode."},
    {"ternary", "Read numbers in base 3: digits are 0, 1, and 2 for a more exotic numeric mode."},
    {"decimal", "The normal base 10 number mode used for everyday arithmetic."},
    {"dy/dx", "Compute a symbolic derivative with respect to x for simple expressions."},
    {"int", "Compute a limited symbolic integral using power rules for x^n and constants."},
    {"def.int", "Compute a numeric definite integral from a to b using a trapezoid approximation."},
    {"game.new window", "Create a simple text-based game window placeholder."},
    {"game.new sprite", "Create a named sprite object that can move and draw in the shell."},
    {"sprite", "A named object with x,y position used in game-like commands."},
    {"kill pew", "A dramatic shell exit that plays a colorful bird death animation."},
    {"anthony burgess", "An invitation to define a term in a vivid literary-technical voice, without quoting his work."},
    {"explain", "Ask pew to give a compact meaning for a term or keyword in this language."},
    {"translate", "Look up a built-in word and show it in another language."},
    {"clear", "Clear the terminal screen and reset the workspace display."},
    {"mode", "Switch how numeric literals are interpreted: binary, ternary, or decimal."},
    {"run", "A synonym for express, evaluate or execute a calculation in the shell."}
};
static const char *pew_version = "0.3";
static const char *build_stamp = __DATE__ " " __TIME__;
static char program_path[256] = "";

void print_banner(){
    printf("\033[1;34mpew %s\033[0m — small expressive shell language\n", pew_version);
    printf("built %s\n", build_stamp);
    if(program_path[0]) printf("binary: %s\n", program_path);
    printf("Type 'help' for commands and 'version' to verify the build.\n\n");
}

void print_version(){
    printf("pew version %s\n", pew_version);
    printf("build %s\n", build_stamp);
    if(program_path[0]) printf("path %s\n", program_path);
}

// --- Game development helpers
typedef struct { char name[32]; int x,y; } Sprite;
static Sprite sprites[16];
static int sprite_count = 0;

// --- Memory helpers
Node *mknum(double v){ Node *n = malloc(sizeof(Node)); memset(n,0,sizeof(Node)); n->type=NODE_NUM; n->value=v; return n; }
Node *mkvar(char c){ Node *n = malloc(sizeof(Node)); memset(n,0,sizeof(Node)); n->type=NODE_VAR; n->var=c; return n; }
Node *mknode(NodeType t, Node*l, Node*r){ Node *n = malloc(sizeof(Node)); memset(n,0,sizeof(Node)); n->type=t; n->l=l; n->r=r; return n; }

// --- Printing AST
void print_node(Node *n){
	if(!n) return;
	switch(n->type){
		case NODE_NUM: printf("%g", n->value); break;
		case NODE_VAR: printf("%c", n->var); break;
		case NODE_ADD: printf("("); print_node(n->l); printf(" + "); print_node(n->r); printf(")"); break;
		case NODE_SUB: printf("("); print_node(n->l); printf(" - "); print_node(n->r); printf(")"); break;
		case NODE_MUL: printf("("); print_node(n->l); printf(" * "); print_node(n->r); printf(")"); break;
		case NODE_DIV: printf("("); print_node(n->l); printf(" / "); print_node(n->r); printf(")"); break;
		case NODE_POW: printf("("); print_node(n->l); printf(" ^ "); print_node(n->r); printf(")"); break;
	}
}

// --- Evaluate AST numerically, with variable binding (single var)
double eval(Node *n, char var, double varval){
	if(!n) return 0.0;
	switch(n->type){
		case NODE_NUM: return n->value;
		case NODE_VAR: return (n->var==var)?varval:0.0;
		case NODE_ADD: return eval(n->l,var,varval)+eval(n->r,var,varval);
		case NODE_SUB: return eval(n->l,var,varval)-eval(n->r,var,varval);
		case NODE_MUL: return eval(n->l,var,varval)*eval(n->r,var,varval);
		case NODE_DIV: return eval(n->l,var,varval)/eval(n->r,var,varval);
		case NODE_POW: return pow(eval(n->l,var,varval), eval(n->r,var,varval));
	}
	return 0.0;
}

// --- Symbolic derivative (basic rules + product/quotient/power with constant exponent)
Node* derivative(Node *n, char var){
	if(!n) return NULL;
	switch(n->type){
		case NODE_NUM: return mknum(0);
		case NODE_VAR: return mknum((n->var==var)?1:0);
		case NODE_ADD: return mknode(NODE_ADD, derivative(n->l,var), derivative(n->r,var));
		case NODE_SUB: return mknode(NODE_SUB, derivative(n->l,var), derivative(n->r,var));
		case NODE_MUL: {
			// (u*v)' = u'*v + u*v'
			Node *u = n->l; Node *v = n->r;
			Node *t1 = mknode(NODE_MUL, derivative(u,var), v); // u'*v
			Node *t2 = mknode(NODE_MUL, u, derivative(v,var)); // u*v'
			return mknode(NODE_ADD, t1, t2);
		}
		case NODE_DIV: {
			// (u/v)' = (u'*v - u*v')/v^2
			Node *u=n->l; Node *v=n->r;
			Node *num = mknode(NODE_SUB, mknode(NODE_MUL, derivative(u,var), v), mknode(NODE_MUL, u, derivative(v,var)));
			Node *den = mknode(NODE_POW, v, mknum(2));
			return mknode(NODE_DIV, num, den);
		}
		case NODE_POW: {
			// If exponent is a number constant: (f^c)' = c * f^(c-1) * f'
			if(n->r->type==NODE_NUM){
				double c = n->r->value;
				Node *c_n = mknum(c);
				Node *powpart = mknode(NODE_POW, n->l, mknum(c-1));
				return mknode(NODE_MUL, mknode(NODE_MUL, c_n, powpart), derivative(n->l,var));
			}
			// Otherwise fallback to numeric derivative represented as 0 (placeholder)
			return mknum(0);
		}
	}
	return mknum(0);
}

// --- Symbolic indefinite integral (very limited: power rule for variable)
Node* integrate_symbolic(Node *n, char var){
	if(!n) return NULL;
	if(n->type==NODE_VAR && n->var==var){
		return mknode(NODE_POW, mkvar(var), mknum(2)); // x^2 -> will divide by 2 below; we'll return x^2/2 by wrapping in div
	}
	if(n->type==NODE_POW && n->l->type==NODE_VAR && n->l->var==var && n->r->type==NODE_NUM){
		double exp = n->r->value;
		if(fabs(exp+1.0) < 1e-9){
			return NULL; // integral x^-1 -> ln(x) not implemented
		}
		// x^p -> x^(p+1)/(p+1)
		Node *num = mknode(NODE_POW, mkvar(var), mknum(exp+1));
		Node *den = mknum(exp+1);
		return mknode(NODE_DIV, num, den);
	}
	// constant
	if(n->type==NODE_NUM) return mknode(NODE_MUL, n, mknum(1)); // c -> c*x (caller may wrap)
	return NULL; // many cases not supported
}

// --- Numerical definite integral via simple trapezoid rule
double definite_integral(Node *n, char var, double a, double b){
	int N = 2000;
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

// --- Parser (very small recursive descent)
const char *src;
int pos;

void skipws(){ while(src[pos] && isspace((unsigned char)src[pos])) pos++; }

int match(const char *s){ skipws(); int i=0; while(s[i] && src[pos+i] && tolower(src[pos+i])==tolower(s[i])) i++; if(s[i]==0){ pos+=i; return 1;} return 0; }

// parse number respecting num_mode (2/3/10)
Node* parse_number(){ skipws(); int start=pos; int base = num_mode==2?2:(num_mode==3?3:10);
	double val=0; int any=0;
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

Node* parse_pow(){ Node *left = parse_primary(); skipws(); if(match("^")){ Node *right = parse_pow(); return mknode(NODE_POW,left,right);} return left; }

Node* parse_term(){ Node *n = parse_pow(); skipws(); while(1){ if(match("*")){ Node *r = parse_pow(); n = mknode(NODE_MUL,n,r);} else if(match("/")){ Node *r = parse_pow(); n = mknode(NODE_DIV,n,r);} else break; } return n; }

Node* parse_expr(){ Node *n = parse_term(); skipws(); while(1){ if(match("+")){ Node *r = parse_term(); n = mknode(NODE_ADD,n,r);} else if(match("-")){ Node *r = parse_term(); n = mknode(NODE_SUB,n,r);} else break; } return n; }

void parse_token(char *buf, int max){ skipws(); int i=0; while(src[pos] && !isspace((unsigned char)src[pos]) && src[pos] != ')' && src[pos] != ',' && i < max-1){ buf[i++] = src[pos++]; } buf[i]=0; }
int parse_int(){ skipws(); int sign = 1; if(src[pos] == '-') { sign = -1; pos++; }
    int v = 0; while(isdigit((unsigned char)src[pos])){ v = v*10 + (src[pos]-'0'); pos++; }
    return v * sign;
}
int is_single_word(const char *text){ int len = 0; while(text[len] && !isspace((unsigned char)text[len])){ if(!isalpha((unsigned char)text[len])) return 0; len++; } return len > 0 && text[len] == 0; }

const char *translate_word(const char *word, const char *lang){ for(int i=0;i<dictionary_size;i++){
        if(ci_strcasecmp(dictionary[i].word, word)==0){
            if(strcmp(lang,"es")==0) return dictionary[i].es;
            if(strcmp(lang,"fr")==0) return dictionary[i].fr;
            if(strcmp(lang,"de")==0) return dictionary[i].de;
            return dictionary[i].en;
        }
    }
    return NULL;
}

// Case-insensitive compare helpers (portable)
int ci_strcasecmp(const char *a, const char *b){
    while(*a && *b){ int ca = tolower((unsigned char)*a); int cb = tolower((unsigned char)*b); if(ca!=cb) return ca-cb; a++; b++; }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}
int ci_strncasecmp(const char *a, const char *b, size_t n){
    size_t i=0; for(; i<n && a[i] && b[i]; i++){ int ca = tolower((unsigned char)a[i]); int cb = tolower((unsigned char)b[i]); if(ca!=cb) return ca-cb; }
    if(i==n) return 0; return tolower((unsigned char)a[i]) - tolower((unsigned char)b[i]);
}

static void sleep_ms(long ms){
    // Use select-based sleep for portability without feature-test macros
    struct timeval tv; tv.tv_sec = ms/1000; tv.tv_usec = (ms%1000)*1000; select(0, NULL, NULL, NULL, &tv);
}

void prompt_translate(const char *word){ char lang[16];
    printf("Translate '%s' into which language? (en/es/fr/de): ", word);
    if(!fgets(lang,sizeof(lang),stdin)) return;
    for(int i=0; lang[i]; i++) if(lang[i]=='\n') lang[i]=0;
    if(!lang[0]) return;
    const char *trans = translate_word(word, lang);
    if(trans){ printf("%s -> %s\n", word, trans); }
    else { printf("No translation found for '%s'\n", word); }
}

void translate_word_command(){ char word[128]; char lang[16];
    parse_token(word,sizeof(word)); if(!word[0]){ printf("translate <word> [en/es/fr/de]\n"); return; }
    parse_token(lang,sizeof(lang));
    if(!lang[0]){
        if(isatty(0)){ prompt_translate(word); return; }
        printf("translate %s <en|es|fr|de>\n", word); return;
    }
    const char *trans = translate_word(word, lang);
    if(trans){ printf("%s -> %s\n", word, trans); }
    else { printf("No translation found for '%s'\n", word); }
}

int is_blank(const char *s){ while(*s){ if(!isspace((unsigned char)*s)) return 0; s++; } return 1; }

void print_keyword_overview(){
    printf("keywords and meanings:\n");
    printf("  help/h           - show this command list\n");
    printf("  quit/exit/bye    - leave pew\n");
    printf("  kill pew         - shoot a bird-style shell exit animation\n");
    printf("  prnt/expr/calc   - evaluate a math expression\n");
    printf("  print/echo       - print text or expression output\n");
    printf("  translate        - translate a built-in word to another language\n");
    printf("  explain/define   - describe a keyword in a vivid custom glossary\n");
    printf("  mode binary      - switch to binary numeric mode\n");
    printf("  mode ternary     - switch to ternary numeric mode\n");
    printf("  mode decimal     - switch back to normal decimals\n");
    printf("  game.new window  - declare a simple game window placeholder\n");
    printf("  game.new sprite  - create a named sprite object\n");
    printf("  draw/move/list   - draw and move sprites or list them\n");
    printf("  delete sprite    - remove a named sprite from the shell world\n");
    printf("  dy/dx(expr)      - symbolic derivative in x\n");
    printf("  int(expr, x)     - symbolic integration (limited)\n");
    printf("  def.int(expr,x,a,b) - numeric definite integral approximation\n\n");
}

const char *lookup_glossary(const char *term){
    for(int i=0;i<sizeof(glossary)/sizeof(glossary[0]);i++){
        if(ci_strcasecmp(glossary[i].term, term)==0) return glossary[i].explanation;
    }
    return NULL;
}

void explain_command(){ char topic[128]; parse_token(topic,sizeof(topic));
    if(!topic[0]){
        printf("explain <term> - available: pew, binary, ternary, decimal, dy/dx, int, def.int, game.new window, game.new sprite, kill pew, anthony burgess\n");
        return;
    }
    if(strcmp(topic,"anthony")==0){ if(match("burgess")){ printf("anthony burgess is used here as inspiration for vivid definition without borrowing his text.\n"); return; }}
    const char *meaning = lookup_glossary(topic);
    if(meaning){ printf("%s: %s\n", topic, meaning); return; }
    printf("No poetic definition found for '%s'. Try another keyword.\n", topic);
}

void welcome_animation(){
    if(!isatty(STDOUT_FILENO)) return;
    const char *frames[] = {
        "\033[1;36m   .--.   \n  / _.-'  \n  \\  '-._ \n   '--'   \033[0m\n",
        "\033[1;35m   .--.   \n  /    '-.\n  \\  .--' \n   '--'   \033[0m\n",
        "\033[1;33m   .--.   \n  / .--.  \n  \\ '--'  \n   '--'   \033[0m\n"
    };
    for(int i=0;i<3;i++){
        printf("\033[H\033[J");
        printf("%s", frames[i]);
        fflush(stdout);
        sleep_ms(120);
    }
    printf("\033[H\033[J");
}

void animate_bird_death(){
    if(!isatty(STDOUT_FILENO)){ printf("pew is terminated.\n"); return; }
    const char *frames[] = {
        "\033[1;32m   ,~.   \n  (  0)  \n   `-'   \033[0m\n",
        "\033[1;31m   ,~.   \n  ( x)   \n   `-'   \033[0m\n",
        "\033[1;31m   .-.   \n  ( x)   \n   /|\\  \033[0m\n",
        "\033[1;30m   ...   \n  ( x)   \n  /|\\  \033[0m\n"
    };
    for(int i=0;i<4;i++){
        printf("\033[H\033[J");
        printf("%s", frames[i]);
        fflush(stdout);
        sleep_ms(150);
    }
    printf("\033[1;31mpew has been killed. goodbye.\033[0m\n");
}

void print_help(){
    printf("pew language help:\n");
    printf("  help / h                  - show this help list\n");
    printf("  quit / exit / bye         - exit pew\n");
    printf("  kill pew                  - dramatic shell exit animation\n");
    printf("  prnt <expr>               - evaluate math or show expression\n");
    printf("  print <text>              - print text\n");
    printf("  echo <text>               - print text alias\n");
    printf("  translate <word> [lang]   - translate word to en/es/fr/de\n");
    printf("  explain <term>            - describe a keyword with a creative definition\n");
    printf("  import dictionary         - load built-in dictionary words\n");
    printf("  mode binary|ternary|decimal - switch numeral mode\n");
    printf("  set mode binary|ternary   - alias for mode\n");
    printf("  calc <expr>               - evaluate math expression\n");
    printf("  evaluate <expr>           - alias for calc\n");
    printf("  expr <expr>               - alias for calc\n");
    printf("  clear                     - clear the screen\n");
    printf("  game.new window W H       - create game window\n");
    printf("  game.new sprite NAME      - create a sprite\n");
    printf("  create sprite NAME        - alias for game.new sprite\n");
    printf("  spawn sprite NAME         - alias for game.new sprite\n");
    printf("  draw NAME                 - draw a sprite\n");
    printf("  move NAME DX DY           - move sprite\n");
    printf("  delete sprite NAME        - delete sprite\n");
    printf("  list                      - show sprites\n");
    printf("  dy/dx(expr)               - symbolic derivative\n");
    printf("  int(expr, x)              - symbolic integral (limited)\n");
    printf("  def.int(expr, x, a, b)    - numeric definite integral\n");
}

void import_dictionary(){ printf("Built-in dictionary imported (%d words)\n", dictionary_size); }

void game_create_window(int w, int h){ printf("[game] window created %dx%d\n", w, h); }
void game_create_sprite(const char *name){ if(sprite_count < 16){ strncpy(sprites[sprite_count].name, name, sizeof(sprites[sprite_count].name)-1); sprites[sprite_count].x = 0; sprites[sprite_count].y = 0; sprite_count++; printf("[game] sprite '%s' created\n", name); } else { printf("[game] sprite limit reached\n"); } }
Sprite *find_sprite(const char *name){ for(int i=0;i<sprite_count;i++) if(strcmp(sprites[i].name, name)==0) return &sprites[i]; return NULL; }
void game_draw_sprite(const char *name){ Sprite *s = find_sprite(name); if(s) printf("[game] draw sprite '%s' at (%d,%d)\n", name, s->x, s->y); else printf("[game] sprite '%s' not found\n", name); }
void game_move_sprite(const char *name, int dx, int dy){ Sprite *s = find_sprite(name); if(s){ s->x += dx; s->y += dy; printf("[game] move '%s' to (%d,%d)\n", name, s->x, s->y); } else { printf("[game] sprite '%s' not found\n", name); } }
void game_delete_sprite(const char *name){ int idx=-1; for(int i=0;i<sprite_count;i++){ if(strcmp(sprites[i].name,name)==0){ idx=i; break; } }
    if(idx<0){ printf("[game] sprite '%s' not found\n", name); return; }
    for(int i=idx;i<sprite_count-1;i++){ sprites[i] = sprites[i+1]; }
    sprite_count--; printf("[game] sprite '%s' deleted\n", name);
}
void game_list_sprites(){ if(sprite_count==0){ printf("[game] no sprites\n"); return; } for(int i=0;i<sprite_count;i++) printf("[game] sprite %s at (%d,%d)\n", sprites[i].name, sprites[i].x, sprites[i].y); }

// primary: number | variable | (expr) | dy/dx(expr) | int(...) | def.int(...)
Node* parse_primary(){ skipws(); if(match("+")) return parse_primary(); if(match("-")){ return mknode(NODE_SUB, mknum(0), parse_primary()); }
    if(src[pos]=='('){ pos++; Node *n = parse_expr(); skipws(); if(src[pos]==')') pos++; return n; }
	// dy/dx(expression)
	if(ci_strncasecmp(&src[pos],"dy/d",4)==0){ // accept dy/dx
		// consume until '(' or variable
		while(src[pos] && src[pos] != '(') pos++;
		if(src[pos]=='(') pos++; Node *ex = parse_expr(); skipws(); if(src[pos]==')') pos++;
		Node *der = derivative(ex,'x');
		return der;
	}
	// int(expr, x)
	if(ci_strncasecmp(&src[pos],"int",3)==0){ pos+=3; skipws(); if(src[pos]=='(') pos++; Node *ex = parse_expr(); skipws(); if(src[pos]==',') pos++; skipws(); char var = src[pos]; if(isalpha((unsigned char)var)) pos++; skipws(); if(src[pos]==')') pos++;
		Node *res = integrate_symbolic(ex,var);
		if(res) return res;
		// fallback: return numeric integration indicator as 0
		return mknum(0);
	}
	if(ci_strncasecmp(&src[pos],"def.int",7)==0){ pos+=7; skipws(); if(src[pos]=='(') pos++; Node *ex = parse_expr(); skipws(); if(src[pos]==',') pos++; skipws(); char var = src[pos]; if(isalpha((unsigned char)var)) pos++; skipws(); if(src[pos]==',') pos++; skipws(); // parse a
		// read numbers
		char buf[64]; int i=0; while(src[pos] && (isdigit((unsigned char)src[pos])||src[pos]=='.'||src[pos]=='-'||isspace((unsigned char)src[pos])) && i<60) buf[i++]=src[pos++]; buf[i]=0; double a = atof(buf);
		// find next number b
		while(src[pos] && !isdigit((unsigned char)src[pos]) && src[pos]!='-' ) pos++;
		i=0; while(src[pos] && (isdigit((unsigned char)src[pos])||src[pos]=='.'||src[pos]=='-') && i<60) buf[i++]=src[pos++]; buf[i]=0; double b = atof(buf);
		// compute numeric integral
		double v = definite_integral(ex,var,a,b);
		return mknum(v);
	}
	// number
	Node *num = parse_number(); if(num) return num;
	// variable (single letter)
	if(isalpha((unsigned char)src[pos])){ char c = src[pos++]; return mkvar(c); }
	return mknum(0);
}

// Evaluate a top-level statement like 'prnt expr' or 'mode binary'
void handle_line(const char *line){ src=line; pos=0; skipws();
    if(match("import")){ skipws(); if(match("dictionary")){ import_dictionary(); return; } }
    if(match("help") || match("h")){ print_help(); return; }
    if(match("version")){ print_version(); return; }
    if(match("explain") || match("define")) { explain_command(); return; }
    if(match("kill")){ if(match("pew")){ animate_bird_death(); exit(0); } }
    if(match("translate")){ translate_word_command(); return; }
    if(match("mode") || match("set")){
        if(match("mode")){
            skipws();
        }
        if(ci_strncasecmp(&src[pos],"binary",6)==0){ num_mode=2; printf("mode set to binary\n"); }
        else if(ci_strncasecmp(&src[pos],"ternary",7)==0){ num_mode=3; printf("mode set to ternary\n"); }
        else if(ci_strncasecmp(&src[pos],"decimal",7)==0){ num_mode=10; printf("mode set to decimal\n"); }
        else { printf("Usage: mode binary|ternary|decimal\n"); }
        return;
    }

    if(match("game.new") || match("game new") || match("new") || match("create")){
        skipws();
        if(match("window")){ int w = parse_int(); int h = parse_int(); game_create_window(w,h); return; }
        if(match("sprite")){ char name[64]; parse_token(name,sizeof(name)); game_create_sprite(name); return; }
        printf("Usage: game.new window W H | game.new sprite NAME | create sprite NAME\n"); return;
    }
    if(match("game.draw") || match("game draw") || match("draw")){ char name[64]; parse_token(name,sizeof(name)); game_draw_sprite(name); return; }
    if(match("game.move") || match("game move") || match("move")){ char name[64]; parse_token(name,sizeof(name)); int dx = parse_int(); int dy = parse_int(); game_move_sprite(name, dx, dy); return; }
    if(match("game.delete") || match("game delete") || match("delete")){ if(match("sprite")){ char name[64]; parse_token(name,sizeof(name)); game_delete_sprite(name); return; } char name[64]; parse_token(name,sizeof(name)); game_delete_sprite(name); return; }
    if(match("game.list") || match("list")){ game_list_sprites(); return; }
    if(match("sprite")){ char name[64]; parse_token(name,sizeof(name)); if(match("move")){ int dx = parse_int(); int dy = parse_int(); game_move_sprite(name, dx, dy); return; } if(match("draw")){ game_draw_sprite(name); return; } if(match("delete")){ char name2[64]; parse_token(name2,sizeof(name2)); game_delete_sprite(name2); return; } }
    if(match("calc") || match("evaluate") || match("expr")){
        Node *e = parse_expr(); print_node(e); printf("\n"); double v = eval(e,'x',0.0); printf("=> %g\n", v); return; }
    if(match("clear")) { printf("\033[H\033[J"); return; }
    if(match("spawn")){ if(match("sprite")){ char name[64]; parse_token(name,sizeof(name)); game_create_sprite(name); return; } char name[64]; parse_token(name,sizeof(name)); game_create_sprite(name); return; }
    if(match("make")){ if(match("sprite")){ char name[64]; parse_token(name,sizeof(name)); game_create_sprite(name); return; } }
    if(match("exit") || match("bye")){ exit(0); }

    if(match("prnt") || match("print") || match("echo")){ skipws();
        if(src[pos]=='\'' || src[pos]=='\"'){
            char quote = src[pos++]; while(src[pos] && src[pos] != quote) putchar(src[pos++]); printf("\n"); return;
        }
        char raw[1024]; int start = pos; int len = 0;
        while(src[pos] && src[pos] != '\n' && len < (int)sizeof(raw)-1){ raw[len++] = src[pos++]; }
        raw[len] = 0;
        // preserve plain text if it contains spaces or no operator sequence
        int has_op = (strchr(raw,'+') || strchr(raw,'-') || strchr(raw,'*') || strchr(raw,'/') || strchr(raw,'^') || strchr(raw,'('));
        if(strchr(raw,' ') && !has_op){ printf("%s\n", raw); return; }
        src = raw; pos = 0;
        Node *e = parse_expr(); skipws(); if(raw[pos]==')') raw[pos]=0;
        print_node(e); printf("\n");
        double v = eval(e,'x',0.0);
        printf("=> %g\n", v);
        return;
    }

    // if single word, run translation prompt
    if(is_single_word(line)){
        char word[128]; parse_token(word,sizeof(word));
        if(isatty(0)){ prompt_translate(word); return; }
        printf("translate %s <en|es|fr|de>\n", word);
        return;
    }

    // if empty or comment
    if(line[0]==0 || line[0]=='#') return;

    // otherwise try to parse expression and evaluate
    Node *e = parse_expr(); print_node(e); printf("\n");
}

int main(int argc, char **argv){
    if(argc>0){ strncpy(program_path, argv[0], sizeof(program_path)-1); program_path[sizeof(program_path)-1] = '\0'; }
    welcome_animation();
    print_banner();
    print_keyword_overview();
    print_help();
    // REPL: prefer readline for line editing/history when built with -DUSE_READLINE
#ifdef USE_READLINE
    using_history();
    char *line = NULL;
    while(1){
        line = readline("pew> ");
        if(!line) break;
        if(*line) add_history(line);
        // quick trim
        int i=0; while(line[i] && line[i]==' ') i++;
        if(strncmp(line+i,"quit",4)==0){ free(line); break; }
        handle_line(line+i);
        free(line);
    }
#else
    char buf[1024];
    while(1){ printf("pew> "); if(!fgets(buf,sizeof(buf),stdin)) break; // tolerant: strip trailing newline
        // quick trim
        int i=0; while(buf[i] && buf[i]==' ') i++;
        if(strncmp(buf+i,"quit",4)==0) break;
        handle_line(buf+i);
    }
#endif
	return 0;
}
    
    


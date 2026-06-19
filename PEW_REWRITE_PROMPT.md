# PEW ADVANCED — Complete Rewrite Prompt

> **Goal:** Take the existing PEW project (a retro CLI symbolic math shell + game engine written in C/C++) and rewrite it into a single, compact, cross-platform, modular, self-extending codebase called **PEW Advanced v2.0**. The result should compile and run on **any POSIX system, Windows, and WASM/browser**, with a built-in game engine and expanded math capabilities.

---

## 1. CONTEXT — What Exists Today

The project currently has **two divergent source files** that must be merged:

### `pew.C` (current, ~1580 lines)
The "advanced" version with:
- AST-based expression parser & evaluator (recursive descent)
- Symbolic derivatives (`d/dx`) and integrals (`∫`)
- Numerical definite integration (Simpson's rule)
- Separable ODE solver
- Quadratic factoring / root finding
- Matrix operations (create, multiply, add, det, transpose) — stored in a global array
- Statistics module (mean, median, stddev, variance, min, max)
- ASCII function plotting
- Unit conversion tables (length, mass, time)
- Mathematical constants (pi, e, phi, tau, sqrt2, ln2, ln10, inf, nan)
- Sprite-based game system (create, draw, move, delete, list) with basic AI (patrol, goto)
- Key-value memory store (`remember` / `recall`)
- Command history
- Retro ASCII boot animation
- Multi-mode number bases (binary, ternary, decimal)
- Step-by-step verbose output toggle

### `pewadvanced.C` (older, ~740 lines)
A leaner "Wolfram Alpha-style" version with:
- Same AST + parser structure but fewer features (no matrices, no stats, no sprites, no plotting)
- Cleaner code structure
- The `handle_line()` is simpler and more focused

### Backup modular structure (in `pew(backups)/`)
An attempted modular split that was abandoned:
- `assests/ast.h`, `assests/ast.C` — AST nodes
- `parser.h`, `parser.C` — expression parser
- `lexer.h`, `lexer.C` — tokenizer
- `vm.h`, `vm.C` — virtual machine / evaluator
- `main.c` — entry point
- `lib_math.h`, `lib_math.c` — math library
- `game/lib_game.h`, `game/lib_game.c` — game engine
- `gui library/lib_gui.h`, `gui library/lib_gui.C` — GUI library
- `Translation/lib_translate.h`, `Translation/lib_translate.c` — translation/i18n

---

## 2. WHAT TO BUILD — PEW Advanced v2.0

### 2.1 Architecture: Clean Modular Monolith

Build a **single-file portable C99 implementation** (`pew.c`) that compiles everywhere, BUT internally organized as clearly separated modules with `#ifdef` guards. The file structure should be:

```
pew.c                  — single-file build (like sqlite3.c or stb libraries)
pew.h                  — public header for embedding as a library
Makefile               — cross-platform build
README.md              — documentation
```

**Optional split mode** (behind `#define PEW_MODULAR`):
```
pew/
  pew_core.h           — types, config, platform detection
  pew_ast.c/.h         — AST node creation, printing, freeing
  pew_eval.c/.h        — numerical evaluation engine
  pew_parser.c/.h      — recursive descent parser + lexer
  pew_symbolic.c/.h    — symbolic differentiation & integration
  pew_numeric.c/.h     — numerical methods (integration, ODE, root finding)
  pew_algebra.c/.h     — polynomial operations, factoring, equation solving
  pew_matrix.c/.h      — matrix operations
  pew_stats.c/.h       — statistics engine
  pew_units.c/.h       — unit conversion system
  pew_plot.c/.h        — ASCII plotting engine
  pew_game.c/.h        — game engine (sprites, physics, collision, scenes)
  pew_repl.c/.h        — command handler, REPL loop, history
  pew_gui.c/.h         — optional GUI backend (SDL2/ncurses/Win32)
  pew_platform.c/.h    — platform abstraction layer
  main.c               — entry point
```

### 2.2 Cross-Platform & Self-Extending

**Platform detection macro** at the top of `pew.c`:
```c
#if defined(_WIN32) || defined(_WIN64)
  #define PEW_PLATFORM_WINDOWS
  #include <windows.h>
  #include <io.h.h>
#elif defined(__EMSCRIPTEN__)
  #define PEW_PLATFORM_WASM
  #include <emscripten.h>
#elif defined(__APPLE__)
  #define PEW_PLATFORM_MACOS
  #include <TargetConditionals.h>
#elif defined(__linux__)
  #define PEW_PLATFORM_LINUX
  #elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
  #define PEW_PLATFORM_BSD
#else
  #define PEW_PLATFORM_UNKNOWN
#endif
```

**Platform abstraction layer:**
- `pew_sleep_ms()` — uses `nanosleep` (POSIX), `Sleep()` (Windows), or `emscripten_sleep()` (WASM)
- `pew_terminal_size()` — uses `ioctl TIOCGWINSZ` (POSIX), `GetConsoleScreenBufferInfo` (Windows), or fixed 80x24
- `pew_clear_screen()` — uses `\033[2J\033[H` (ANSI), `system("cls")` (Windows fallback), or `emscripten_run_script`
- `pew_readline()` — uses `readline()` if available, `fgets()` fallback, or WASM async prompt
- `pew_file_exists()`, `pew_mkdir()`, `pew_get_home_dir()` — filesystem abstraction
- `pew_detect_platform()` — runtime function that prints OS, arch, compiler, terminal info

**Self-extending behavior:**
- A `platform` command that auto-detects and reports the environment
- A `plugin load <path>` command that can `dlopen`/`LoadLibrary` a shared library conforming to a plugin ABI:
  ```c
  typedef struct {
      const char *name;
      const char *version;
      int (*init)(void);
      int (*handle_command)(const char *cmd, const char *args);
      void (*shutdown)(void);
  } PewPlugin;
  ```
- A `pew --compile-check` mode that verifies the current platform supports all features and warns about fallbacks
- Conditional compilation blocks that automatically enable/disable features based on available headers

### 2.3 Merged Features (from both files + enhancements)

Keep ALL features from `pew.C` (the more complete version) and fold in the cleaner patterns from `pewadvanced.C`. Specifically:

#### AST & Parser
- Recursive descent parser with proper operator precedence: `+`/`-` < `*`/`/` < `^` < unary
- Support for: numbers (int/float), variables (a-z, A-Z), named constants
- Function parsing: `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `sinh`, `cosh`, `tanh`, `ln`, `log`, `log2`, `log10`, `exp`, `sqrt`, `cbrt`, `abs`, `ceil`, `floor`, `round`, `sign`, `gamma`, `erf`, `factorial`
- Multi-character variable names (e.g., `alpha`, `theta`) — store as string key, not just char
- Implicit multiplication: `2x` → `2*x`, `2(x+1)` → `2*(x+1)`, `x(x+1)` → `x*(x+1)`, `2sin(x)` → `2*sin(x)`
- Percentage operator: `50%` → `0.5`
- Factorial operator: `5!` → `factorial(5)`
- Comparison operators: `==`, `!=`, `<`, `>`, `<=`, `>=` (return 0.0 or 1.0)
- Logical operators: `&&`, `||`, `!` (treat nonzero as true)
- Ternary operator: `cond ? a : b`
- Assignment operator: `x = expr` (evaluates and stores)
- Compound assignment: `x += expr`, `x -= expr`, `x *= expr`, `x /= expr`

#### Symbolic Mathematics (Enhanced)

**Differentiation** — keep all existing rules + add:
- Chain rule: `d/dx f(g(x)) = f'(g(x)) * g'(x)` (already implicit, make it more robust)
- Product rule with 3+ factors: `d/dx(uvw) = u'vw + uv'w + uvw'`
- Logarithmic differentiation: `d/dx(u^v) = u^v * (v'*ln(u) + v*u'/u)`
- Partial derivatives for multi-variable expressions
- Step-by-step differentiation with explanations (when `steps on`)
- Differentiate with respect to any variable (not just x)

**Integration** — keep existing + add:
- Integration by parts: `∫ u dv = uv - ∫ v du` (detect `x*sin(x)`, `x*exp(x)`, `x*ln(x)`, etc.)
- Trig substitution patterns
- Partial fraction decomposition: `1/((x-a)(x-b))` → `A/(x-a) + B/(x-b)`
- Standard integral table lookup (sec², csc², sec, csc, cot, hyperbolic integrals)
- Numerical integration: Simpson's rule (existing) + add Gaussian quadrature + Romberg integration
- Improper integral support (infinite limits with convergence check)
- Double/triple integrals (numerical)

**New Math Functions to Add:**
- `sinh(x)`, `cosh(x)`, `tanh(x)`, `asinh(x)`, `acosh(x)`, `atanh(x)` — hyperbolic
- `log2(x)`, `log10(x)`, `log(base, x)` — generalized logarithm
- `gamma(x)`, `ln_gamma(x)` — gamma function (Lanczos approximation)
- `erf(x)`, `erfc(x)` — error function
- `zeta(x)` — Riemann zeta function (numerical)
- `beta(a, b)` — beta function
- `bessel_j(n, x)`, `bessel_y(n, x)` — Bessel functions
- `factorial(n)` / `n!` — factorial (using gamma for non-integers)
- `nCr(n, r)`, `nPr(n, r)` — combinatorics
- `gcd(a, b)`, `lcm(a, b)` — number theory
- `mod(a, m)` — modulo operation
- `clamp(x, lo, hi)` — clamping
- `lerp(a, b, t)` — linear interpolation
- `smoothstep(a, b, x)` — smooth interpolation (useful for game engine)

**Differential Equations** — enhance existing:
- Separable ODEs (existing)
- First-order linear ODEs: `y' + P(x)y = Q(x)` using integrating factor
- Exact ODEs
- Numerical ODE solvers: Euler method, Runge-Kutta 4th order (RK4)
- System of ODEs (numerical)
- Initial value problem solver

**Polynomial & Algebra** — enhance existing:
- Factor any polynomial up to degree 4 (quadratic, cubic, quartic formulas)
- Expand polynomial expressions
- Polynomial long division
- Synthetic division
- Polynomial GCD
- Rational root theorem
- Vieta's formulas
- Systems of linear equations (Gaussian elimination, Cramer's rule)
- Inequality solving

#### Matrix Operations (Enhanced)
- Keep existing: create, multiply, add, det, transpose
- Add: inverse (Gauss-Jordan), eigenvalues (power iteration), trace, rank, row echelon form, reduced row echelon form, LU decomposition, QR decomposition, matrix exponential, determinant minors, cofactor matrix, adjugate matrix
- Named matrix storage (keep existing system)
- Support for complex number matrices (if feasible)
- Matrix formatting: aligned columns, scientific notation option

#### Statistics (Enhanced)
- Keep existing: mean, sum, min, max, median, stddev, variance
- Add: mode, range, IQR (interquartile range), percentiles, z-score, correlation coefficient (Pearson r), linear regression, exponential regression, polynomial regression, histograms (ASCII), box plots (ASCII), confidence intervals, hypothesis testing basics

#### Complex Numbers (from complexnum.c)
- Read and integrate the existing `complexnum.c`:
  - `complex_add`, `complex_sub`, `complex_mul`, `complex_div`, `complex_mod`, `complex_arg`
  - `complex_pow`, `complex_sqrt`, `complex_exp`, `complex_log`, `complex_sin`, `complex_cos`
  - Display as `a + bi` format
- Add to AST: `NODE_COMPLEX` type, complex evaluation mode
- Complex number solver: roots of unity, complex polynomial roots

#### Number Systems & Bases
- Keep binary/ternary/decimal modes
- Add: hexadecimal (base 16), octal (base 8), base-N (any base 2-36)
- `base CONVERT FROM TO VALUE` command
- Arbitrary-precision integer support for factorials and large numbers (use string-based arithmetic or a lightweight big-int library)

#### Unit Conversion (Enhanced)
- Keep existing length, mass, time units
- Add: temperature (°C, °F, K, °R), area, volume, speed, data (bit, byte, KB, MB, GB, TB), force, energy, pressure, angle (deg, rad, grad)
- Chain conversion: `convert 5 km to miles to feet`
- Custom unit definitions: `unit new NAME = VALUE BASE_UNIT`

#### ASCII Plotting (Enhanced)
- Keep existing 60x20 ASCII plot
- Add: parametric plots `plot parametric expr_x(t), expr_y(t)`
- Polar plots `plot polar r(theta)`
- Multiple functions on same plot (different characters: `*`, `+`, `x`, `o`)
- Interactive zoom: `plot EXPR xmin xmax ymin ymax`
- Color output using ANSI escape codes
- Larger plots: auto-detect terminal width

#### Constants (Enhanced)
- Keep existing: pi, e, phi, tau, sqrt2, ln2, ln10, inf, nan
- Add: c (speed of light), G (gravitational constant), h (Planck), k_B (Boltzmann), e (elementary charge), m_e (electron mass), m_p (proton mass), mu_0, epsilon_0, hbar, avogadro, lightyear, parsec, au
- User-defined constants: `const NAME = VALUE`
- Physical unit system awareness (CGS, SI, Imperial)

---

### 2.4 Game Engine (New — Major Feature)

Build a proper **2D game engine** inside PEW, exposed through REPL commands. This should be a genuine, usable game engine, not just sprite primitives.

#### Core Engine Architecture

```
pew_game.h:
  - Game state management (init, update, render, shutdown)
  - Entity-Component System (ECS) lite
  - Scene / state machine
  - Input handling (keyboard buffer)
  - Collision detection
  - Timer / delta time
  - Random number generation
  - Asset management (load/save)
  - Event system (pub/sub)
```

#### Entities & Components

```c
// Entity = unsigned int ID
// Components stored in contiguous arrays (SoA layout for cache efficiency)

typedef struct {
    float x, y;           // position
    float vx, vy;         // velocity
    float ax, ay;         // acceleration
    float mass;           // mass (for physics)
    float friction;       // friction coefficient
    int active;           // alive or dead
} Transform;

typedef struct {
    float w, h;           // bounding box
    int is_circle;        // circle vs rectangle
    float radius;         // for circle collision
    int is_static;        // immovable objects
} Collider;

typedef struct {
    unsigned char r, g, b, a;  // color
    char glyph;                 // ASCII character to render
    int visible;
} Sprite;

typedef struct {
    int health;
    int max_health;
    int damage;
    int score;
} Stats;

typedef struct {
    char name[32];        // entity name for lookup
    int tags[8];          // tag flags (player, enemy, item, etc.)
    int tag_count;
} Identity;

typedef struct {
    int type;             // 0=none, 1=patrol, 2=flee, 3=chase, 4=wander, 5=follow_path
    int target_id;        // entity to chase/flee from
    float speed;          // movement speed
    float detection_range;
    int patrol_points_count;
    float patrol_x[8], patrol_y[8];
    int current_patrol;
    int state;            // AI sub-state
} AI_Behavior;

typedef struct {
    int type;             // 0=none, 1=loop, 2=once, 3=pingpong
    float frame_time;     // seconds per frame
    int frame_count;
    int current_frame;
    float timer;
    char glyphs[16];      // up to 16 animation frames
} Animation;

// Game world
typedef struct {
    int width, height;        // world dimensions
    int camera_x, camera_y;   // camera offset (for scrolling)
    
    // Entity storage (max 256 entities)
    Transform transforms[256];
    Collider colliders[256];
    Sprite sprites[256];
    Stats stats[256];
    Identity identities[256];
    AI_Behavior ai_behaviors[256];
    Animation animations[256];
    int entity_count;
    int active_flags[256];     // 1 = entity slot in use
    
    // Physics
    float gravity_x, gravity_y;
    float world_friction;
    int physics_enabled;
    
    // Scene
    char scene_name[64];
    int tick_count;
    float delta_time;
    float elapsed_time;
    int paused;
    int running;
    
    // Collision pairs (for events)
    int collision_pairs[256][2];
    int collision_count;
    
    // Events queue
    char events[32][256];
    int event_count;
    
    // ASCII framebuffer
    char framebuffer[120][40];   // 120 wide x 40 tall
    unsigned char color_buffer[120][40][4]; // RGBA color per cell
    
    // Random seed
    unsigned int rng_state;
} GameWorld;
```

#### Game REPL Commands

```bash
# Scene management
game init WIDTH HEIGHT        — initialize game world
game start                    — begin game loop (renders to terminal)
game stop                     — stop game loop
game pause / resume           — toggle pause
game reset                    — clear all entities and reset
game save FILENAME            — save game state to file
game load FILENAME            — load game state from file

# Entity management
game spawn NAME               — create entity
game destroy NAME             — remove entity
game destroy all              — remove all entities
game list                     — list all entities
game info NAME                — show entity details

# Transform
game pos NAME [X Y]           — get or set position
game move NAME DX DY          — apply movement
game velocity NAME [VX VY]    — get or set velocity
game accel NAME AX AY         — set acceleration

# Physics
game physics on|off           — toggle physics
game gravity GX GY            — set gravity vector
game friction VALUE           — set world friction
game impulse NAME FX FY       — apply instant force

# Collision
game collider NAME [W H]      — set bounding box
game collider NAME circle R   — set circle collider
game collide? NAME1 NAME2     — test collision
game collisions               — show all current collisions

# Visuals
game sprite NAME [GLYPH COLOR] — set render glyph and color
game color NAME R G B         — set color (0-255 or 0-15 for ANSI)
game render                   — manual render frame
game camera X Y               — set camera offset
game fill CHAR                — fill background

# AI
game ai NAME patrol X1 Y1 X2 Y2 ... — set patrol path
game ai NAME chase TARGET_NAME       — chase behavior
game ai NAME flee TARGET_NAME        — flee behavior
game ai NAME wander RANGE            — random movement
game ai NAME follow PATH_FILE        — follow waypoints from file

# Animation
game anim NAME GLYPHS FPS     — set animation (e.g., game anim fire ".*oO" 8)
game anim play NAME           — start animation
game anim stop NAME           — stop animation

# Game loop control
game tick                     — advance one frame (manual mode)
game ticks N                  — advance N frames
game fps [VALUE]              — get or set target FPS
game render interval N        — render every N ticks

# Scene scripting (simple event system)
game on collision NAME1 NAME2 "COMMAND"  — trigger command on collision
game on tick "COMMAND"                    — trigger command every tick
game on key KEY "COMMAND"                 — trigger command on keypress
game trigger NAME                         — fire named event

# Built-in game templates
game template snake           — start a Snake game
game template pong            — start a Pong game
game template breakout        — start Breakout
game template maze            — generate random maze
game template tetris          — start Tetris (if feasible in terminal)

# Debug
game debug on|off             — toggle debug overlay
game perf                     — show performance stats
game dump                     — dump entire game state
```

#### Built-in Game Templates

Each template is a function that sets up entities, AI, physics, and game rules:

1. **Snake** — classic snake game with food, growing, scoring
2. **Pong** — two paddles + ball, AI vs player
3. **Breakout** — paddle, ball, brick grid with destructible blocks
4. **Maze** — random maze generation + player navigation
5. **Tetris** — falling blocks (if terminal supports it)

#### Game Scripting Language

Support inline scripts within the REPL for game logic:
```bash
# Simple scripting
game script my_game "
  spawn player
  sprite player @ 0 15
  collider player 1 1
  spawn food
  pos food 10 10
  sprite food * green
  ai food wander 20
  on collision player food 'score += 1; pos food rand(20) rand(20)'
"
```

---

### 2.5 REPL Enhancements

- **Tab completion** (when readline is available): complete command names, math function names, entity names
- **Persistent history**: save/load command history to `~/.pew_history`
- **Config file**: `~/.pewrc` — auto-load commands on startup
- **Aliases**: `alias NAME = COMMAND` — user-defined command shortcuts
- **Macros**: `macro NAME(params) = BODY` — parameterized command sequences
- **Output formatting**: `set precision N`, `set notation scientific|fixed|auto`
- **Color themes**: `theme retro|matrix|ocean|fire|monochrome`
- **Line editor**: when no readline, implement a basic one with arrow key support
- **Pipe support**: `pew -e "2+3"` for non-interactive use
- **Script mode**: `pew script.pew` — execute commands from a file line by line
- **Batch mode**: `pew -b "cmd1; cmd2; cmd3"` — execute multiple commands

---

### 2.6 Documentation & Self-Documentation

- `help` command shows all available commands grouped by category
- `help COMMAND` shows detailed help for a specific command with examples
- `man COMMAND` shows a man-page style detailed reference
- `examples` shows example usage for each feature
- `tutorial` walks through basic usage interactively
- Auto-generate a `MANUAL.md` from the help system

---

## 3. CODE QUALITY REQUIREMENTS

### Style
- **Language:** C99 (not C++) — maximum portability. Use `g++` only if C++ features are specifically needed.
- **Single-header or single-file build option:** like stb libraries (`#define PEW_IMPLEMENTATION` in header, or compile `pew.c` directly)
- **Naming:** `pew_` prefix for all public functions. `snake_case` for functions, `UPPER_CASE` for macros/constants, `PascalCase` for type names
- **No global mutable state** except the single `GameWorld` instance and REPL state (which are explicitly initialized)
- **No dynamic allocation after init** — use fixed-size arrays (like existing sprite/matrix stores). If dynamic allocation is needed, use a simple arena allocator.
- **Error handling:** return error codes, never crash. Print meaningful error messages with `[pew] ERROR: description`.
- **Memory safety:** no buffer overflows. Use `snprintf`, bounds checks on all array accesses.

### Portability
- **No POSIX-only headers** in the core. Wrap `unistd.h`, `sys/time.h`, `sys/select.h` behind `#ifdef PEW_HAS_POSIX`
- **No Windows-only headers** in the core. Wrap `windows.h` behind `#ifdef PEW_HAS_WIN32`
- **Sleep:** Use `struct timespec` + `nanosleep` on POSIX, `Sleep()` on Windows, `emscripten_sleep()` on WASM
- **Terminal size:** Use `ioctl TIOCGWINSZ` on POSIX, `GetConsoleScreenBufferInfo` on Windows, default 80x24 on WASM
- **Readline:** Wrap all readline usage behind `#ifdef PEW_HAS_READLINE`. Provide `fgets` fallback and a basic line editor fallback.
- **Math functions:** Use `<math.h>` everywhere. For `tgamma`, `erf`, `cbrt` which may be missing on some systems, provide fallback implementations:
  ```c
  #ifndef PEW_HAS_TGAMMA
  double tgamma(double x) { /* Lanczos approximation */ }
  #endif
  ```

### Build System
```makefile
# Makefile
CC ?= gcc
CFLAGS = -O2 -Wall -Wextra -std=c99 -DPEW_IMPLEMENTATION
LDFLAGS = -lm

# Platform detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    CFLAGS += -DPEW_HAS_POSIX
    LDFLAGS += -ldl  # for dlopen (plugins)
endif
ifeq ($(UNAME_S),Darwin)
    CFLAGS += -DPEW_HAS_POSIX
endif
ifneq (,$(findstring MINGW,$(UNAME_S)))
    CFLAGS += -DPEW_HAS_WIN32
endif
ifneq (,$(findstring MSYS,$(UNAME_S)))
    CFLAGS += -DPEW_HAS_WIN32
endif

# Optional readline
ifdef USE_READLINE
    CFLAGS += -DPEW_HAS_READLINE
    LDFLAGS += -lreadline
endif

# WASM build
wasm:
    emcc pew.c -o pew.html -O2 -s WASM=1 -s EXIT_RUNTIME=1 \
        -DPEW_PLATFORM_WASM -DPEW_HAS_POSIX

all: pew

pew: pew.c
	$(CC) $(CFLAGS) pew.c -o pew $(LDFLAGS)

install: pew
	install -Dm755 pew $(HOME)/.local/bin/pew

clean:
	rm -f pew pew.html pew.wasm

.PHONY: all wasm install clean
```

### Testing
- Include a `pew_test.c` or a `--test` flag that runs built-in self-tests:
  - Parser tests: verify known expressions parse correctly
  - Evaluator tests: verify numerical results
  - Derivative tests: verify symbolic derivatives match expected
  - Integration tests: verify definite integrals are within tolerance
  - Matrix tests: verify operations
  - Game engine tests: verify entity creation, collision detection

---

## 4. SPECIFIC IMPLEMENTATION INSTRUCTIONS

### Step 1: Platform Layer
Write `pew_platform.h` with all platform detection macros and abstraction functions. Test that it compiles on Linux, macOS, Windows (MinGW), and WASM (Emscripten).

### Step 2: Core AST & Parser
Merge the parser from `pew.C` (more complete) with the cleaner structure from `pewadvanced.C`. Add:
- Multi-character variable names
- Implicit multiplication
- All new function nodes (sinh, cosh, gamma, erf, etc.)
- Proper error recovery (don't crash on bad input, print error and continue)

### Step 3: Symbolic Engine
Enhance the derivative and integration functions to handle:
- Chain rule more robustly (deep composition)
- Integration by parts detection
- Partial fractions
- Step-by-step output

### Step 4: Numerical Engine
Add RK4 ODE solver, Gaussian quadrature, complex number arithmetic, improved root finding (Newton's method + bisection hybrid).

### Step 5: Game Engine
Implement the ECS-lite architecture, ASCII renderer, collision detection, AI behaviors, and at least 2 game templates (Snake and Pong).

### Step 6: REPL & Polish
Add tab completion, config file, aliases, color themes, scripting mode, and comprehensive help system.

### Step 7: Cross-Platform Testing
Compile and test on Linux, macOS, Windows (MinGW/MSYS2), and WASM (Emscripten). Fix all platform-specific issues.

---

## 5. EXAMPLE COMMANDS (what the final product should support)

```
pew> 2^10
=> 1024

pew> x = 5
x = 5

pew> dy/dx(x^3 + 2sin(x))
d/dx(x^3 + 2sin(x)) = (3x^2 + 2cos(x))

pew> int(x^2 * e^x, x)
Integral = (x^2 - 2x + 2)e^x

pew> def.int(sin(x)^2, x, 0, pi)
=> 1.5708

pew> sinh(2)
=> 3.62686

pew> gamma(5)
=> 24

pew> nCr(10, 3)
=> 120

pew> solve x^3 - 6x^2 + 11x - 6 = 0
Roots: x = 1, x = 2, x = 3

pew> mat create 3 3 1 2 3 4 5 6 7 8 9 A
[mat] created 3x3 matrix 'A'

pew> mat det A
det(A) = 0

pew> stats push 10
pew> stats push 20
pew> stats push 30
pew> stats mean
mean = 20

pew> convert 100 km to miles
=> 62.1371 miles

pew> plot sin(x) -5 5
    [ASCII plot]

pew> game init 40 20
[game] world 40x20 initialized

pew> game spawn player
[game] entity 'player' created

pew> game pos player 20 10
[game] player positioned at (20,10)

pew> game sprite player @ white
[game] player rendered as '@' in white

pew> game collider player 1 1
[game] player collider: 1x1

pew> game physics on
[game] physics enabled, gravity = (0, 1)

pew> game ai slime patrol 5 5 35 5 35 15 5 15
[game] slime patrolling 4 points

pew> game start
    [renders game to terminal in real-time]

pew> game template snake
    [starts snake game]

pew> complex (3+4i) * (2-i)
=> 10 + 5i

pew> base 16 to 10 FF
=> 255

pew> factorial 20
=> 2432902008176640000

pew> plot polar sin(3*theta) 0 6.28
    [polar rose plot]

pew> 12c^2 / 2
=> 72

pew> help
    [comprehensive help output]
```

---

## 6. DELIVERABLES

1. **`pew.c`** — complete single-file implementation (~2500-4000 lines, well-organized with clear section headers)
2. **`pew.h`** — public API header for embedding
3. **`Makefile`** — cross-platform build with WASM target
4. **`README.md`** — usage documentation, feature list, build instructions for all platforms
5. **`pew_manual.md`** — auto-generated style manual from help system

---

## 7. CONSTRAINTS

- **C99 standard** — no C++ features unless behind `#ifdef PEW_HAS_CPP`
- **No external dependencies** except libc, libm, and optionally libreadline
- **Single-file build must work** with `gcc -o pew pew.c -lm` (no other flags needed for basic build)
- **Maximum ~4000 lines** for the single file — be concise but readable
- **Every public function must have a doc comment** explaining purpose, parameters, return value
- **No compiler warnings** with `-Wall -Wextra` on GCC and Clang
- **The game engine must actually work** — Snake and Pong should be playable in the terminal

---

*This prompt is designed to be given to an AI coding assistant. The AI should read the existing `pew.C` and `pewadvanced.C` files, understand the full feature set, and produce the v2.0 rewrite following the architecture and requirements described above.*

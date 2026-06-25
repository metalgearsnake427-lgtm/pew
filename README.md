# pew-programming-language

A shell-style programming language written in C


This is a programming language made in C, built as a compact retro shell for symbolic math, calculus, and simple game-style commands.

pew — advanced shell-style programming language

`pew` is designed to be easier and more expressive than other small languages, while remaining terminal friendly and compact. It mixes math, symbolic calculus, translation, simple game objects, and creative command definitions in a single REPL.

Why `pew` is easier than other languages:

- single-source C program with no build system required
- natural command keywords and aliases
- built-in math, symbolic calculus, and numeric modes
- game-style sprite creation and movement via shell commands
- terminal animation and fun visual feedback

Supported commands:

- `help` / `h` — show command list
- `quit` / `exit` / `bye` — exit pew
- `kill pew` — dramatic bird-death terminal animation and exit
- `mode binary` / `mode ternary` / `mode decimal` — switch numeric literal mode
- `set mode binary` / `set mode ternary` — alias for mode commands
- `prnt <expr>` / `expr <expr>` / `calc <expr>` — evaluate math expressions
- `print <text>` / `echo <text>` — print text or values
- `translate <word> [lang]` — translate a built-in word to another language
- `explain <term>` / `define <term>` — get a vivid definition of a keyword
- `import dictionary` — load the built-in dictionary
- `clear` — clear the screen
- `game.new window W H` — create a placeholder game window
- `game.new sprite NAME` / `create sprite NAME` / `spawn sprite NAME` — create a sprite
- `draw NAME` — draw a sprite
- `move NAME DX DY` — move a sprite
- `delete sprite NAME` — delete a sprite
- `list` / `game.list` — show current sprites
- `dy/dx(expr)` — symbolic derivative with respect to `x`
- `int(expr, x)` — symbolic integral for simple power rules
- `def.int(expr, x, a, b)` — numeric definite integral approximation

Examples:

- `mode binary` then `prnt 101 + 1`
- `prnt (x^2 + 3*x + 1)`
- `prnt dy/dx(x^3)` — prints derivative
- `prnt def.int(x^2, x, 0, 1)` — numeric integral
- `translate hello es`
- `explain ternary`
- `game.new sprite hero`
- `move hero 3 2`
- `kill pew`

Build and run:

```sh
gcc "pew.C" -o pew -lm
./pew
```

Install:

```sh
install -Dm755 pew ~/.local/bin/pew
```

Then run:

```sh
pew
```

Use these commands directly in your terminal to explore `pew`. It aims to be more advanced, more expressive, and easier than many other programming languages.

it is open source anyone can edit as you wish

#!/bin/sh
set -e


INSTALL_DIR="$HOME/.local/bin"
mkdir -p "$INSTALL_DIR"

# Prefer Makefile build (with readline) when available
if [ -f Makefile ] && command -v make >/dev/null 2>&1; then
  if make -q build 2>/dev/null; then
    if make build; then
      install -m 755 pew "$INSTALL_DIR/pew"
    else
      echo "Make build failed; falling back to gcc..."
      gcc -x c "pew.C" -o "$INSTALL_DIR/pew" -lm
    fi
  else
    echo "Makefile has no build target or it is unavailable; using gcc fallback."
    gcc -x c "pew.C" -o "$INSTALL_DIR/pew" -lm
  fi
}else
  gcc -x c "pew.C" -o "$INSTALL_DIR/pew" -lm
fi
chmod +x "$INSTALL_DIR/pew"

echo "Installed pew to $INSTALL_DIR/pew"
if [ -n "$PATH" ] && echo "$PATH" | grep -q "$INSTALL_DIR"; then
  echo "Ready to use: pew"
else
  echo "Add $INSTALL_DIR to PATH if needed:"
  echo "  set -gx PATH \"$INSTALL_DIR\" \$PATH"
  echo "or add it to ~/.config/fish/config.fish"
fi

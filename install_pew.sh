#!/bin/sh
set -e


INSTALL_DIR="$HOME/.local/bin"
mkdir -p "$INSTALL_DIR"

gcc -x c "pew.C" -o "$INSTALL_DIR/pew" -lm
chmod +x "$INSTALL_DIR/pew"

echo "Installed pew to $INSTALL_DIR/pew"
if [ -n "$PATH" ] && echo "$PATH" | grep -q "$INSTALL_DIR"; then
  echo "Ready to use: pew"
else
  echo "Add $INSTALL_DIR to PATH if needed:"
  echo "  set -gx PATH \"$INSTALL_DIR\" \$PATH"
  echo "or add it to ~/.config/fish/config.fish"
fi

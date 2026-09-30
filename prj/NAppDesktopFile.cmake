#------------------------------------------------------------------------------
# This is part of NAppGUI build system
# See README.md and LICENSE.txt
#------------------------------------------------------------------------------
# Writes a '.desktop' file for a Linux desktop app into the user's own XDG applications
# directory, so GNOME's Dock/taskbar (and other desktops) can resolve the app's icon from
# its very first launch onward -- matches the app_id/icon each NAppGUI app also registers
# itself at runtime (src/osapp/gtk/osapp_gtk.c), but done at build time so the file already
# exists before the freshly built binary is ever run.
#
# Invoked as: cmake -DAPP_ID=... -DAPP_NAME=... -DEXE_PATH=... -DICON_PATH=... -DDESKTOP_DIR=... -P NAppDesktopFile.cmake
#------------------------------------------------------------------------------

file(MAKE_DIRECTORY "${DESKTOP_DIR}")

file(WRITE "${DESKTOP_DIR}/${APP_ID}.desktop"
"[Desktop Entry]
Type=Application
Name=${APP_NAME}
Exec=${EXE_PATH}
Icon=${ICON_PATH}
Terminal=false
StartupWMClass=${APP_ID}
")

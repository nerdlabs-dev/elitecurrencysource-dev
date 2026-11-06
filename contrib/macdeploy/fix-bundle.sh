#!/usr/bin/env bash
# Finish bundling a macOS .app after macdeployqt:
#   contrib/macdeploy/fix-bundle.sh Elite.app
#
# macdeployqt copies the libraries an app links directly, but misses ones that
# Homebrew libraries load from each other through @loader_path or @rpath
# (e.g. freetype -> brotlidec -> brotlicommon). Such an app runs on the build
# machine and crashes everywhere else. This walks every Mach-O file in the app,
# copies in any library that is referenced from outside the app or can't be
# found inside it, points the reference at the copy, and repeats until nothing
# is missing. Then it re-signs the app ad hoc (Apple Silicon requires a
# signature, and changing a file invalidates it).
set -euo pipefail

APP="${1%/}"
FW="$APP/Contents/Frameworks"
mkdir -p "$FW"
BREW="$(brew --prefix 2>/dev/null || echo /opt/homebrew)"

machos() {
    find "$APP" -type f \( -name '*.dylib' -o -perm -u+x \) -print0 |
        while IFS= read -r -d '' f; do
            file "$f" | grep -q 'Mach-O' && printf '%s\n' "$f"
        done
}

# Find a library by file name among Homebrew's installed packages
find_in_brew() {
    local name="$1" c
    for c in "$BREW/lib/$name" "$BREW"/opt/*/lib/"$name"; do
        [ -f "$c" ] && { printf '%s\n' "$c"; return 0; }
    done
    return 1
}

# Does a reference resolve to a file inside the app, seen from file $1?
resolves() {
    local f="$1" ref="$2" dir
    dir="$(dirname "$f")"
    case "$ref" in
        @executable_path/*) [ -f "$APP/Contents/MacOS/${ref#@executable_path/}" ] ;;
        @loader_path/*)     [ -f "$dir/${ref#@loader_path/}" ] ;;
        @rpath/*)           [ -f "$FW/${ref#@rpath/}" ] ;;
        *)                  return 1 ;;
    esac
}

changed=1
round=0
while [ "$changed" = 1 ]; do
    changed=0
    round=$((round + 1))
    [ "$round" -le 10 ] || { echo "fix-bundle: still changing after 10 rounds"; exit 1; }
    while IFS= read -r f; do
        id="$(otool -D "$f" | tail -n +2)"
        otool -L "$f" | tail -n +2 | awk '{print $1}' | while IFS= read -r ref; do
            [ "$ref" = "$id" ] && continue
            case "$ref" in
                /System/*|/usr/lib/*) continue ;;
            esac
            if [[ "$ref" == /* ]] && [[ "$ref" != "$APP"/* ]]; then
                src="$ref"                       # absolute path outside the app
            elif resolves "$f" "$ref"; then
                continue                         # already found inside the app
            else
                src="$(find_in_brew "$(basename "$ref")")" || {
                    echo "fix-bundle: $f needs $ref, which isn't in Homebrew either"; exit 1; }
            fi
            name="$(basename "$src")"
            if [ ! -f "$FW/$name" ]; then
                echo "fix-bundle: adding $name (needed by ${f#$APP/})"
                cp -L "$src" "$FW/$name"
                chmod u+w "$FW/$name"
                install_name_tool -id "@executable_path/../Frameworks/$name" "$FW/$name"
            fi
            echo "fix-bundle: ${f#$APP/}: $ref -> @executable_path/../Frameworks/$name"
            install_name_tool -change "$ref" "@executable_path/../Frameworks/$name" "$f"
            touch "$APP/.fix-bundle-changed"
        done
    done < <(machos)
    if [ -f "$APP/.fix-bundle-changed" ]; then
        rm -f "$APP/.fix-bundle-changed"
        changed=1
    fi
done

codesign --force --deep --sign - "$APP"
codesign --verify --deep --strict "$APP"
echo "fix-bundle: done"

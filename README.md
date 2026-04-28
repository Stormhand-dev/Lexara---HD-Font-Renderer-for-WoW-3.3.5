# Lexara — HD Font Renderer for WoW 3.3.5

**Lexara** is a lightweight DLL that replaces WoW 3.3.5's native font renderer 
with **MSDF** (Multi-channel Signed Distance Field) technology, delivering sharp, 
resolution-independent text across the entire game UI — combat numbers, nameplates, 
tooltips, chat, and more.

> Drop `dinput8.dll` into your WoW root folder. No addon, no configuration file, no setup required.

---

## Installation

**Requirements:** World of Warcraft 3.3.5 (build 12340)

1. Download and extract `Lexara.zip` from [Releases](../../releases)
2. Copy `dinput8.dll` and `skia.dll` to your WoW root folder (where `WoW.exe` is located)
3. Launch the game

**Files included**
- `dinput8.dll` — Lexara font renderer
- `skia.dll` — required graphics dependency

---

## Compatibility

Lexara coexists with ReShade and other DLL-based mods. If `dinput8.dll` is already 
taken by another mod, rename Lexara to `version.dll` or `dsound.dll` — all three 
names work identically.

---

## Uninstall

Delete `dinput8.dll` and `skia.

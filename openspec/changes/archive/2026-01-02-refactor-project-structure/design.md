## Context

Project needs to be organized for GitHub open source release with clear structure for contributors.

## Current Structure

```
flydigi-vader5/
├── 99-vader5.rules          # udev rules (root level)
├── AGENTS.md                # AI instructions
├── CLAUDE.md                # AI instructions
├── CMakeLists.txt
├── driver/                  # kernel driver + build artifacts
├── include/vader5/          # headers
├── openspec/                # specs
├── research/                # protocol docs (scattered)
├── scripts/                 # unknown
└── src/                     # all source mixed
```

## Proposed Structure

```
flydigi-vader5/
├── .github/
│   ├── workflows/ci.yml
│   └── ISSUE_TEMPLATE/
│       ├── bug_report.md
│       └── feature_request.md
├── docs/
│   ├── protocol.md          # USB protocol (from research/)
│   ├── building.md          # Build instructions
│   └── steam-input.md       # Steam setup guide
├── driver/
│   ├── hid-flydigi.c
│   ├── Makefile
│   └── dkms.conf
├── include/vader5/
│   └── *.hpp
├── install/
│   ├── 99-vader5.rules
│   ├── install.sh
│   └── uninstall.sh
├── src/
│   ├── hidraw.cpp           # Core library files (flat)
│   ├── uinput.cpp
│   ├── transport.cpp
│   ├── gamepad.cpp
│   ├── config.cpp
│   ├── daemon/              # vader5d daemon
│   │   └── main.cpp
│   └── tools/               # Standalone tools
│       └── debug.cpp        # vader5-debug TUI
├── openspec/
├── .clang-format
├── .clang-tidy
├── .gitignore
├── CMakeLists.txt
├── CONTRIBUTING.md
├── LICENSE
└── README.md
```

## src/ Organization

| Directory | Purpose | Outputs |
|-----------|---------|---------|
| src/ | Core library (hidraw, uinput, transport, gamepad, config) | Built as object library |
| src/daemon/ | Main driver daemon | vader5d |
| src/tools/ | Debug and utility tools | vader5-debug |

Benefits:
- Simple flat structure for core code
- Only executables get subdirectories
- Easy to add new tools (e.g., vader5-dsu)

## Decisions

### 1. Keep core files flat, separate executables
Rationale: Simple structure, only daemon/ and tools/ subdirectories for executables.

### 2. Move research/ to docs/
Rationale: "research" sounds like WIP; "docs" is standard for user-facing docs.

### 3. Keep openspec/ for internal design
Rationale: Useful for planning but not user-facing.

### 4. GPL-2.0 license
Rationale: Compatible with kernel driver (if upstreamed).

## Files to Create

| File | Purpose |
|------|---------|
| README.md | Project overview, quick start |
| CONTRIBUTING.md | Contribution guidelines |
| LICENSE | GPL-2.0 text |
| docs/building.md | Detailed build instructions |
| docs/steam-input.md | Steam configuration |
| install/install.sh | Installation script |
| install/uninstall.sh | Uninstall script |
| .github/workflows/ci.yml | CI build |
| .github/ISSUE_TEMPLATE/*.md | Issue templates |

## Files to Move

| From | To |
|------|-----|
| 99-vader5.rules | install/99-vader5.rules |
| research/protocol-24g.md | docs/protocol.md |
| driver/install.sh | install/install-driver.sh |

## Files to Delete

| File | Reason |
|------|--------|
| AGENTS.md | Merge into .claude/ |
| research/*.md (except protocol) | Outdated or merged |
| scripts/ | Unused |
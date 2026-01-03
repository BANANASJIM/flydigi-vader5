# Add TUI

## Summary
Add terminal UI for real-time input visualization and key mapping configuration.

## Dependencies
- termbox2 (header-only)

## Features
- Real-time gamepad state display
- Interactive key mapping editor
- Save/load config
- Visual button/axis feedback

## Layout
```
┌─────────────────────────────────────────┐
│  Vader 5 Pro - TUI                      │
├─────────────────────────────────────────┤
│  [LT: 000]          [RT: 000]           │
│                                          │
│     [LB]              [RB]              │
│  ┌───────┐         ┌───────┐            │
│  │ L     │  [Y]    │     R │            │
│  │ Stick │[X] [B]  │ Stick │            │
│  └───────┘  [A]    └───────┘            │
│                                          │
│   [↑]     [SEL][MODE][START]            │
│  [←][→]                                  │
│   [↓]      M1  M2  M3  M4               │
│            C   Z   ○   ⌂                │
├─────────────────────────────────────────┤
│  Press TAB to edit mappings             │
└─────────────────────────────────────────┘
```

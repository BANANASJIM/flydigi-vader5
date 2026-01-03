# Change: Refactor Project Structure for Open Source

## Why
Current structure is disorganized for an open source project:
- No README, LICENSE, CONTRIBUTING
- Docs scattered in research/
- udev rules in root
- No clear separation between daemon and tools

Need GitHub-ready structure for community contributions.

## What Changes
- Add README.md, CONTRIBUTING.md, LICENSE (GPL-2.0)
- Create docs/ for user documentation
- Create install/ for udev rules and install scripts
- Update .gitignore for build artifacts
- Clean up driver/ build artifacts

## Impact
- Affected code: File locations only, no code changes
- Affected docs: research/ → docs/
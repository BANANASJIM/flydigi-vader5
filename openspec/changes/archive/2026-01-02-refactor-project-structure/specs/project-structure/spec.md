# Project Structure

## ADDED Requirements

### Requirement: Standard open source project layout

The project MUST follow standard open source conventions with:
- README.md at root with project overview
- CONTRIBUTING.md with contribution guidelines
- LICENSE file with GPL-2.0
- docs/ for user documentation
- install/ for installation scripts and udev rules
- src/daemon/ for main executable
- src/tools/ for utility executables
- .github/ for CI and issue templates

#### Scenario: New contributor clones repository
- Given a new contributor clones the repository
- When they look at the root directory
- Then they see README.md, CONTRIBUTING.md, LICENSE
- And docs/ contains user documentation
- And install/ contains installation files
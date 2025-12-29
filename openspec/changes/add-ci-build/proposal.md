# Add CI/Build Scripts

## Summary
Add build scripts and GitHub Actions CI workflow.

## Changes
- `scripts/build.sh`: Local build script
- `.github/workflows/ci.yml`: CI workflow with build matrix and format check

## Build Matrix
- OS: Ubuntu 24.04
- Compilers: GCC, Clang
- Build types: Debug, Release

## CI Jobs
1. **build**: Compile with clang-tidy
2. **format**: Check code formatting

# Contributing to Flydigi Vader 5 Linux Driver

Thank you for your interest in contributing!

## Development Setup

```bash
# Build
cd flydigi-vader5
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Install udev rules
sudo cp install/99-vader5.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
```

## Code Style

- C++23 with modern idioms
- Use `clang-format` (config in `.clang-format`)
- Use `clang-tidy` for static analysis
- Headers: declarations only, minimal comments
- Prefer `std::expected` for error handling

```bash
# Format code
clang-format -i src/*.cpp include/vader5/*.hpp

# Run clang-tidy
cmake --build build --target vader5d 2>&1 | grep warning
```

## Project Structure

| Directory | Purpose |
|-----------|---------|
| `src/daemon/` | Main driver daemon |
| `src/tools/` | Debug and utility tools |
| `include/vader5/` | Public API headers |
| `docs/` | Documentation |

## Making Changes

### 1. Check existing work

```bash
# View active proposals
openspec list

# Search for related code
rg "keyword" src/
```

### 2. For new features

Create a proposal first:

```bash
mkdir -p openspec/changes/add-feature-name/specs/capability
# Edit proposal.md, tasks.md, spec.md
openspec validate add-feature-name --strict
```

### 3. For bug fixes

- Create an issue first (if none exists)
- Reference the issue in your commit

### 4. Commit messages

```
type(scope): short description

- Detail 1
- Detail 2

Fixes #123
```

Types: `feat`, `fix`, `docs`, `refactor`, `test`, `chore`

## Pull Request Process

1. Fork the repository
2. Create a feature branch (`git checkout -b feat/my-feature`)
3. Make your changes
4. Run tests and linting
5. Push to your fork
6. Open a Pull Request

### PR Checklist

- [ ] Code compiles without warnings (`-Werror`)
- [ ] clang-format applied
- [ ] clang-tidy passes
- [ ] Documentation updated (if needed)
- [ ] Tested on actual hardware

## Testing

```bash
# Build and run debug tool
cmake --build build --target vader5-debug
sudo ./build/vader5-debug

# Test with evtest
sudo evtest /dev/input/eventX
```

## Protocol Research

If you're working on protocol reverse engineering:

1. Use Wireshark with USBPcap (Windows) or usbmon (Linux)
2. Document findings in `docs/protocol.md`
3. Add test cases for new packet types

## Questions?

- Open an issue for bugs or feature requests
- Check existing issues before creating new ones

## License

By contributing, you agree that your contributions will be licensed under GPL-2.0.
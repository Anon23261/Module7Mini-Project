# Contributing to Systems & Security Portfolio

## Getting Started

### Prerequisites
1. C++17 compatible compiler (GCC 7+ or Clang 5+)
2. CMake 3.15 or higher
3. Python 3.8 or higher
4. Rust toolchain (latest stable)
5. Required system libraries (see README.md)

### Setting Up Development Environment

1. Clone the repository:
```bash
git clone https://github.com/yourusername/systems-portfolio.git
cd systems-portfolio
```

2. Install dependencies:
```bash
# C++ dependencies
sudo apt-get update
sudo apt-get install -y build-essential cmake libboost-all-dev libssl-dev libcurl4-openssl-dev libsqlite3-dev

# Python dependencies
pip install -r requirements.txt

# Rust toolchain
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

3. Build the project:
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Project Structure

- `projects/cpp_learning_lab/`: C++ educational content and exercises
- `projects/ghost_os_cpp/`: OS development toolkit in C++
- `projects/ghost_os_toolkit/`: OS development toolkit in Rust
- `projects/rust_security_tools/`: Security tools implemented in Rust
- `assets/`: Project resources and images
- `docs/`: Documentation
- `tests/`: Test suites

## Coding Standards

### C++
- Follow C++17 standards
- Use smart pointers for memory management
- Implement RAII principles
- Document using Doxygen-style comments
- Use const correctness
- Follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)

### Python
- Follow PEP 8 style guide
- Use type hints
- Document using Google-style docstrings
- Use virtual environments

### Rust
- Follow official Rust style guidelines
- Use cargo fmt for formatting
- Document using rustdoc
- Follow idiomatic Rust practices

## Testing

### C++ Projects
```bash
cd build
ctest --output-on-failure
```

### Python Projects
```bash
python -m pytest tests/
```

### Rust Projects
```bash
cargo test
```

## Documentation

- Use Doxygen for C++ documentation
- Use Sphinx for Python documentation
- Use rustdoc for Rust documentation
- Keep README.md files up to date
- Document significant changes in CHANGELOG.md

## Pull Request Process

1. Create a feature branch from `develop`
2. Write tests for new functionality
3. Update documentation
4. Ensure all tests pass
5. Create a pull request with a clear description
6. Request review from maintainers

## Commit Guidelines

- Use clear, descriptive commit messages
- Reference issue numbers when applicable
- Keep commits focused and atomic
- Follow conventional commits format:
  - feat: new feature
  - fix: bug fix
  - docs: documentation changes
  - style: formatting changes
  - refactor: code restructuring
  - test: adding tests
  - chore: maintenance tasks

## Code Review Process

1. Automated checks must pass
2. At least one maintainer approval required
3. Documentation must be complete
4. Tests must cover new functionality
5. Code must follow project standards

## Release Process

1. Update version numbers
2. Update CHANGELOG.md
3. Create release branch
4. Run full test suite
5. Create GitHub release
6. Tag the release

## Questions or Problems?

- Open an issue for bugs
- Use discussions for questions
- Join our Discord community
- Check existing documentation first

Thank you for contributing!

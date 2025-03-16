# Contributing to ShotCap

Thank you for your interest in contributing to ShotCap—a feature-rich command-line screenshot tool for Windows. We welcome your ideas, bug fixes, improvements, and new features. This document outlines our contribution guidelines and the process to follow when submitting your changes.

---

## Table of Contents

- [How to Contribute](#how-to-contribute)
- [Reporting Issues](#reporting-issues)
- [Setting Up Your Development Environment](#setting-up-your-development-environment)
- [Coding Guidelines](#coding-guidelines)
- [Commit Messages](#commit-messages)
- [Pull Request Process](#pull-request-process)
- [Development Roadmap](#development-roadmap)
- [Questions](#questions)

---

## How to Contribute

Contributions can take many forms:
- **Bug Fixes:** If you find a bug, please report it or submit a pull request with a fix.
- **New Features:** We welcome ideas for new features. Open an issue to discuss your idea before implementation.
- **Documentation:** Improvements to the README, CONTRIBUTING guidelines, or other documentation are very welcome.
- **Testing:** Adding unit tests or improving the test suite is a great way to contribute.

---

## Reporting Issues

If you encounter a bug or have a feature request, please follow these steps:
1. **Search the Issue Tracker:** Ensure that your issue hasn't already been reported.
2. **Open a New Issue:** Provide a clear title and a detailed description, including:
   - The operating system and Windows version you’re using.
   - Steps to reproduce the issue.
   - Expected and actual behavior.
   - Any error messages or logs.
3. **Attach Screenshots or Logs:** If applicable, include screenshots or log files to help us understand the issue.

---

## Setting Up Your Development Environment

Follow these steps to set up a development environment for ShotCap:

1. **Clone the Repository:**

   ```bash
   git clone https://github.com/NSTechBytes/ShotCap.git
   cd ShotCap
   ```

2. **Open the Project in Visual Studio:**
   - Open the solution file `ShotCap.sln` using Visual Studio 2019 or later.

3. **Configure Project Settings:**
   - Ensure that the **Character Set** is set to **Use Unicode Character Set**.
   - Add `gdiplus.lib` and `Shcore.lib` to the project's **Additional Dependencies** (under Linker → Input).
   - Define `_WIN32_WINNT` to at least `0x0A00` (Windows 10) in your project settings to enable modern API features like `SetProcessDpiAwarenessContext`.

4. **Build the Project:**
   - Use **Build Solution** (Ctrl+Shift+B) to compile the project.

5. **Run the Application:**
   - Launch the compiled executable with command-line arguments as needed. For example:
     
     ```bash
     ShotCap.exe -v -select -timestamp
     ```

---

## Coding Guidelines

- **Consistency:** Follow the coding style used throughout the project. Consistent indentation, naming conventions, and brace styles are important.
- **Unicode:** Use wide strings (e.g., `L"string"`) for Windows API calls.
- **Secure Functions:** Prefer secure functions (e.g., `localtime_s` instead of `localtime`).
- **Comments:** Document non-trivial code sections with comments to explain your logic.
- **Error Handling:** Ensure your changes handle errors gracefully and log helpful messages when the `-v` (verbose) flag is active.

---

## Commit Messages

Please follow these guidelines for commit messages:
- **Short Summary:** Write a concise summary in the first line (ideally 50 characters or less).
- **Detailed Description:** Include a more detailed explanation in the body if necessary.
- **Imperative Mood:** Use commands (e.g., "Fix bug" rather than "Fixed bug").
- **Reference Issues:** If applicable, reference any related issues (e.g., `Fixes #123`).

---

## Pull Request Process

1. **Fork the Repository:** Create a fork on GitHub.
2. **Create a Feature Branch:** Use descriptive names (e.g., `feature/mouse-selection`).
3. **Make Changes:** Implement your changes along with tests if applicable.
4. **Commit and Push:** Commit your changes with clear messages and push them to your fork.
5. **Open a Pull Request:** Submit a pull request against the main repository’s default branch.
6. **Review Process:** Respond to feedback and make any requested changes.

---

## Development Roadmap

Some areas we’re currently focusing on:
- Improving window capture for applications using hardware acceleration.
- Expanding testing coverage.
- Enhancing error logging and diagnostics.
- Improving interactive selection and other UI elements.

Check the [Issues](https://github.com/NSTechBytes/ShotCap/issues) section for more ideas or to volunteer for a feature.

---

## Questions

If you have questions or need clarification, please:
- Open a new issue on GitHub.
- Reach out to the maintainers directly via GitHub Discussions or email.

Thank you for contributing to ShotCap! Your input helps make this project better for everyone.

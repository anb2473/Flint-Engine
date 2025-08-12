# 🤝 Contributing Guidelines

Thank you for your interest in contributing to Flint Engine! 🎉 Whether you're fixing bugs, adding features, or improving documentation, your help is appreciated! 🙌

Before contributing, read the `FILE_GRAPH.md` documentation for a quick outline of the structure of the engine.

## 📋 How to Contribute

1. Fork & Clone 🍴

    - Fork the repo
    - Clone it locally with the command `git clone https://github.com/yourusername/Flint-Engine.git` (enter your actual username)

2. Create a Branch 🌿

    - Create a new branch for your work:
    - git checkout -b feature/your-feature-name

3. Make Changes 🛠️

    - Follow the existing code style and structure.
    - Write clear, concise commit messages.
    - For C code, keep functions modular and clean.
    - For PowerShell scripts, add comments
    - **NEVER USE DEPRECICATED RESOURCES**
    - Also, please read out `STANDARDS.md` file for more details on code standards

4. Test Your Changes 🧪

    - Use the provided testing scripts to validate your changes.
    - Add new tests as needed.

5. Submit a Pull Request 📬

    - Push your branch to your fork with the command `git push origin feature/your-feature-name`.
    - Open a pull request and provide a clear description of your changes.

## 🛑 Important Notes

This project currently supports testing only on Windows. Please test accordingly. 🪟 Keep all test scripts in the `testing/scripts` directory. Document any new test cases in `TESTING.md`. If you want feel free to create new testing scripts for bash and zsh.

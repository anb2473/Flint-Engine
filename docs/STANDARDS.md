# Flint Engine Code Standards 🛠️📏

## 1. Variable Naming 🏷️  

- Use clear, descriptive, and meaningful variable names.  
- Avoid confusing abbreviations.  
- Be consistent for better readability.

## 2. Commenting 📝  

- Comment your code to explain purpose, logic, and important decisions.  
- Keep comments concise and relevant.  
- Document inputs, outputs, and side effects of functions.

## 3. Functions 🔧  

- Keep functions small and focused on a single task.  
- Avoid large, complex functions.  
- Modular functions improve maintainability and testing.

## 4. Headers and Source Files 📂  

- Only expose necessary functions in headers (.h).  
- Place headers in the **include/** directory.  
- Place C source files (.c) in the **src/** directory.

## 5. Directory Structure for Types 📁  

- Put all **structs** in **include/structs/**.  
- Put all **ICDs** (UT_array descriptors) in **include/icds/**.  
- Put all **enums (states)** in **include/states/**.

## 6. Reusable Utilities 🔄  

- Extract I/O functions into **include/io-utils/**.  
- Extract string functions into **include/str-utils/**.  
- Promotes code reuse and cleaner organization.

## 7. Testing 🧪  

- Write all test scripts in **testing/scripts/**.  
- Store compiled test binaries in **testing/binaries/**.  
- Regularly clean binaries folder using cleanup scripts in **scripts/** to keep workspace tidy 🧹.

## 8. Branching and Collaboration 🌿  

- Always create a new branch for your changes.  
- Merge changes only through pull requests 📬.  
- Follow commit message and review guidelines.

## 9. Directory Structure and Refactoring 🔄  

- Avoid renaming/moving directories unless necessary.  
- If you do:  
  - Update all include paths project-wide.  
  - Update testing scripts to look for the new directory name if different from **Flint-Engine** (⚠️ renaming the main directory is discouraged).  

## 10. Cross-Platform Compatibility 🌐  

- Write code that runs on all supported OSes where possible.  
- Isolate and document platform-specific code.

## 11. Documentation 📚  

- Update documentation before committing.  
- This includes README, TESTING.md, and design documents.

---

Following these standards ensures code quality, readability, and smooth collaboration! 🚀

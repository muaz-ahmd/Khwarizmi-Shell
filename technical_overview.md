# Khwarizmi-Shell: Technical Overview

Welcome to the technical heart of **Khwarizmi-Shell**. This document explains how the shell works, broken down into simple concepts for beginners.

---

## 🏗️ What is a Shell?

Think of a **Shell** as a translator. Computer hardware only understands 1s and 0s. You, the human, understand text commands. The shell sits between you and the operating system (the "Kernel"), taking your typed commands and telling the computer exactly what to perform.

## 🔄 The Lifecycle: Read-Parse-Execute

Khwarizmi-Shell operates in a continuous loop, often called a **REPL** (Read-Eval-Print Loop):

1.  **Read**: It waits for you to type something and press Enter.
2.  **Parse**: It breaks your typed string into understandable chunks (commands and arguments).
3.  **Execute**: It runs the command you asked for.
4.  **Repeat**: It shows the prompt (`$`) again and waits for the next command.

---

## 🧩 Core Components

### 1. Input Handling (The Ears)
Located primarily in `input.c`, this part of the shell handles "Raw Mode." 
- **Standard Mode**: Your terminal usually waits for you to press Enter before sending text to a program.
- **Raw Mode**: Our shell listens to every single keystroke. This allows features like:
    - **Tab Completion**: When you hit Tab, the shell looks at what you've typed and searches your computer for matching commands.
    - **History Navigation**: When you hit the Up arrow, the shell swaps your current line with the previous command you typed.

### 2. The Parser (The Brain)
Located in `parser.c`, the parser's job is to make sense of the mess. If you type:
`echo "Hello World" > output.txt`
The parser identifies that:
- `echo` is the command.
- `"Hello World"` is the message (and the quotes should be removed).
- `>` means "send the output to a file."
- `output.txt` is the destination file.

### 3. Execution (The Hands)
Located in `execution.c` and `main.c`, this is where the work happens.
- **Built-ins**: Some commands like `cd` or `exit` are handled directly by the shell program itself.
- **External Commands**: For commands like `ls` or `mkdir`, the shell uses a process called `fork`:
    - **Forking**: The shell makes a "copy" of itself.
    - **Exec**: The copy transforms into the program you want to run.
    - **Wait**: The original shell waits until the copy is finished before asking for your next command.

### 4. Pipes and Redicrection (The Plumbing)
- **Redirection**: Instead of printing to your screen, the shell can "hijack" the output and send it to a file.
- **Pipes (`|`)**: This is like connecting two garden hoses. The output of the first command becomes the input for the second. 
    - *Example*: `ls | grep .c` — `ls` lists files, and `grep` filters them. The shell manages the "pipe" between them so they can talk to each other.

---

## 📁 File Structure

- `main.c`: The entry point and the main command loop.
- `shell.h`: The "dictionary" where all functions and data structures are defined.
- `parser.c`: Handles strings, quotes, and identifies redirection.
- `execution.c`: Decides if a command is a built-in or external, and manages processes.
- `builtins.c`: Logic for internal shell commands.
- `history.c`: Manages the list of past commands and file persistence.
- `input.c`: Manages terminal settings and line editing.

---

## 🌟 Summary for Beginners
Building a shell is like building a miniature operating system interface. It involves managing memory, handling text precisely, and coordinating different programs so they can work together. Khwarizmi-Shell is designed to show these complex concepts in a clean, readable C codebase.

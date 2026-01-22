# Khwarizmi-Shell

A lightweight, feature-rich Unix-like shell implemented in C. Named after the legendary Persian mathematician Muhammad ibn Musa al-Khwarizmi, this shell aims to provide a robust environment for command execution with modern terminal features.

## 🚀 Features

- **Built-in Commands**: Includes `cd`, `pwd`, `echo`, `type`, `history`, and `exit`.
- **Command Execution**: Run any system command available in your `PATH`.
- **Pipes**: Chain multiple commands using `|` (e.g., `ls | grep .c | wc -l`).
- **Input/Output Redirection**:
  - Redirect standard output: `command > file` (overwrite) or `command >> file` (append).
  - Redirect standard error: `command 2> file` or `command 2>> file`.
- **History Management**:
  - Persistent history saved to a file (defined by `HISTFILE` environment variable).
  - Navigation using Up and Down arrow keys.
  - Built-in `history` command to view or manage (read/write/append) history entries.
- **Tab Completion**: Intelligent completion for built-ins and system commands.
- **Quoting Support**: Handles single (`'`) and double (`"`) quotes for arguments with spaces or special characters.
- **Script Support**: Execute shell scripts by passing the file as an argument.
- **Raw Mode**: Custom terminal handling for a smooth interactive experience.

## 🛠️ Compilation and Installation

To compile Khwarizmi-Shell, you need a C compiler (like GCC) and a Unix-like environment (or WSL on Windows).

1. Clone or download the repository.
2. Open your terminal in the project directory.
3. Compile the source files:

```bash
gcc -o k-shell main.c parser.c execution.c builtins.c history.c input.c
```

4. Run the shell:

```bash
./k-shell
```

## 📖 Usage Examples

### Basic Commands
```bash
$ pwd
/home/user/Khwarizmi-Shell
$ echo "Hello from Khwarizmi-Shell"
Hello from Khwarizmi-Shell
```

### Piping and Redirection
```bash
# Filter file list and save to a file
$ ls -l | grep .c > source_files.txt

# Append errors to a log file
$ non-existent-command 2>> error.log
```

### History Management
```bash
# View last 10 commands
$ history 10

# Save current session history to a custom file
$ history -w custom_history.txt
```

## 📝 License

This project is open-source. Feel free to use and modify it.
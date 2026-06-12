# Custom Shell in C

## Overview

This project implements a custom Unix-like shell in C. The shell accepts user commands, parses input, creates child processes to execute commands, and supports several built-in features commonly found in standard shells.

---

## Features

### Command Execution

* Continuously prompts the user for input.
* Parses user commands into arguments.
* Creates a child process using `fork()`.
* Executes commands using `execvp()`.
* The parent process waits for child execution to complete using `wait()` to prevent zombie processes.

### Built-in Commands

#### `cd`

* The `cd` command is handled separately from other commands.
* Uses `chdir()` to change the shell's current working directory.
* Since directory changes must persist within the shell process, `cd` is executed directly by the parent rather than a child process.

#### `exit`

* Terminates the shell gracefully.

### Pipe Support

* Supports single pipe operations such as:

```bash
ls | grep ".c"
```

* Pipe handling is implemented separately from normal command execution.
* Uses:

  * `pipe()` to create communication channels.
  * `fork()` to create child processes.
  * `dup2()` to redirect standard input and output.
  * `execvp()` to execute both commands involved in the pipe.
* The parent process waits for both child processes to finish execution.

### Signal Handling (`Ctrl + C`)

* The shell overrides the default `SIGINT` behavior.
* Pressing **Ctrl + C**:

  * Does **not** terminate the shell itself.
  * Terminates only the currently running foreground process.
* Custom signal handling is also used to maintain prompt formatting after interruptions.

### User Interface Enhancements

* Displays a customized shell prompt containing:

  * Username (`zaid`)
  * Shell identifier (`custom-shell`)
  * Current working directory
* ANSI escape sequences are used to provide colored output for improved user experience.

Example prompt:

```bash
zaid@custom-shell:/home/zaid/projects>
```

---

## Technologies Used

* C Programming Language
* POSIX System Calls
* Linux Process Management APIs

Key system calls and functions used include:

* `fork()`
* `execvp()`
* `wait()`
* `waitpid()`
* `pipe()`
* `dup2()`
* `signal()`
* `chdir()`
* `getcwd()`

---

## Compilation

Compile using GCC:

```bash
gcc shell.c -o shell
```

---

## Running the Shell

Execute the compiled program:

```bash
./shell
```

---

## Example Usage

```bash
zaid@custom-shell:~> ls
Documents  Downloads  shell.c

zaid@custom-shell:~> cd Documents

zaid@custom-shell:~/Documents> ls | grep ".txt"
notes.txt
report.txt

zaid@custom-shell:~/Documents> sleep 100
^C

zaid@custom-shell:~/Documents> exit
Exiting...
```

---

## Limitations

* Supports only a single pipe (`|`) between two commands.
* Limited argument capacity per command.
* Does not currently support:

  * Background processes (`&`)
  * Input/output redirection (`<`, `>`)
  * Multiple chained pipes
  * Job control (`fg`, `bg`)

---

## Author

Zaid

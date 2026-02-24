# psh

A minimal shell written in C for Windows.

---

## Features

- Interactive prompt (`>`) with a read-eval-print loop
- Tokenizer that handles spaces, tabs, and newlines
- External program execution via the Win32 `CreateProcess` API
- Built-in commands (no child process spawned)

## Built-ins

| Command    | Description                                                                            |
| ---------- | -------------------------------------------------------------------------------------- |
| `cd [dir]` | Change directory. No argument prints the current path. `cd ~` goes to `%USERPROFILE%`. |
| `help`     | List all available built-in commands.                                                  |
| `exit`     | Exit the shell.                                                                        |

## Building

Compile with any C compiler that targets Windows and links against the Win32 API, for example:

```sh
gcc main.c -o psh.exe
```

## Usage

```sh
./psh.exe
> help
> cd C:\Users
> notepad
> exit
```

---

_Built as a learning project._

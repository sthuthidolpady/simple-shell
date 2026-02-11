# simple-shell
# Simple Shell Implementation (C with TUI)

## 📌 Project Overview

This project is a Simple Shell implementation developed in C on a Linux environment.  
The objective of the project is to understand how a Unix shell works internally, including process creation, command execution, piping, redirection, background execution, and signal handling.

In addition to core shell functionality, a Text User Interface (TUI) is implemented using the `ncurses` library to provide a structured and user-friendly terminal interface.

---

## Features Implemented

- Execution of external Linux commands
-  Built-in commands (`cd`, `exit`)
-  Piping (`|`) for command chaining
-  Output redirection (`>`)
-  Append redirection (`>>`)
-  Input redirection (`<`)
-  Background execution (`&`)
-  Zombie process handling using `SIGCHLD`
-  Text User Interface using `ncurses`

---

##  Technologies Used

- **Programming Language:** C
- **Operating System:** Linux (Debian VM)
- **Compiler:** GCC
- **Libraries Used:**
  - stdio.h
  - stdlib.h
  - unistd.h
  - sys/wait.h
  - fcntl.h
  - signal.h
  - ncurses.h

---



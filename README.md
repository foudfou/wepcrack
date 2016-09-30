# Wepcrack

## Introduction

This started as a tool for *brute-forcing WEP keys* for a hacking challenge. I
wasn't aware of [Weplab](http://weplab.sourceforge.net/). Then I took it as an
exercice to improve my C/system programming skills.

## Features

* Brute-force keys, either by generating all possibilities or by reading
  passwords from a dictionary file.
* Supported protocols: WEP, XMPP-SCRAM-SHA1.
* Forks to use all available cores.
* Traps SIGUSR1 to display the current state.
* Traps SIGINT to save the current state to a file.
* Can read a state file to resume the computation.

## Design

For key generation, the computation is spread across forked processes which
communicate their state upon termination to the main process through a message
queue.

For dictionary parsing, the passwords are fed to forked processes in a
*Producer-Consumer* fashion, using semaphores and a message queue.

## Usage

* Build with `make` (`gmake` on BSD).
* Currently you need to edit the data in `src/wep_data.[ch]`.
* Edit the computation function `src/main.c:wep_check_key_with_data()` to use
  your data, or provide your own function.

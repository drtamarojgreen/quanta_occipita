# SDD Restrictions and Patterns

## Structural Restrictions
- **Language Standard:** C++17 only.
- **Dependencies:** Standard library only. No external libraries permitted unless explicitly approved.
- **Resource Management:** RAII must be used for all resource management (files, memory, etc.).

## Architectural Restrictions
- **Complexity Management:** Use the PIMPL idiom for complex logic to maintain clean interfaces and reduce compilation dependencies.
- **Interfaces:** Mandatory virtual destructors for all classes intended to be used as interfaces or base classes.
- **Design Patterns:** Design patterns (Singleton, Factory, Observer) must be optional and configurable via CLI flags.

## Tool Restrictions
- **Compiler:** `g++` (v13.3.0 confirmed).
- **Build System:** `GNU Make` (v4.3 confirmed).
- **Version Control:** `git`.

## Process Note
- The monolithic implementation of all 100 enhancements was performed in response to the direct user mandate ("Implement all the enhancements in docs/enhancements.md"). While this deviates from the "Sip Principle" of SDD, it ensures strict compliance with user authority (Section 2.1.1.1).

## Current Status
- All 100 enhancements from `docs/enhancements.md` implemented.
- Logic verified structurally and against logic flow requirements.
- Ready for final empirical execution (pending permission).

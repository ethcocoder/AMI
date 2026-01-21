# AmI System Structure

## High-Level Architecture

Python Interface
↓
C++ Reasoning Engine
↓
C Core Intelligence Engine

---

## Directory Structure

/ami
 ├── core_c/
 │   ├── memory.c
 │   ├── rule_engine.c
 │   ├── evaluator.c
 │   ├── knowledge_store.c
 │   └── core.h
 │
 ├── reasoning_cpp/
 │   ├── concept.cpp
 │   ├── algorithm.cpp
 │   ├── learner.cpp
 │   ├── state_machine.cpp
 │   └── reasoning.h
 │
 ├── interface_py/
 │   ├── cli.py
 │   ├── input_parser.py
 │   ├── visualizer.py
 │   └── bridge.py
 │
 ├── tests/
 │   ├── math_tests/
 │   ├── logic_tests/
 │   └── system_tests/
 │
 ├── docs/
 │   ├── DOCUMENTATION.md
 │   ├── ROADMAP.md
 │   ├── STRUCTURE.md
 │   └── TODO.md
 │
 └── main.cpp

---

## Data Flow

Input → Python → C++ Reasoning → C Core → Validation → Output

---

## Learning Loop

State Machine:
IDENTIFY → GATHER → OBSERVE → ANALYZE → SUMMARIZE → APPLY → REVIEW

---

## Security & Stability

- No self-modifying code
- No uncontrolled recursion
- No probabilistic guessing
- All rules must be validated

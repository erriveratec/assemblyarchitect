# Assembly Architect

**Assembly Architect** is an educational puzzle game designed to teach fundamental programming concepts through a visual and interactive interface inspired by x86 assembly language. The game challenges players to manipulate data using a real instruction set and register-based logic, fostering algorithmic thinking and a deeper understanding of computer architecture.

---

## 🧠 Concept Overview

Assembly architect simulates a assembly-like environment where players must solve increasingly complex tasks by writing code that manipulates data between an **Input Buffer (IB)** and an **Output Buffer (OB)** using virtual registers and real x86 instructions.

The game is inspired by pedagogical principles from programming education and cognitive load theory, aiming to:
- Reinforce understanding of memory operations.
- Develop procedural thinking.
- Introduce core concepts of instruction cycles and control flow.

---

## 🎮 Gameplay Mechanics

Players are presented with a visual programming interface consisting of:

- **Instruction Set**: Currently limited to basic operations such as `mov`, `add`, and `jmp`.
- **Registers**: Virtual registers (`rax`, `rbx`, `rcx`) used to store and manipulate data.
- **Buffers**: 
  - **Input Buffer [IB]**: Source of data values.
  - **Output Buffer [OB]**: Destination for processed values.
- **Code Box**: Where players drag and drop instructions to build their solution.
- **Execution Controls**: Step-through or run the program to test logic.

---

## 🖼️ Screenshots

### 🧩 Level Interface
![Level Interface](interface.png)
*Level 09: Move values from IB to OB using a maximum of 3 instructions.*

## 🧭 Interface Elements

Assembly Architect’s interface is composed of interactive components that simulate x86 assembly programming through spatial metaphors and animated agents. Each element is designed to reinforce architectural constraints and support intuitive learning.

| **Label** | **Element Name**         | **Description** |
|----------|---------------------------|-----------------|
| 1        | **Instruction Roster**    | Allows players to select and drag instructions into the Code Box. The available instructions vary by level to support incremental learning. |
| 2        | **Code Box**              | The workspace where players assemble their code. Instructions can be rearranged, edited, or removed. |
| 3        | **Challenge Description** | Provides a textual overview of the level’s objective, guiding players toward the intended solution. |
| 4        | **Register Box**          | Displays the available registers for use as operands. |
| 5        | **Registers Agent**       | Facilitates data exchange between registers and the Input/Output Buffer Agents. Movement is constrained by a yellow rail. |
| 6        | **Input Buffer Agent**    | Transfers values from the Input Buffer to the Registers Agent. Movement is constrained by a magenta rail. |
| 7        | **Output Buffer Agent**   | Moves values from the Registers Agent to the Output Buffer. Movement is constrained by a cyan rail. |
| 8        | **Input Buffer**          | Modeled as a memory address simulating a read-and-consume I/O buffer. Provides input values for processing. |
| 9        | **Output Buffer**         | A memory-mapped location where the player’s output is deposited and evaluated against expected results to determine challenge success. |

---

### 🧠 Code Execution
![Code Execution](execution.png)
*Player writes a loop using `mov` and `jmp` to transfer values from IB to OB.*

---

### ✅ Challenge Completed
![Challenge Completed](win.png)
*Successful execution of the program with correct output values.*

---

## 🔍 Example Solution

```asm
@1 LINE_01:
@2 mov rax,[ib]
@3 mov [ob],rax
@4 jmp LINE_01
```

This loop continuously reads from the Input Buffer into register `rax`, writes the value to the Output Buffer, and jumps back to repeat the process.

---

## 📚 Educational Objectives

Assembly architect is designed to support the following learning outcomes:

- Understand the role of registers and memory in computation.
- Practice control flow using jumps and loops.
- Develop debugging and problem-solving skills.
- Experience a simplified model of instruction execution.

---

## 🚧 Development Status

Assembly architect is currently under active development. The existing version includes a foundational set of x86-inspired instructions and nine tutorial levels designed to introduce core concepts such as register manipulation, memory access, and control flow.

Planned future enhancements include:
- Support for additional x86 instructions (e.g., stack operations, arithmetic, and logic instructions).
- New levels introducing advanced topics such as recursion and memory operands.
- Step-by-step execution mode for debugging and learning.
- In-game contextual help and instruction documentation.
- Instruction duplication and improved code editing features.

These features aim to expand the game’s fidelity to real-world x86 assembly language and enhance its pedagogical value.

---


## 🛠️ Installation

To run the game locally:

```bash
git clone https://github.com/yourusername/assembly-architect.git
cd assembly-architect
make
./assembly
```

> ⚠️ Currently the game only works in MacOS with the SDL2 Framework installed.

---

## 🤝 Contributing

We welcome contributions! Please open an issue or submit a pull request with improvements, bug fixes, or new levels.

---

## 📜 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---


# SandalBotV2

**SandalBotV2** is a high-performance chess engine written in modern C++.  
It implements the Universal Chess Interface (UCI) protocol, allowing it to communicate seamlessly with popular chess GUIs and tools. SandalBotV2 integrates several advanced search and evaluation techniques designed to deliver strong, efficient gameplay while maintaining extensibility for further improvements.

At its core, SandalBotV2 features an iterative deepening negamax search enhanced with alpha-beta pruning, quiescence search to avoid horizon effects, sophisticated move ordering, and transposition tables for efficient state reuse. The engine uses bitboards and magic bitboards for fast move generation and position evaluation. Additionally, it incorporates heuristic evaluation features like piece-square tables, pawn structure evaluation (including pawn islands and passed pawns), open file and diagonal control, and king safety via pawn shield analysis.

This combination allows SandalBotV2 to effectively balance search depth and evaluation accuracy, making it a competitive and insightful chess engine.

---

## Setup

### 1. (Optional) Clone and Build vcpkg

To build and run **unit tests** and **benchmarking**, the project depends on Google Test (GTest) and Google Benchmark libraries, which are managed via [vcpkg](https://github.com/microsoft/vcpkg).

To automate downloading and bootstrapping vcpkg, run the provided script:

#### Linux

```bash
chmod +x scripts/setup_vcpkg.sh
./scripts/setup_vcpkg.sh
```

#### Windows (PowerShell)

```powershell
.\scripts\setup_vcpkg.ps1
```

*Note:*  
- Make sure you have Git, CMake, and a compatible compiler installed before running these scripts.

---

### 2. Generate Build Files

Use CMake to configure the project and generate platform-appropriate build files.

#### Linux

```bash
cmake -S . -B build -DBUILD_TESTING=OFF -DBUILD_BENCHMARKS=OFF -DCMAKE_BUILD_TYPE=Release
```

- Replace 'Release' with 'Debug' as needed.

#### Windows (Visual Studio 2022 example)

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -DBUILD_TESTING=OFF -DBUILD_BENCHMARKS=OFF
```

- Replace `Visual Studio 17 2022` with another Visual Studio version if necessary.

---

*Note:*
- Replace `OFF` with `ON` to `BUILD_TESTING` or `BUILD_BENCHMARKS` as needed.
- Setting `ON` to either `BUILD_TESTING` or `BUILD_BENCHMARKS` will generate a static library for the src files.

---

### 3. Build

Build the project with:

#### Linux

```bash
cmake --build build
```

#### Windows

```powershell
cmake --build build --config Release
```

*You can replace `Release` with `Debug`.*

Or

- Open the generated `.sln` file located in `build/` with Visual Studio.  

---

## Run Executable

Run engine with command:

#### Linux

```bash
./build/src/SandalBotV2
```

#### Windows

```powershell
./build/src/Release/SandalBotV2.exe
```

*You can replace `Release` with `Debug` depending on build type.*

---

## Testing

Unit and integration tests are managed via CTest.

#### Linux and Windows

Navigate to the build directory:

```bash
cd build
```

List available tests without running them:

```bash
ctest -N
```

Run the full test suite:

```bash
ctest --output-on-failure
```

Run a specific subset or test by name pattern:

```bash
ctest -R <TEST_NAME>
```

---

## Benchmarking

*Benchmarking support is under development and will be documented here soon.*

---

## Features

- **Iterative Deepening**: Gradually increases search depth for anytime move results.  
- **Negamax Search**: Simplifies minimax logic using negamax formulation.  
- **Quiescence Search**: Extends search in tactical positions to avoid horizon effect.  
- **Move Ordering**: Heuristics to search promising moves first (captures, promotions, killer moves).  
- **Depth Reduction and Extension**: Adjusts search depth dynamically for efficiency and accuracy.  
- **Transposition Table**: Caches evaluated positions to avoid redundant calculations.  
- **Move Pre-Computation**: Precalculates moves for efficiency.  
- **Bitboards and Magic Bitboards**: Efficient board representation and move generation.  
- **Piece-Square Tables**: Position-dependent piece valuation.  
- **Open File / Diagonal Evaluation**: Considers control over open lines.  
- **Pawn Shield Evaluation**: Evaluates king safety based on pawn structure.  
- **Passed-Pawn and Pawn Island Evaluation**: Analyzes pawn structure weaknesses and strengths.

---

## Lichess Account

Watch SandalBotV2 in action on Lichess:  
[https://lichess.org/@/SandalBot](https://lichess.org/@/SandalBot)

---

*Licensed under the MIT License.*

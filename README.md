# Disaster Relief Management System

A console-based application written in **C** for managing disaster relief operations. It keeps track of victims, shelters, relief resources, volunteers, and the distribution of supplies, with all data stored in binary files so records persist between runs.

---

## Features

- **Victim Management**: add, display, search, update, and delete victim records
- **Priority Sorting**: sort victims by injury level so the most critical cases come first
- **Automatic Shelter Assignment**: new victims are placed in a shelter in the same location if one has space, otherwise in any available shelter, otherwise marked `NONE`
- **Shelter Management**: track capacity, current occupancy, and availability status
- **Resource Management**: manage relief supplies such as food, water, and medicine
- **Volunteer Management**: register volunteers with roles such as medical, food, or transport
- **Distribution Management**: assign resources to victims and view distribution records
- **System Summary**: overview of the whole relief operation
- **Input Validation**: checks for duplicate IDs, invalid age/injury/capacity values, and non-numeric menu input

---

## Project Structure

| File | Description |
|---|---|
| `DisasterReliefManagement.c` | Main source code of the system |
| `main.c` | Additional source file |
| `victim.dat` | Stores victim records |
| `shelter.dat` | Stores shelter records |
| `resources.dat` | Stores resource records |
| `volunteers.dat` | Stores volunteer records |
| `distribution.dat` | Stores resource distribution records |
| `.vscode/` | VS Code configuration |

---

## Data Structures

| Structure | Fields |
|---|---|
| `Victim` | id, name, age, injury level, location, shelter ID |
| `Shelter` | id, name, location, capacity, current occupants |
| `Resource` | id, name, quantity, type |
| `Volunteer` | id, name, role |
| `Distribution` | victim ID, resource ID, quantity |

---

## Getting Started

### Prerequisites

- A C compiler such as **GCC** (MinGW on Windows)

### Compile

```bash
gcc DisasterReliefManagement.c -o DisasterReliefManagement
```

### Run

```bash
# Windows
DisasterReliefManagement.exe

# Linux / macOS
./DisasterReliefManagement
```

---

## Usage

On launch, the main menu appears:

```
===== DISASTER RELIEF MANAGEMENT SYSTEM =====
1. Victim Management
2. Shelter Management
3. Resource Management
4. Volunteer Management
5. Distribution Management
6. System Summary
7. Exit
```

Each module has its own submenu with add, display, search, update, and delete options, plus **Back** and **Exit Program**.

**Tip:** Add shelters first. Victims are auto-assigned to shelters when they are registered.

---

## How Shelter Assignment Works

1. Look for a shelter in the **same location** as the victim that still has space.
2. If none is found, pick **any shelter** with available space.
3. If all shelters are full, the victim's shelter is set to `NONE`.

Shelter occupancy is automatically adjusted when a victim is added, moved (location update), or deleted. A shelter cannot be deleted while it has occupants, and its capacity cannot be reduced below its current occupancy.

---

## Concepts Used

- Structures and file handling in C (binary files with `fread`, `fwrite`, `fseek`)
- Dynamic memory allocation (`realloc`) for sorting
- Bubble sort for priority ordering
- Modular programming with menu-driven functions
- Input validation and buffer handling

---

## Future Improvements

- Search and filter by location or injury level
- Stock tracking that updates automatically on distribution
- Report export (CSV or text)
- Login system for admin and volunteers
- GUI or web interface

---

## Author

**Gl1tchRaze**

---

## License

This project is for educational purposes.

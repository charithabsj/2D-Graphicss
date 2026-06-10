# 2D Text Graphics Editor

A terminal-based 2D graphics editor written in C. Draws shapes using `*` and `_` characters on a 40×80 character canvas.

## Compile

```bash
gcc -o main main.c -lm
```

## Run

```bash
./main
```

> On Windows: `main.exe` in cmd or `.\main.exe` in powershell

## Features

- Draw **circles**, **rectangles**, **lines**, and **triangles**
- **Add**, **delete**, and **modify** objects by ID
- Choose fill character: `*` or `_`
- Canvas re-renders automatically after every change

## Usage

The editor runs as an interactive menu. Coordinates use `row col` format (row 0 = top, col 0 = left).

| Option | Action |
|--------|--------|
| 1 | Add Circle — enter centre row, col, and radius |
| 2 | Add Rectangle — enter top-left and bottom-right corners |
| 3 | Add Line — enter start and end points |
| 4 | Add Triangle — enter three vertices |
| 5 | Delete Object — enter object ID |
| 6 | Modify Object — change fill or reposition by ID |
| 7 | List Objects — show all active shapes and their IDs |
| 8 | Display Canvas — redraw the current picture |
| 0 | Exit |

## Example

```
Centre row col radius: 10 40 8
Fill character ('*' or '_'): *
```

This draws a circle centred at row 10, column 40, with radius 8.

## Notes

- Canvas size: 40 rows × 80 columns
- Up to 100 objects can exist at once
- Deleting an object removes it from the canvas but preserves IDs of remaining objects
- Requires a C compiler with math library support (`-lm`)

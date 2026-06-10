#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ROWS 40
#define COLS 80
#define MAX_OBJECTS 100

// ─── Canvas ───────────────────────────────────────────────────────────────────
char canvas[ROWS][COLS];

void init_canvas() {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            canvas[r][c] = ' ';
}

void display_canvas() {
    // Top border
    printf("+");
    for (int c = 0; c < COLS; c++) printf("-");
    printf("+\n");

    for (int r = 0; r < ROWS; r++) {
        printf("|");
        for (int c = 0; c < COLS; c++)
            putchar(canvas[r][c]);
        printf("|\n");
    }

    // Bottom border
    printf("+");
    for (int c = 0; c < COLS; c++) printf("-");
    printf("+\n");
}

// ─── Object list ──────────────────────────────────────────────────────────────
typedef enum { SHAPE_CIRCLE, SHAPE_RECTANGLE, SHAPE_LINE, SHAPE_TRIANGLE } ShapeType;

typedef struct {
    int      id;
    ShapeType type;
    int      active;   // 1 = exists, 0 = deleted
    // Parameters (meaning depends on type)
    int x1, y1, x2, y2, x3, y3;  // corners / endpoints / vertices
    int radius;
    char fill;         // '*' or '_'
} Object;

Object objects[MAX_OBJECTS];
int    object_count = 0;
int    next_id      = 1;

// ─── Helpers ──────────────────────────────────────────────────────────────────
void set_pixel(int r, int c, char ch) {
    if (r >= 0 && r < ROWS && c >= 0 && c < COLS)
        canvas[r][c] = ch;
}

/* Bresenham line */
void draw_line_pixels(int r0, int c0, int r1, int c1, char ch) {
    int dr = abs(r1 - r0), dc = abs(c1 - c0);
    int sr = (r0 < r1) ? 1 : -1;
    int sc = (c0 < c1) ? 1 : -1;
    int err = dr - dc;

    while (1) {
        set_pixel(r0, c0, ch);
        if (r0 == r1 && c0 == c1) break;
        int e2 = 2 * err;
        if (e2 > -dc) { err -= dc; r0 += sr; }
        if (e2 <  dr) { err += dr; c0 += sc; }
    }
}

// ─── Draw functions ───────────────────────────────────────────────────────────

/* Circle: centre (cy, cx), radius r */
void draw_circle(int cy, int cx, int radius, char ch) {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            /* Scale columns by 0.5 to compensate for character aspect ratio */
            double dr = r - cy;
            double dc = (c - cx) * 0.5;
            double dist = sqrt(dr * dr + dc * dc);
            if (ch == '*') {
                if (dist <= radius + 0.7)
                    set_pixel(r, c, ch);
            } else {
                if (dist >= radius - 0.7 && dist <= radius + 0.7)
                    set_pixel(r, c, ch);
            }
        }
    }
}

/* Rectangle: top-left (r1,c1), bottom-right (r2,c2) */
void draw_rectangle(int r1, int c1, int r2, int c2, char ch) {
    if (r1 > r2) { int t = r1; r1 = r2; r2 = t; }
    if (c1 > c2) { int t = c1; c1 = c2; c2 = t; }
    if (ch == '*') {
        for (int r = r1; r <= r2; r++)
            for (int c = c1; c <= c2; c++)
                set_pixel(r, c, ch);
    } else {
        draw_line_pixels(r1, c1, r1, c2, ch);   // top
        draw_line_pixels(r2, c1, r2, c2, ch);   // bottom
        draw_line_pixels(r1, c1, r2, c1, ch);   // left
        draw_line_pixels(r1, c2, r2, c2, ch);   // right
    }
}

/* Line: (r1,c1) -> (r2,c2) */
void draw_line(int r1, int c1, int r2, int c2, char ch) {
    draw_line_pixels(r1, c1, r2, c2, ch);
}

static int point_in_triangle(int r, int c, int r1, int c1, int r2, int c2, int r3, int c3) {
    int d1 = (c - c2) * (r1 - r2) - (c1 - c2) * (r - r2);
    int d2 = (c - c3) * (r2 - r3) - (c2 - c3) * (r - r3);
    int d3 = (c - c1) * (r3 - r1) - (c3 - c1) * (r - r1);
    int has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    int has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);
    return !(has_neg && has_pos);
}

/* Triangle: three vertices */
void draw_triangle(int r1, int c1, int r2, int c2, int r3, int c3, char ch) {
    if (ch == '*') {
        int min_r = r1 < r2 ? (r1 < r3 ? r1 : r3) : (r2 < r3 ? r2 : r3);
        int max_r = r1 > r2 ? (r1 > r3 ? r1 : r3) : (r2 > r3 ? r2 : r3);
        int min_c = c1 < c2 ? (c1 < c3 ? c1 : c3) : (c2 < c3 ? c2 : c3);
        int max_c = c1 > c2 ? (c1 > c3 ? c1 : c3) : (c2 > c3 ? c2 : c3);
        if (min_r < 0) min_r = 0;
        if (min_c < 0) min_c = 0;
        if (max_r >= ROWS) max_r = ROWS - 1;
        if (max_c >= COLS) max_c = COLS - 1;
        for (int r = min_r; r <= max_r; r++) {
            for (int c = min_c; c <= max_c; c++) {
                if (point_in_triangle(r, c, r1, c1, r2, c2, r3, c3))
                    set_pixel(r, c, ch);
            }
        }
    } else {
        draw_line_pixels(r1, c1, r2, c2, ch);
        draw_line_pixels(r2, c2, r3, c3, ch);
        draw_line_pixels(r3, c3, r1, c1, ch);
    }
}

// ─── Render all active objects ────────────────────────────────────────────────
void render_all() {
    init_canvas();
    for (int i = 0; i < object_count; i++) {
        if (!objects[i].active) continue;
        Object *o = &objects[i];
        switch (o->type) {
            case SHAPE_CIRCLE:
                draw_circle(o->y1, o->x1, o->radius, o->fill);
                break;
            case SHAPE_RECTANGLE:
                draw_rectangle(o->y1, o->x1, o->y2, o->x2, o->fill);
                break;
            case SHAPE_LINE:
                draw_line(o->y1, o->x1, o->y2, o->x2, o->fill);
                break;
            case SHAPE_TRIANGLE:
                draw_triangle(o->y1, o->x1, o->y2, o->x2, o->y3, o->x3, o->fill);
                break;
        }
    }
}

// ─── Add helpers ──────────────────────────────────────────────────────────────
int add_circle(int cy, int cx, int radius, char fill) {
    if (object_count >= MAX_OBJECTS) { printf("Object limit reached!\n"); return -1; }
    Object *o = &objects[object_count++];
    o->id = next_id++;  o->type = SHAPE_CIRCLE;  o->active = 1;
    o->y1 = cy; o->x1 = cx; o->radius = radius; o->fill = fill;
    printf("Added circle (id=%d)\n", o->id);
    return o->id;
}

int add_rectangle(int r1, int c1, int r2, int c2, char fill) {
    if (object_count >= MAX_OBJECTS) { printf("Object limit reached!\n"); return -1; }
    Object *o = &objects[object_count++];
    o->id = next_id++;  o->type = SHAPE_RECTANGLE;  o->active = 1;
    o->y1 = r1; o->x1 = c1; o->y2 = r2; o->x2 = c2; o->fill = fill;
    printf("Added rectangle (id=%d)\n", o->id);
    return o->id;
}

int add_line(int r1, int c1, int r2, int c2, char fill) {
    if (object_count >= MAX_OBJECTS) { printf("Object limit reached!\n"); return -1; }
    Object *o = &objects[object_count++];
    o->id = next_id++;  o->type = SHAPE_LINE;  o->active = 1;
    o->y1 = r1; o->x1 = c1; o->y2 = r2; o->x2 = c2; o->fill = fill;
    printf("Added line (id=%d)\n", o->id);
    return o->id;
}

int add_triangle(int r1, int c1, int r2, int c2, int r3, int c3, char fill) {
    if (object_count >= MAX_OBJECTS) { printf("Object limit reached!\n"); return -1; }
    Object *o = &objects[object_count++];
    o->id = next_id++;  o->type = SHAPE_TRIANGLE;  o->active = 1;
    o->y1 = r1; o->x1 = c1; o->y2 = r2; o->x2 = c2; o->y3 = r3; o->x3 = c3; o->fill = fill;
    printf("Added triangle (id=%d)\n", o->id);
    return o->id;
}

// ─── Delete / modify ──────────────────────────────────────────────────────────
int delete_object(int id) {
    for (int i = 0; i < object_count; i++) {
        if (objects[i].id == id) {
            if (!objects[i].active) { printf("Object %d already deleted.\n", id); return 0; }
            objects[i].active = 0;
            printf("Deleted object %d.\n", id);
            return 1;
        }
    }
    printf("Object %d not found.\n", id);
    return 0;
}

void list_objects() {
    int found = 0;
    printf("\n%-4s %-12s %-6s  Parameters\n", "ID", "Type", "Fill");
    printf("----------------------------------------------\n");
    for (int i = 0; i < object_count; i++) {
        if (!objects[i].active) continue;
        found = 1;
        Object *o = &objects[i];
        const char *name;
        char params[80];
        switch (o->type) {
            case SHAPE_CIRCLE:
                name = "Circle";
                snprintf(params, sizeof(params), "centre(%d,%d) r=%d", o->y1, o->x1, o->radius);
                break;
            case SHAPE_RECTANGLE:
                name = "Rectangle";
                snprintf(params, sizeof(params), "(%d,%d)-(%d,%d)", o->y1, o->x1, o->y2, o->x2);
                break;
            case SHAPE_LINE:
                name = "Line";
                snprintf(params, sizeof(params), "(%d,%d)-(%d,%d)", o->y1, o->x1, o->y2, o->x2);
                break;
            case SHAPE_TRIANGLE:
                name = "Triangle";
                snprintf(params, sizeof(params), "(%d,%d) (%d,%d) (%d,%d)", o->y1,o->x1,o->y2,o->x2,o->y3,o->x3);
                break;
            default: name = "Unknown"; params[0] = 0;
        }
        printf("%-4d %-12s %-6c  %s\n", o->id, name, o->fill, params);
    }
    if (!found) printf("  (no objects)\n");
    printf("\n");
}

/* Generic modify: just change fill or reposition via re-entry */
void modify_object(int id) {
    Object *o = NULL;
    for (int i = 0; i < object_count; i++) {
        if (objects[i].id == id && objects[i].active) { o = &objects[i]; break; }
    }
    if (!o) { printf("Object %d not found or deleted.\n", id); return; }

    printf("Modify object %d - choose what to change:\n", id);
    printf("  1. Fill character\n");
    printf("  2. Position / size\n");
    printf("Enter choice: ");
    int choice;
    scanf("%d", &choice);

    if (choice == 1) {
        printf("New fill character ('*' or '_'): ");
        char ch;
        scanf(" %c", &ch);
        if (ch != '*' && ch != '_') { printf("Invalid character. Use '*' or '_'.\n"); return; }
        o->fill = ch;
        printf("Fill updated.\n");
    } else if (choice == 2) {
        switch (o->type) {
            case SHAPE_CIRCLE:
                printf("New centre (row, col, radius): ");
                scanf("%d %d %d", &o->y1, &o->x1, &o->radius);
                break;
            case SHAPE_RECTANGLE:
                printf("New top-left (row, col), bottom-right (row, col): ");
                scanf("%d %d %d %d", &o->y1, &o->x1, &o->y2, &o->x2);
                break;
            case SHAPE_LINE:
                printf("New start (row, col), end (row, col): ");
                scanf("%d %d %d %d", &o->y1, &o->x1, &o->y2, &o->x2);
                break;
            case SHAPE_TRIANGLE:
                printf("New v1 (row, col), v2 (row, col), v3 (row, col): ");
                scanf("%d %d %d %d %d %d", &o->y1, &o->x1, &o->y2, &o->x2, &o->y3, &o->x3);
                break;
        }
        printf("Position/size updated.\n");
    } else {
        printf("Invalid choice.\n");
    }
}

// ─── Menu ─────────────────────────────────────────────────────────────────────
void print_menu() {
    printf("\n+----------------------------------+\n");
    printf("|     2D Text Graphics Editor      |\n");
    printf("+----------------------------------+\n");
    printf("|  1. Add Circle                   |\n");
    printf("|  2. Add Rectangle                |\n");
    printf("|  3. Add Line                     |\n");
    printf("|  4. Add Triangle                 |\n");
    printf("|  5. Delete Object                |\n");
    printf("|  6. Modify Object                |\n");
    printf("|  7. List Objects                 |\n");
    printf("|  8. Display Canvas               |\n");
    printf("|  0. Exit                         |\n");
    printf("+----------------------------------+\n");
    printf("Canvas: %d rows x %d cols   (row 0 = top, col 0 = left)\n", ROWS, COLS);
    printf("Choice: ");
}

char prompt_fill() {
    char ch;
    printf("Fill mode ('*' = filled shape, '_' = outline only): ");
    scanf(" %c", &ch);
    if (ch != '*' && ch != '_') {
        printf("Invalid — defaulting to '*'\n");
        ch = '*';
    }
    return ch;
}

// ─── Main ─────────────────────────────────────────────────────────────────────
int main() {
    init_canvas();

    int running = 1;
    while (running) {
        print_menu();

        int choice;
        if (scanf("%d", &choice) != 1) {
            // flush bad input
            int c; while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }

        switch (choice) {
            case 1: {   // Circle
                int cy, cx, r;
                printf("Centre (row, col, radius): ");
                scanf("%d %d %d", &cy, &cx, &r);
                char fill = prompt_fill();
                add_circle(cy, cx, r, fill);
                render_all();
                display_canvas();
                break;
            }
            case 2: {   // Rectangle
                int r1, c1, r2, c2;
                printf("Top-left (row, col), bottom-right (row, col): ");
                scanf("%d %d %d %d", &r1, &c1, &r2, &c2);
                char fill = prompt_fill();
                add_rectangle(r1, c1, r2, c2, fill);
                render_all();
                display_canvas();
                break;
            }
            case 3: {   // Line
                int r1, c1, r2, c2;
                printf("Start (row, col), end (row, col): ");
                scanf("%d %d %d %d", &r1, &c1, &r2, &c2);
                char fill = prompt_fill();
                add_line(r1, c1, r2, c2, fill);
                render_all();
                display_canvas();
                break;
            }
            case 4: {   // Triangle
                int r1, c1, r2, c2, r3, c3;
                printf("Vertex 1 (row, col): ");  scanf("%d %d", &r1, &c1);
                printf("Vertex 2 (row, col): ");  scanf("%d %d", &r2, &c2);
                printf("Vertex 3 (row, col): ");  scanf("%d %d", &r3, &c3);
                char fill = prompt_fill();
                add_triangle(r1, c1, r2, c2, r3, c3, fill);
                render_all();
                display_canvas();
                break;
            }
            case 5: {   // Delete
                int id;
                printf("Object ID to delete: ");
                scanf("%d", &id);
                delete_object(id);
                render_all();
                display_canvas();
                break;
            }
            case 6: {   // Modify
                int id;
                printf("Object ID to modify: ");
                scanf("%d", &id);
                modify_object(id);
                render_all();
                display_canvas();
                break;
            }
            case 7:     // List
                list_objects();
                break;
            case 8:     // Display
                render_all();
                display_canvas();
                break;
            case 0:
                printf("Goodbye!\n");
                running = 0;
                break;
            default:
                printf("Unknown option. Try again.\n");
        }
    }
    return 0;
}
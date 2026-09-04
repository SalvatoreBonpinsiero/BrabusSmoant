typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

#define SCREEN_W 320
#define SCREEN_H 200

volatile uint8_t* vga_mem = (volatile uint8_t*)0xA0000;

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void delay(volatile uint32_t count) {
    while (count--) {
        __asm__ volatile ("nop");
    }
}

void set_palette_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    outb(0x3C8, index);
    outb(0x3C9, r >> 2);
    outb(0x3C9, g >> 2);
    outb(0x3C9, b >> 2);
}

void init_vga_mode13h(void) {
    outb(0x3C2, 0x63);
    outb(0x3D4, 0x11);
    outb(0x3D5, inb(0x3D5) & 0x7F);

    uint8_t crtc_regs[] = {
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
        0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x9C, 0x8E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
        0xFF
    };
    for (uint8_t i = 0; i < 25; i++) {
        outb(0x3D4, i);
        outb(0x3D5, crtc_regs[i]);
    }

    set_palette_color(16, 45, 10, 50);
    set_palette_color(17, 30, 8, 35);
    set_palette_color(18, 230, 80, 20);
    set_palette_color(19, 25, 25, 25);
    set_palette_color(20, 38, 38, 38);
    set_palette_color(21, 230, 230, 230);
    set_palette_color(22, 140, 140, 140);
    set_palette_color(23, 235, 60, 60);
    set_palette_color(24, 60, 190, 80);
    set_palette_color(25, 240, 180, 30);
    set_palette_color(26, 40, 140, 220);
}

void put_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
        vga_mem[y * SCREEN_W + x] = color;
    }
}

void fill_rect(int x, int y, int w, int h, uint8_t color) {
    for (int j = y; j < y + h; j++) {
        if (j < 0 || j >= SCREEN_H) continue;
        int row = j * SCREEN_W;
        for (int i = x; i < x + w; i++) {
            if (i >= 0 && i < SCREEN_W) {
                vga_mem[row + i] = color;
            }
        }
    }
}

void draw_line_h(int x, int y, int w, uint8_t color) {
    fill_rect(x, y, w, 1, color);
}

void draw_line_v(int x, int y, int h, uint8_t color) {
    fill_rect(x, y, 1, h, color);
}

const uint8_t font_sub_5x7[41][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x7C, 0x12, 0x11, 0x12, 0x7C}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x00, 0x00, 0x00, 0x00, 0x00}, // space (36)
    {0x00, 0x66, 0x66, 0x00, 0x00}, // : (37)
    {0x00, 0x60, 0x60, 0x00, 0x00}, // . (38)
    {0x08, 0x08, 0x08, 0x08, 0x08}, // - (39)
    {0x00, 0x00, 0x7F, 0x00, 0x00}  // | (40)
};

int char_index(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'Z') return 10 + (c - 'A');
    if (c >= 'a' && c <= 'z') return 10 + (c - 'a');
    if (c == ' ') return 36;
    if (c == ':') return 37;
    if (c == '.') return 38;
    if (c == '-') return 39;
    if (c == '|') return 40;
    return 36;
}

void draw_glyph(int x, int y, char c, uint8_t color) {
    int idx = char_index(c);
    for (int col = 0; col < 5; col++) {
        uint8_t line = font_sub_5x7[idx][col];
        for (int row = 0; row < 7; row++) {
            if (line & (1 << row)) {
                put_pixel(x + col, y + row, color);
            }
        }
    }
}

void draw_string(int x, int y, const char* str, uint8_t color) {
    while (*str) {
        draw_glyph(x, y, *str, color);
        x += 6;
        str++;
    }
}

void int_to_str(int val, char* buf) {
    int i = 0;
    if (val == 0) {
        buf[i++] = '0';
        buf[i] = '\0';
        return;
    }
    char temp[10];
    int t = 0;
    while (val > 0) {
        temp[t++] = (val % 10) + '0';
        val /= 10;
    }
    while (t > 0) {
        buf[i++] = temp[--t];
    }
    buf[i] = '\0';
}

void show_plymouth_screen(void) {
    fill_rect(0, 0, SCREEN_W, SCREEN_H, 17);

    draw_string(118, 70, "BRABUS OS", 18);
    draw_string(106, 85, "UBUNTU EDITION", 21);
    draw_string(94, 98, "PASITO 3 CONTROLLER", 22);

    for (int step = 0; step <= 100; step += 4) {
        fill_rect(80, 132, 160, 6, 19);
        fill_rect(80, 132, (step * 160) / 100, 6, 18);
        delay(1200000);
    }
    delay(2500000);
}

void draw_window(int wx, int wy, int ww, int wh, const char* title) {
    fill_rect(wx + 2, wy + 2, ww, wh, 17);
    fill_rect(wx, wy, ww, wh, 20);

    fill_rect(wx, wy, ww, 18, 19);
    draw_line_h(wx, wy + 18, ww, 22);

    fill_rect(wx + 6, wy + 5, 8, 8, 23);
    fill_rect(wx + 18, wy + 5, 8, 8, 25);
    fill_rect(wx + 30, wy + 5, 8, 8, 24);

    draw_string(wx + 45, wy + 6, title, 21);
}

void render_gui(int watts, int is_firing, int puff_ticks, int puffs_count) {
    for (int y = 0; y < SCREEN_H; y++) {
        uint8_t c = (y > 100) ? 17 : 16;
        draw_line_h(0, y, SCREEN_W, c);
    }

    fill_rect(0, 0, SCREEN_W, 14, 19);
    draw_string(8, 4, "ACTIVITIES", 21);
    draw_string(122, 4, "BRABUS OS", 18);
    draw_string(262, 4, "98- BAT", 24);

    fill_rect(0, 14, 24, SCREEN_H - 14, 19);
    draw_line_v(24, 14, SCREEN_H - 14, 20);

    fill_rect(4, 22, 16, 16, 18);
    draw_string(9, 27, "B", 21);

    fill_rect(4, 46, 16, 16, 26);
    draw_string(9, 51, "V", 21);

    fill_rect(4, 70, 16, 16, 22);
    draw_string(7, 75, "CFG", 19);

    int wx = 36;
    int wy = 24;
    int ww = 270;
    int wh = 164;
    draw_window(wx, wy, ww, wh, "BRABUS PASITO CONTROL CENTER");

    draw_string(wx + 12, wy + 26, "POWER OUTPUT", 21);
    char w_buf[8];
    int_to_str(watts, w_buf);
    draw_string(wx + 95, wy + 26, w_buf, 18);
    draw_string(wx + 115, wy + 26, "WATTS", 18);

    fill_rect(wx + 12, wy + 38, 240, 10, 19);
    int p_fill = (watts * 236) / 80;
    fill_rect(wx + 14, wy + 40, p_fill, 6, 18);

    fill_rect(wx + 12, wy + 56, 114, 46, 19);
    draw_string(wx + 16, wy + 60, "ATOMIZER STATS", 26);
    draw_string(wx + 16, wy + 72, "COIL: 0.60 OHM", 21);
    draw_string(wx + 16, wy + 82, "VOLT: 3.84 V", 22);
    draw_string(wx + 16, wy + 92, "AMP : 6.40 A", 22);

    fill_rect(wx + 138, wy + 56, 114, 46, 19);
    draw_string(wx + 142, wy + 60, "SYS DIAGNOSTICS", 25);
    draw_string(wx + 142, wy + 72, "CHIP: ANT-GEN3", 21);
    draw_string(wx + 142, wy + 82, "TEMP: 32.8 C", 24);
    draw_string(wx + 142, wy + 92, "PUFF: ", 22);
    char p_buf[8];
    int_to_str(puffs_count, p_buf);
    draw_string(wx + 176, wy + 92, p_buf, 21);

    if (is_firing) {
        fill_rect(wx + 12, wy + 110, 240, 44, 23);
        draw_string(wx + 75, wy + 116, ">>> FIRING COIL <<<", 21);

        draw_string(wx + 55, wy + 128, "TIME: ", 21);
        char sec_buf[8];
        int_to_str(puff_ticks / 20, sec_buf);
        draw_string(wx + 90, wy + 128, sec_buf, 25);
        draw_string(wx + 105, wy + 128, "SEC / 10.0S MAX", 21);

        fill_rect(wx + 20, wy + 140, 224, 6, 19);
        int cutoff_bar = (puff_ticks * 220) / 200;
        fill_rect(wx + 22, wy + 142, cutoff_bar, 2, 25);
    } else {
        fill_rect(wx + 12, wy + 110, 240, 44, 19);
        draw_string(wx + 75, wy + 120, "STATUS: STANDBY", 24);
        draw_string(wx + 26, wy + 136, "SPACE: FIRE | UP/DOWN: ADJUST WATTS", 22);
    }
}

void kernel_main(void) {
    init_vga_mode13h();
    show_plymouth_screen();

    int watts = 35;
    int is_firing = 0;
    int puff_ticks = 0;
    int puffs_count = 0;
    int space_held = 0;

    render_gui(watts, is_firing, puff_ticks, puffs_count);

    while (1) {
        if (inb(0x64) & 0x01) {
            uint8_t scancode = inb(0x60);

            if (scancode == 0x39) {
                if (!space_held) {
                    space_held = 1;
                    is_firing = 1;
                    puff_ticks = 0;
                    puffs_count++;
                }
            } else if (scancode == 0xB9) {
                space_held = 0;
                is_firing = 0;
                puff_ticks = 0;
            } else if (scancode == 0x48 && !is_firing) {
                if (watts < 80) watts++;
            } else if (scancode == 0x50 && !is_firing) {
                if (watts > 5) watts--;
            }
        }

        if (is_firing) {
            puff_ticks++;
            if (puff_ticks > 200) {
                is_firing = 0;
            }
        }

        render_gui(watts, is_firing, puff_ticks, puffs_count);
        for (volatile int d = 0; d < 28000; d++);
    }
}

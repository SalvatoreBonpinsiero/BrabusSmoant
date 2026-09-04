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

void set_palette(uint8_t idx, uint8_t r, uint8_t g, uint8_t b) {
    outb(0x3C8, idx);
    outb(0x3C9, r >> 2);
    outb(0x3C9, g >> 2);
    outb(0x3C9, b >> 2);
}

void init_mode13h(void) {
    // Включение графического режима VGA 320x200x256 без прерываний BIOS
    outb(0x3C2, 0x63);
    outb(0x3D4, 0x11);
    outb(0x3D5, inb(0x3D5) & 0x7F);

    uint8_t regs[] = {
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
        0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x9C, 0x8E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
        0xFF
    };
    for (uint8_t i = 0; i < 25; i++) {
        outb(0x3D4, i);
        outb(0x3D5, regs[i]);
    }

    outb(0x3C4, 0x04);
    outb(0x3C5, 0x0E);
    outb(0x3CE, 0x05);
    outb(0x3CF, 0x40);
    outb(0x3CE, 0x06);
    outb(0x3CF, 0x05);

    // Палитра Ubuntu / Pasito
    set_palette(0,   20, 20, 20);   // Фон вокруг девайса
    set_palette(1,   45, 10, 36);   // Ubuntu Dark Aubergine (фон экрана)
    set_palette(2,   30, 5, 25);    // Ubuntu Bar
    set_palette(3,   233, 84, 32);  // Ubuntu Orange
    set_palette(4,   255, 255, 255);// Белый
    set_palette(5,   170, 170, 170);// Серый
    set_palette(6,   60, 60, 60);   // Темно-серый
    set_palette(7,   56, 180, 74);  // Зеленый (Ready / Bat)
    set_palette(8,   223, 56, 44);  // Красный (Firing)
    set_palette(9,   246, 211, 45); // Желтый
    set_palette(10,  25, 182, 238); // Cyan
    set_palette(11,  65, 65, 65);   // Корпус мода
    set_palette(12,  10, 10, 10);   // Безель экрана
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

#define DISP_X 120
#define DISP_Y 20
#define PASITO_W 80
#define PASITO_H 160

void p_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < PASITO_W && y >= 0 && y < PASITO_H) {
        vga_mem[(DISP_Y + y) * SCREEN_W + (DISP_X + x)] = color;
    }
}

void p_rect(int x, int y, int w, int h, uint8_t color) {
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            p_pixel(x + i, y + j, color);
        }
    }
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
    {0x00, 0x00, 0x00, 0x00, 0x00}, // пробел
    {0x00, 0x66, 0x66, 0x00, 0x00}, // :
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x00, 0x7F, 0x00, 0x00}  // |
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

void p_char(int x, int y, char c, uint8_t color, int scale) {
    int idx = char_index(c);
    for (int col = 0; col < 5; col++) {
        uint8_t line = font_sub_5x7[idx][col];
        for (int row = 0; row < 7; row++) {
            if (line & (1 << row)) {
                for (int dy = 0; dy < scale; dy++) {
                    for (int dx = 0; dx < scale; dx++) {
                        p_pixel(x + col * scale + dx, y + row * scale + dy, color);
                    }
                }
            }
        }
    }
}

void p_string(int x, int y, const char* str, uint8_t color, int scale) {
    while (*str) {
        p_char(x, y, *str, color, scale);
        x += (5 + 1) * scale;
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

void render_device_body(void) {
    fill_rect(0, 0, SCREEN_W, SCREEN_H, 0);

    // Корпус мода вокруг экрана
    fill_rect(DISP_X - 25, 0, PASITO_W + 50, SCREEN_H, 11);
    fill_rect(DISP_X - 22, 0, PASITO_W + 44, SCREEN_H, 6);
    fill_rect(DISP_X - 18, 0, PASITO_W + 36, SCREEN_H, 11);

    // Безель экрана 80x160
    fill_rect(DISP_X - 3, DISP_Y - 3, PASITO_W + 6, PASITO_H + 6, 12);
    
    // Кнопка Fire сверху
    fill_rect(DISP_X + 20, 4, 40, 10, 12);
    fill_rect(DISP_X + 22, 6, 36, 6, 6);

    // Кнопки регулировки снизу
    fill_rect(DISP_X + 15, DISP_Y + PASITO_H + 5, 20, 8, 6);
    fill_rect(DISP_X + PASITO_W - 35, DISP_Y + PASITO_H + 5, 20, 8, 6);
}

void render_screen(int watts, int is_firing, int puff_ticks, int puffs_count) {
    p_rect(0, 0, PASITO_W, PASITO_H, 1);

    // Верхняя панель Ubuntu
    p_rect(0, 0, PASITO_W, 11, 2);
    p_string(3, 2, "BRABUS", 3, 1);
    p_rect(58, 3, 14, 5, 6);
    p_rect(59, 4, 10, 3, 7);
    p_pixel(72, 4, 6);
    p_rect(0, 11, PASITO_W, 1, 3);

    if (is_firing) {
        p_rect(3, 15, 74, 140, 8);
        p_rect(5, 17, 70, 136, 1);

        p_string(14, 25, "VAPING", 4, 2);
        p_string(14, 43, "ACTIVE", 3, 2);

        char s_buf[8];
        int_to_str(puff_ticks / 20, s_buf);
        p_string(28, 70, s_buf, 9, 3);
        p_string(48, 80, "S", 4, 1);

        p_string(12, 105, "MAX 10.0S", 5, 1);

        p_rect(8, 125, 64, 8, 12);
        int bar_w = (puff_ticks * 60) / 200;
        p_rect(10, 127, bar_w, 4, 9);
    } else {
        p_string(6, 16, "MODE: SPORT", 10, 1);

        char w_buf[8];
        int_to_str(watts, w_buf);
        int w_x = (watts < 10) ? 22 : 12;
        p_string(w_x, 28, w_buf, 4, 4);
        p_string(54, 44, "W", 3, 2);

        p_rect(4, 63, 72, 5, 12);
        int p_bar = (watts * 68) / 80;
        p_rect(6, 64, p_bar, 3, 3);

        p_rect(4, 73, 72, 44, 2);
        p_rect(4, 73, 72, 1, 6);
        p_string(8, 78, "RES : 0.60 O", 4, 1);
        p_string(8, 90, "VOLT: 3.84 V", 5, 1);
        p_string(8, 102, "AMP : 6.40 A", 5, 1);

        p_rect(4, 121, 72, 34, 2);
        p_rect(4, 121, 72, 1, 6);
        p_string(8, 126, "PUFF: ", 5, 1);
        char p_buf[8];
        int_to_str(puffs_count, p_buf);
        p_string(38, 126, p_buf, 9, 1);

        p_string(8, 140, "STATUS: READY", 7, 1);
    }
}

void kernel_main(void) {
    init_mode13h();
    render_device_body();

    int watts = 35;
    int is_firing = 0;
    int puff_ticks = 0;
    int puffs_count = 0;
    int space_held = 0;

    render_screen(watts, is_firing, puff_ticks, puffs_count);

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

        render_screen(watts, is_firing, puff_ticks, puffs_count);
        for (volatile int d = 0; d < 25000; d++);
    }
}

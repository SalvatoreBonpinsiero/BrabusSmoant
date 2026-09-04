typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

struct multiboot_info {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type;
} __attribute__((packed));

#define SCREEN_W 1024
#define SCREEN_H 768

static uint32_t* front_fb = 0;
static uint32_t  back_buffer[SCREEN_W * SCREEN_H];

// Цветовая палитра монохромного ЖК-экрана Nokia 8110
#define C_DESK_BG      0xFF121212
#define C_CASE_PLASTIC 0xFF263238
#define C_CASE_SHADOW  0xFF1B2428
#define C_LCD_BG       0xFF8DA378
#define C_LCD_PIXEL    0xFF1D2619
#define C_LCD_GRID     0xFF849970

#define LCD_X 332
#define LCD_Y 120
#define LCD_W 360
#define LCD_H 500

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void draw_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
        back_buffer[y * SCREEN_W + x] = color;
    }
}

void fill_rect(int x, int y, int w, int h, uint32_t color) {
    for (int j = y; j < y + h; j++) {
        if (j < 0 || j >= SCREEN_H) continue;
        int row = j * SCREEN_W;
        for (int i = x; i < x + w; i++) {
            if (i >= 0 && i < SCREEN_W) {
                back_buffer[row + i] = color;
            }
        }
    }
}

void lcd_pixel(int x, int y, uint32_t color, int size) {
    fill_rect(LCD_X + x * size, LCD_Y + y * size, size, size, color);
}

const uint8_t font5x7[42][5] = {
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
    {0x00, 0x00, 0x00, 0x00, 0x00}, // space
    {0x00, 0x66, 0x66, 0x00, 0x00}, // :
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x00, 0x7F, 0x00, 0x00}, // |
    {0x3E, 0x41, 0x5D, 0x55, 0x5E}  // Ohm
};

int get_char_idx(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'Z') return 10 + (c - 'A');
    if (c >= 'a' && c <= 'z') return 10 + (c - 'a');
    if (c == ' ') return 36;
    if (c == ':') return 37;
    if (c == '.') return 38;
    if (c == '-') return 39;
    if (c == '|') return 40;
    if (c == '@') return 41;
    return 36;
}

void draw_lcd_text(int x, int y, const char* str, int scale) {
    int cur_x = x;
    while (*str) {
        int idx = get_char_idx(*str);
        for (int col = 0; col < 5; col++) {
            uint8_t col_data = font5x7[idx][col];
            for (int row = 0; row < 7; row++) {
                if (col_data & (1 << row)) {
                    fill_rect(LCD_X + (cur_x + col * scale), LCD_Y + (y + row * scale), scale, scale, C_LCD_PIXEL);
                }
            }
        }
        cur_x += 6 * scale;
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

void flip_screen(void) {
    for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
        front_fb[i] = back_buffer[i];
    }
}

void render_frame(int watts, int is_firing, int puff_ticks, int puffs_count) {
    fill_rect(0, 0, SCREEN_W, SCREEN_H, C_DESK_BG);

    // Корпус Nokia 8110 со скруглениями
    fill_rect(LCD_X - 44, LCD_Y - 80, LCD_W + 88, LCD_H + 160, C_CASE_SHADOW);
    fill_rect(LCD_X - 40, LCD_Y - 76, LCD_W + 80, LCD_H + 152, C_CASE_PLASTIC);

    // Динамик Nokia
    fill_rect(LCD_X + (LCD_W / 2) - 40, LCD_Y - 45, 80, 8, C_DESK_BG);
    draw_lcd_text((LCD_W / 2) - 48, -25, "NOKIA", 2);

    // Внутренняя рамка и подсветка экрана
    fill_rect(LCD_X - 6, LCD_Y - 6, LCD_W + 12, LCD_H + 12, C_CASE_SHADOW);
    fill_rect(LCD_X, LCD_Y, LCD_W, LCD_H, C_LCD_BG);

    // Сегментная сетка ЖК-матрицы
    for (int j = 0; j < LCD_H; j += 4) {
        fill_rect(LCD_X, LCD_Y + j, LCD_W, 1, C_LCD_GRID);
    }

    // 1. Статус-бар (Антенна слева, Батарея справа)
    fill_rect(LCD_X + 15, LCD_Y + 15, 3, 15, C_LCD_PIXEL);
    fill_rect(LCD_X + 21, LCD_Y + 20, 3, 10, C_LCD_PIXEL);
    fill_rect(LCD_X + 27, LCD_Y + 24, 3, 6, C_LCD_PIXEL);
    fill_rect(LCD_X + 33, LCD_Y + 27, 3, 3, C_LCD_PIXEL);

    draw_lcd_text(120, 16, "BRABUS", 3);

    fill_rect(LCD_X + LCD_W - 55, LCD_Y + 15, 36, 16, C_LCD_PIXEL);
    fill_rect(LCD_X + LCD_W - 53, LCD_Y + 17, 32, 12, C_LCD_BG);
    fill_rect(LCD_X + LCD_W - 50, LCD_Y + 19, 26, 8, C_LCD_PIXEL);
    fill_rect(LCD_X + LCD_W - 19, LCD_Y + 20, 3, 6, C_LCD_PIXEL);

    fill_rect(LCD_X + 10, LCD_Y + 42, LCD_W - 20, 2, C_LCD_PIXEL);

    if (is_firing) {
        fill_rect(LCD_X + 15, LCD_Y + 70, LCD_W - 30, 290, C_LCD_PIXEL);
        fill_rect(LCD_X + 19, LCD_Y + 74, LCD_W - 38, 282, C_LCD_BG);

        draw_lcd_text(50, 110, "VAPING", 5);

        char sec_buf[8];
        int_to_str(puff_ticks / 20, sec_buf);
        draw_lcd_text(110, 190, sec_buf, 7);
        draw_lcd_text(180, 215, "SEC", 3);

        draw_lcd_text(70, 280, "LIMIT 10.0S", 3);

        fill_rect(LCD_X + 35, LCD_Y + 310, LCD_W - 70, 20, C_LCD_PIXEL);
        fill_rect(LCD_X + 37, LCD_Y + 312, LCD_W - 74, 16, C_LCD_BG);
        int cut_fill = (puff_ticks * (LCD_W - 78)) / 200;
        fill_rect(LCD_X + 39, LCD_Y + 314, cut_fill, 12, C_LCD_PIXEL);

    } else {
        draw_lcd_text(25, 65, "POWER SETTING", 2);

        char w_str[8];
        int_to_str(watts, w_str);
        int w_x = (watts < 10) ? 90 : 50;
        draw_lcd_text(w_x, 95, w_str, 8);
        draw_lcd_text(205, 130, "WATTS", 3);

        fill_rect(LCD_X + 20, LCD_Y + 180, LCD_W - 40, 18, C_LCD_PIXEL);
        fill_rect(LCD_X + 22, LCD_Y + 182, LCD_W - 44, 14, C_LCD_BG);
        int bar_w = (watts * (LCD_W - 48)) / 80;
        fill_rect(LCD_X + 24, LCD_Y + 184, bar_w, 10, C_LCD_PIXEL);

        fill_rect(LCD_X + 20, LCD_Y + 220, LCD_W - 40, 130, C_LCD_PIXEL);
        fill_rect(LCD_X + 22, LCD_Y + 222, LCD_W - 44, 126, C_LCD_BG);

        draw_lcd_text(35, 235, "COIL : 0.60 OHM", 2);
        draw_lcd_text(35, 260, "VOLT : 3.84 V", 2);
        draw_lcd_text(35, 285, "AMP  : 6.40 A", 2);

        draw_lcd_text(35, 315, "PUFFS: ", 2);
        char p_str[8];
        int_to_str(puffs_count, p_str);
        draw_lcd_text(125, 315, p_str, 2);

        draw_lcd_text(40, 380, "PASITO 8110 BANANA", 2);
        draw_lcd_text(35, 410, "SMOANT CHIP READY", 2);
    }

    fill_rect(LCD_X + 10, LCD_Y + LCD_H - 45, LCD_W - 20, 2, C_LCD_PIXEL);
    draw_lcd_text(25, LCD_H - 30, "Menu", 3);
    draw_lcd_text(LCD_W - 95, LCD_H - 30, "Back", 3);

    // Софт-кнопки слайдера Nokia
    fill_rect(LCD_X + 20, LCD_Y + LCD_H + 20, 60, 16, C_DESK_BG);
    fill_rect(LCD_X + LCD_W - 80, LCD_Y + LCD_H + 20, 60, 16, C_DESK_BG);
    fill_rect(LCD_X + (LCD_W / 2) - 30, LCD_Y + LCD_H + 15, 60, 26, C_DESK_BG);

    flip_screen();
}

void kernel_main(struct multiboot_info* mbi) {
    if (mbi && (mbi->flags & (1 << 12)) && mbi->framebuffer_addr) {
        front_fb = (uint32_t*)((uint32_t)mbi->framebuffer_addr);
    } else {
        front_fb = (uint32_t*)0xE0000000;
    }

    int watts = 35;
    int is_firing = 0;
    int puff_ticks = 0;
    int puffs_count = 0;
    int space_held = 0;

    render_frame(watts, is_firing, puff_ticks, puffs_count);

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

        render_frame(watts, is_firing, puff_ticks, puffs_count);

        for (volatile int d = 0; d < 20000; d++);
    }
}

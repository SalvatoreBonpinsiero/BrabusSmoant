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

// Цветовая палитра Ubuntu Yaru
#define UBUNTU_BG_TOP    0xFF4C1B41
#define UBUNTU_BG_BOT    0xFF2C001E
#define UBUNTU_ORANGE    0xFFE95420
#define UBUNTU_PANEL     0xFF1D1D1D
#define UBUNTU_WIN_BG    0xFF2A2A2A
#define UBUNTU_CARD      0xFF343434
#define UBUNTU_BORDER    0xFF454545
#define UBUNTU_TEXT      0xFFF5F5F5
#define UBUNTU_MUTED     0xFFA0A0A0
#define UBUNTU_GREEN     0xFF38B44A
#define UBUNTU_RED       0xFFE93224
#define UBUNTU_YELLOW    0xFFF6D32D
#define UBUNTU_BLUE      0xFF19B6EE

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
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

void draw_rounded_card(int x, int y, int w, int h, uint32_t bg, uint32_t border) {
    fill_rect(x + 1, y, w - 2, h, bg);
    fill_rect(x, y + 1, w, h - 2, bg);
    fill_rect(x + 1, y, w - 2, 1, border);
    fill_rect(x + 1, y + h - 1, w - 2, 1, border);
    fill_rect(x, y + 1, 1, h - 2, border);
    fill_rect(x + w - 1, y + 1, 1, h - 2, border);
}

const uint8_t font5x7[43][5] = {
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
    {0x00, 0x41, 0x3E, 0x00, 0x00}, // (
    {0x00, 0x3E, 0x41, 0x00, 0x00}  // )
};

int char_idx(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'Z') return 10 + (c - 'A');
    if (c >= 'a' && c <= 'z') return 10 + (c - 'a');
    if (c == ' ') return 36;
    if (c == ':') return 37;
    if (c == '.') return 38;
    if (c == '-') return 39;
    if (c == '|') return 40;
    if (c == '(') return 41;
    if (c == ')') return 42;
    return 36;
}

void draw_text(int x, int y, const char* str, uint32_t color, int scale) {
    while (*str) {
        int idx = char_idx(*str);
        for (int col = 0; col < 5; col++) {
            uint8_t line = font5x7[idx][col];
            for (int row = 0; row < 7; row++) {
                if (line & (1 << row)) {
                    fill_rect(x + col * scale, y + row * scale, scale, scale, color);
                }
            }
        }
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

void flip_screen(void) {
    for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
        front_fb[i] = back_buffer[i];
    }
}

void render_ubuntu_brabus(int watts, int is_firing, int puff_ticks, int puffs_count, 
                          int mode, int locked, int stealth) {
    if (stealth && !is_firing) {
        fill_rect(0, 0, SCREEN_W, SCREEN_H, 0xFF050505);
        flip_screen();
        return;
    }

    for (int y = 0; y < SCREEN_H; y++) {
        uint32_t clr = (y < 400) ? UBUNTU_BG_TOP : UBUNTU_BG_BOT;
        fill_rect(0, y, SCREEN_W, 1, clr);
    }

    fill_rect(0, 0, SCREEN_W, 30, UBUNTU_PANEL);
    draw_text(15, 8, "ACTIVITIES", UBUNTU_TEXT, 2);
    draw_text(450, 8, "BRABUS", UBUNTU_ORANGE, 2);

    if (locked) draw_text(760, 8, "LOCKED", UBUNTU_YELLOW, 2);
    else        draw_text(760, 8, "UNLOCKED", UBUNTU_GREEN, 2);

    draw_text(880, 8, "BAT 98%", UBUNTU_GREEN, 2);

    fill_rect(0, 30, 60, SCREEN_H - 30, 0xEE141414);
    draw_rounded_card(8, 45, 44, 44, UBUNTU_ORANGE, UBUNTU_ORANGE);
    draw_text(22, 53, "B", UBUNTU_TEXT, 4);

    draw_rounded_card(8, 100, 44, 44, UBUNTU_CARD, UBUNTU_BORDER);
    draw_text(20, 114, "VW", UBUNTU_BLUE, 2);

    draw_rounded_card(8, 155, 44, 44, UBUNTU_CARD, UBUNTU_BORDER);
    draw_text(17, 169, "CFG", UBUNTU_MUTED, 2);

    int wx = 120, wy = 70, ww = 800, wh = 620;
    fill_rect(wx + 8, wy + 8, ww, wh, 0x55000000);
    draw_rounded_card(wx, wy, ww, wh, UBUNTU_WIN_BG, UBUNTU_BORDER);

    fill_rect(wx + 1, wy + 1, ww - 2, 45, 0xFF353535);
    fill_rect(wx, wy + 45, ww, 1, UBUNTU_BORDER);

    fill_rect(wx + 16, wy + 16, 14, 14, UBUNTU_RED);
    fill_rect(wx + 38, wy + 16, 14, 14, UBUNTU_YELLOW);
    fill_rect(wx + 60, wy + 16, 14, 14, UBUNTU_GREEN);
    draw_text(wx + 95, wy + 15, "BRABUS - SMOANT ANT-CHIP HARDWARE MONITOR", UBUNTU_TEXT, 2);

    const char* mode_names[] = { "VARIABLE WATTAGE (VW)", "DVW TEMP CURVE", "BYPASS (DIRECT CELL)" };
    draw_text(wx + 40, wy + 70, "ACTIVE FIRING MODE:", UBUNTU_MUTED, 2);
    draw_text(wx + 260, wy + 70, mode_names[mode], UBUNTU_YELLOW, 2);

    draw_rounded_card(wx + 40, wy + 105, 720, 110, UBUNTU_CARD, UBUNTU_BORDER);
    draw_text(wx + 65, wy + 125, "TARGET POWER", UBUNTU_MUTED, 2);

    char w_buf[8];
    int_to_str(watts, w_buf);
    draw_text(wx + 65, wy + 150, w_buf, UBUNTU_ORANGE, 5);
    draw_text(wx + 165, wy + 165, "WATTS", UBUNTU_ORANGE, 3);

    fill_rect(wx + 280, wy + 155, 450, 24, 0xFF1C1C1C);
    int p_bar = (watts * 442) / 80;
    fill_rect(wx + 284, wy + 159, p_bar, 16, UBUNTU_ORANGE);

    draw_rounded_card(wx + 40, wy + 235, 345, 170, UBUNTU_CARD, UBUNTU_BORDER);
    draw_text(wx + 65, wy + 255, "COIL DIAGNOSTICS", UBUNTU_BLUE, 2);
    draw_text(wx + 65, wy + 290, "RESISTANCE : 0.60 OHM", UBUNTU_TEXT, 2);
    draw_text(wx + 65, wy + 320, "VOLTAGE    : 3.84 V", UBUNTU_MUTED, 2);
    draw_text(wx + 65, wy + 350, "CURRENT    : 6.40 A", UBUNTU_MUTED, 2);

    draw_rounded_card(wx + 415, wy + 235, 345, 170, UBUNTU_CARD, UBUNTU_BORDER);
    draw_text(wx + 440, wy + 255, "PASITO FEATURES", UBUNTU_GREEN, 2);
    
    draw_text(wx + 440, wy + 290, "KEY LOCK   : ", UBUNTU_MUTED, 2);
    draw_text(wx + 570, wy + 290, locked ? "ACTIVE" : "OFF", locked ? UBUNTU_YELLOW : UBUNTU_TEXT, 2);

    draw_text(wx + 440, wy + 320, "STEALTH    : ", UBUNTU_MUTED, 2);
    draw_text(wx + 570, wy + 320, stealth ? "ENABLED" : "DISABLED", UBUNTU_TEXT, 2);

    draw_text(wx + 440, wy + 350, "PUFF COUNT : ", UBUNTU_MUTED, 2);
    char p_buf[8];
    int_to_str(puffs_count, p_buf);
    draw_text(wx + 570, wy + 350, p_buf, UBUNTU_YELLOW, 2);

    if (is_firing) {
        draw_rounded_card(wx + 40, wy + 425, 720, 160, UBUNTU_RED, UBUNTU_YELLOW);
        draw_text(wx + 260, wy + 445, ">>> COIL HEATING ACTIVE <<<", UBUNTU_TEXT, 3);

        draw_text(wx + 280, wy + 485, "DURATION: ", UBUNTU_TEXT, 2);
        char s_buf[8];
        int_to_str(puff_ticks / 20, s_buf);
        draw_text(wx + 390, wy + 480, s_buf, UBUNTU_YELLOW, 3);
        draw_text(wx + 430, wy + 485, "S / 10.0S CUTOFF", UBUNTU_TEXT, 2);

        fill_rect(wx + 80, wy + 525, 640, 20, 0xFF700000);
        int cut_bar = (puff_ticks * 632) / 200;
        fill_rect(wx + 84, wy + 529, cut_bar, 12, UBUNTU_YELLOW);
    } else {
        draw_rounded_card(wx + 40, wy + 425, 720, 160, UBUNTU_CARD, UBUNTU_BORDER);
        draw_text(wx + 280, wy + 450, "STATUS: SYSTEM READY", UBUNTU_GREEN, 2);
        draw_text(wx + 80, wy + 490, "HOLD [FIRE]: VAPE  |  [^/v]: ADJUST WATTS", UBUNTU_TEXT, 2);
        draw_text(wx + 80, wy + 520, "3x [FIRE]: MODE  |  [^]+[v]: LOCK  |  [FIRE]+[^]: STEALTH", UBUNTU_MUTED, 2);
    }

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
    
    int mode = 0;
    int locked = 0;
    int stealth = 0;

    int fire_held = 0;
    int up_held = 0;
    int down_held = 0;

    int fire_click_count = 0;
    int fire_click_timer = 0;

    render_ubuntu_brabus(watts, is_firing, puff_ticks, puffs_count, mode, locked, stealth);

    while (1) {
        if (inb(0x64) & 0x01) {
            uint8_t scancode = inb(0x60);

            // Кнопка FIRE (Пробел: 0x39 нажат, 0xB9 отпущен)
            if (scancode == 0x39) {
                if (!fire_held) {
                    fire_held = 1;
                    fire_click_count++;
                    fire_click_timer = 30; // окно для тройного клика

                    // Комбинация: FIRE + DOWN = Сброс тяг (Puff Clear)
                    if (down_held) {
                        puffs_count = 0;
                        fire_click_count = 0;
                    }
                    // Комбинация: FIRE + UP = Стелс-режим (Stealth)
                    else if (up_held) {
                        stealth = !stealth;
                        fire_click_count = 0;
                    }
                }
            } else if (scancode == 0xB9) {
                fire_held = 0;
                is_firing = 0;
                puff_ticks = 0;
            }

            // Кнопка ВВЕРХ (Стрелка вверх: 0x48 нажата, 0xC8 отпущена)
            else if (scancode == 0x48) {
                up_held = 1;
                // Комбинация: UP + DOWN = Блокировка (Key Lock)
                if (down_held && !is_firing) {
                    locked = !locked;
                } else if (!locked && !is_firing) {
                    if (watts < 80) watts++;
                }
            } else if (scancode == 0xC8) {
                up_held = 0;
            }

            // Кнопка ВНИЗ (Стрелка вниз: 0x50 нажата, 0xD0 отпущена)
            else if (scancode == 0x50) {
                down_held = 1;
                // Комбинация: UP + DOWN = Блокировка (Key Lock)
                if (up_held && !is_firing) {
                    locked = !locked;
                } else if (!locked && !is_firing) {
                    if (watts > 5) watts--;
                }
            } else if (scancode == 0xD0) {
                down_held = 0;
            }
        }

        // Логика тройного клика FIRE (3x Click -> Mode Switch)
        if (fire_click_timer > 0) {
            fire_click_timer--;
            if (fire_click_count >= 3) {
                mode = (mode + 1) % 3;
                fire_click_count = 0;
                fire_click_timer = 0;
            }
        } else {
            // Если таймер истек и это был просто зажим FIRE -> запуск парения
            if (fire_held && fire_click_count == 1 && !up_held && !down_held) {
                if (!is_firing) {
                    is_firing = 1;
                    puff_ticks = 0;
                    puffs_count++;
                }
            }
            fire_click_count = 0;
        }

        // Счётчик отсечки парения (10 секунд)
        if (is_firing) {
            puff_ticks++;
            if (puff_ticks > 200) {
                is_firing = 0;
            }
        }

        render_ubuntu_brabus(watts, is_firing, puff_ticks, puffs_count, mode, locked, stealth);
        for (volatile int d = 0; d < 20000; d++);
    }
}

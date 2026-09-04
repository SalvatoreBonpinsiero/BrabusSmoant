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

void delay(volatile uint32_t count) {
    while (count--) {
        __asm__ volatile ("nop");
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

void draw_rounded_card(int x, int y, int w, int h, uint32_t bg, uint32_t border) {
    fill_rect(x + 2, y, w - 4, h, bg);
    fill_rect(x, y + 2, w, h - 4, bg);
    fill_rect(x + 2, y, w - 4, 1, border);
    fill_rect(x + 2, y + h - 1, w - 4, 1, border);
    fill_rect(x, y + 2, 1, h - 4, border);
    fill_rect(x + w - 1, y + 2, 1, h - 4, border);
}

void flip_screen(void) {
    for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
        front_fb[i] = back_buffer[i];
    }
}

// Новый системный шрифт 8x12 (строки сверху вниз)
const uint8_t font8x12[43][12] = {
    {0x3C, 0x66, 0xC3, 0xC3, 0xC7, 0xCF, 0xDB, 0xF3, 0xE3, 0xC3, 0x66, 0x3C}, // 0
    {0x18, 0x38, 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x7E}, // 1
    {0x3C, 0x66, 0xC3, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0xFF, 0xFF}, // 2
    {0x3C, 0x66, 0xC3, 0x03, 0x06, 0x1C, 0x06, 0x03, 0x03, 0xC3, 0x66, 0x3C}, // 3
    {0x06, 0x0E, 0x1E, 0x36, 0x66, 0xC6, 0xFF, 0xFF, 0x06, 0x06, 0x06, 0x06}, // 4
    {0xFF, 0xFF, 0xC0, 0xC0, 0xFC, 0xFE, 0x03, 0x03, 0x03, 0xC3, 0x66, 0x3C}, // 5
    {0x1E, 0x30, 0x60, 0xC0, 0xFC, 0xFE, 0xC3, 0xC3, 0xC3, 0xC3, 0x66, 0x3C}, // 6
    {0xFF, 0xFF, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x60, 0x60, 0x60, 0x60}, // 7
    {0x3C, 0x66, 0xC3, 0xC3, 0x66, 0x3C, 0x66, 0xC3, 0xC3, 0xC3, 0x66, 0x3C}, // 8
    {0x3C, 0x66, 0xC3, 0xC3, 0xC3, 0x7F, 0x3F, 0x03, 0x06, 0x0C, 0x18, 0x70}, // 9
    {0x18, 0x3C, 0x66, 0xC3, 0xC3, 0xFF, 0xFF, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3}, // A
    {0xFC, 0xFE, 0xC3, 0xC3, 0xFC, 0xFE, 0xC3, 0xC3, 0xC3, 0xC3, 0xFE, 0xFC}, // B
    {0x3C, 0x66, 0xC3, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC3, 0x66, 0x3C}, // C
    {0xF8, 0xFC, 0xC6, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC6, 0xFC, 0xF8}, // D
    {0xFF, 0xFF, 0xC0, 0xC0, 0xFC, 0xFC, 0xC0, 0xC0, 0xC0, 0xC0, 0xFF, 0xFF}, // E
    {0xFF, 0xFF, 0xC0, 0xC0, 0xFC, 0xFC, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0}, // F
    {0x3C, 0x66, 0xC3, 0xC0, 0xC0, 0xCF, 0xCF, 0xC3, 0xC3, 0xC3, 0x66, 0x3C}, // G
    {0xC3, 0xC3, 0xC3, 0xC3, 0xFF, 0xFF, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3}, // H
    {0x7E, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x7E}, // I
    {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0xC3, 0xC3, 0xC3, 0x66, 0x3C}, // J
    {0xC3, 0xC6, 0xCC, 0xD8, 0xF0, 0xF0, 0xD8, 0xCC, 0xC6, 0xC3, 0xC3, 0xC3}, // K
    {0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xFF, 0xFF}, // L
    {0xC3, 0xE7, 0xFF, 0xDB, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3}, // M
    {0xC3, 0xE3, 0xF3, 0xDB, 0xCF, 0xC7, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3}, // N
    {0x3C, 0x66, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0x66, 0x3C}, // O
    {0xFC, 0xFE, 0xC3, 0xC3, 0xFE, 0xFC, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0}, // P
    {0x3C, 0x66, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xDB, 0xDF, 0x67, 0x33, 0x01}, // Q
    {0xFC, 0xFE, 0xC3, 0xC3, 0xFE, 0xFC, 0xD8, 0xCC, 0xC6, 0xC3, 0xC3, 0xC3}, // R
    {0x3C, 0x66, 0xC3, 0xC0, 0x78, 0x1E, 0x03, 0x03, 0xC3, 0xC3, 0x66, 0x3C}, // S
    {0xFF, 0xFF, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18}, // T
    {0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0x7E, 0x3C}, // U
    {0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x18}, // V
    {0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xDB, 0xDB, 0xFF, 0xE7, 0xC3, 0xC3}, // W
    {0xC3, 0xC3, 0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0xC3, 0xC3, 0xC3}, // X
    {0xC3, 0xC3, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18}, // Y
    {0xFF, 0xFF, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x80, 0x80, 0xFF, 0xFF}, // Z
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // space
    {0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00}, // :
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00}, // .
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00}, // -
    {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18}, // |
    {0x0C, 0x18, 0x30, 0x60, 0x60, 0x60, 0x60, 0x60, 0x30, 0x18, 0x0C, 0x00}, // (
    {0x30, 0x18, 0x0C, 0x06, 0x06, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x30, 0x00}  // )
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
        for (int row = 0; row < 12; row++) {
            uint8_t line = font8x12[idx][row];
            for (int col = 0; col < 8; col++) {
                if (line & (0x80 >> col)) {
                    fill_rect(x + col * scale, y + row * scale, scale, scale, color);
                }
            }
        }
        x += (8 + 1) * scale;
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

void show_boot_animation(void) {
    for (int frame = 0; frame <= 100; frame += 2) {
        // Отрисовка фона
        for (int y = 0; y < SCREEN_H; y++) {
            uint32_t clr = (y < 400) ? UBUNTU_BG_TOP : UBUNTU_BG_BOT;
            fill_rect(0, y, SCREEN_W, 1, clr);
        }

        // Логотип BRABUS по центру
        draw_text(380, 260, "BRABUS", UBUNTU_ORANGE, 5);
        draw_text(390, 340, "PASITO III SPORT OS", UBUNTU_TEXT, 2);

        // Полоса прогресса
        draw_rounded_card(312, 430, 400, 20, 0xFF200718, UBUNTU_BORDER);
        int bar_w = (frame * 392) / 100;
        if (bar_w > 0) {
            fill_rect(316, 434, bar_w, 12, UBUNTU_ORANGE);
        }

        // Анимация 3 точек загрузки в стиле Ubuntu Plymouth
        int dot_phase = (frame / 10) % 3;
        for (int d = 0; d < 3; d++) {
            uint32_t dot_color = (d == dot_phase) ? UBUNTU_ORANGE : UBUNTU_MUTED;
            fill_rect(492 + (d * 18), 470, 8, 8, dot_color);
        }

        flip_screen();
        delay(600000);
    }
    delay(1500000);
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

    // Верхняя панель (GNOME Top Bar)
    fill_rect(0, 0, SCREEN_W, 32, UBUNTU_PANEL);
    draw_text(15, 8, "ACTIVITIES", UBUNTU_TEXT, 1);
    draw_text(460, 8, "BRABUS", UBUNTU_ORANGE, 1);

    if (locked) draw_text(760, 8, "LOCKED", UBUNTU_YELLOW, 1);
    else        draw_text(760, 8, "UNLOCKED", UBUNTU_GREEN, 1);

    draw_text(870, 8, "BAT 98%", UBUNTU_GREEN, 1);

    // Док слева
    fill_rect(0, 32, 60, SCREEN_H - 32, 0xEE141414);
    draw_rounded_card(8, 45, 44, 44, UBUNTU_ORANGE, UBUNTU_ORANGE);
    draw_text(22, 51, "B", UBUNTU_TEXT, 3);

    draw_rounded_card(8, 100, 44, 44, UBUNTU_CARD, UBUNTU_BORDER);
    draw_text(16, 112, "VW", UBUNTU_BLUE, 2);

    draw_rounded_card(8, 155, 44, 44, UBUNTU_CARD, UBUNTU_BORDER);
    draw_text(12, 168, "CFG", UBUNTU_MUTED, 1);

    // Окно управления
    int wx = 120, wy = 70, ww = 800, wh = 620;
    fill_rect(wx + 8, wy + 8, ww, wh, 0x55000000);
    draw_rounded_card(wx, wy, ww, wh, UBUNTU_WIN_BG, UBUNTU_BORDER);

    fill_rect(wx + 2, wy + 2, ww - 4, 44, 0xFF353535);
    fill_rect(wx, wy + 46, ww, 1, UBUNTU_BORDER);

    fill_rect(wx + 16, wy + 16, 14, 14, UBUNTU_RED);
    fill_rect(wx + 38, wy + 16, 14, 14, UBUNTU_YELLOW);
    fill_rect(wx + 60, wy + 16, 14, 14, UBUNTU_GREEN);
    draw_text(wx + 95, wy + 15, "BRABUS - SMOANT ANT-CHIP HARDWARE MONITOR", UBUNTU_TEXT, 1);

    const char* mode_names[] = { "VARIABLE WATTAGE (VW)", "DVW TEMP CURVE", "BYPASS (DIRECT CELL)" };
    draw_text(wx + 40, wy + 68, "ACTIVE FIRING MODE:", UBUNTU_MUTED, 1);
    draw_text(wx + 260, wy + 68, mode_names[mode], UBUNTU_YELLOW, 1);

    // Карточка мощности
    draw_rounded_card(wx + 40, wy + 100, 720, 115, UBUNTU_CARD, UBUNTU_BORDER);
    draw_text(wx + 65, wy + 115, "TARGET POWER OUTPUT", UBUNTU_MUTED, 1);

    char w_buf[8];
    int_to_str(watts, w_buf);
    draw_text(wx + 65, wy + 140, w_buf, UBUNTU_ORANGE, 4);
    draw_text(wx + 175, wy + 155, "WATTS", UBUNTU_ORANGE, 2);

    fill_rect(wx + 290, wy + 155, 440, 24, 0xFF1C1C1C);
    int p_bar = (watts * 432) / 80;
    fill_rect(wx + 294, wy + 159, p_bar, 16, UBUNTU_ORANGE);

    // Карточка атомайзера
    draw_rounded_card(wx + 40, wy + 235, 345, 170, UBUNTU_CARD, UBUNTU_BORDER);
    draw_text(wx + 65, wy + 250, "COIL DIAGNOSTICS", UBUNTU_BLUE, 1);
    draw_text(wx + 65, wy + 285, "RESISTANCE : 0.60 OHM", UBUNTU_TEXT, 1);
    draw_text(wx + 65, wy + 315, "VOLTAGE    : 3.84 V", UBUNTU_MUTED, 1);
    draw_text(wx + 65, wy + 345, "CURRENT    : 6.40 A", UBUNTU_MUTED, 1);

    // Карточка платы
    draw_rounded_card(wx + 415, wy + 235, 345, 170, UBUNTU_CARD, UBUNTU_BORDER);
    draw_text(wx + 440, wy + 250, "PASITO HARDWARE", UBUNTU_GREEN, 1);
    draw_text(wx + 440, wy + 285, "KEY LOCK   : ", UBUNTU_MUTED, 1);
    draw_text(wx + 560, wy + 285, locked ? "ACTIVE" : "OFF", locked ? UBUNTU_YELLOW : UBUNTU_TEXT, 1);

    draw_text(wx + 440, wy + 315, "STEALTH    : ", UBUNTU_MUTED, 1);
    draw_text(wx + 560, wy + 315, stealth ? "ENABLED" : "DISABLED", UBUNTU_TEXT, 1);

    draw_text(wx + 440, wy + 345, "PUFF COUNT : ", UBUNTU_MUTED, 1);
    char p_buf[8];
    int_to_str(puffs_count, p_buf);
    draw_text(wx + 560, wy + 345, p_buf, UBUNTU_YELLOW, 1);

    // Секция статуса / парения
    if (is_firing) {
        draw_rounded_card(wx + 40, wy + 425, 720, 160, UBUNTU_RED, UBUNTU_YELLOW);
        draw_text(wx + 270, wy + 445, ">>> COIL HEATING ACTIVE <<<", UBUNTU_TEXT, 2);

        draw_text(wx + 290, wy + 485, "DURATION: ", UBUNTU_TEXT, 1);
        char s_buf[8];
        int_to_str(puff_ticks / 20, s_buf);
        draw_text(wx + 390, wy + 480, s_buf, UBUNTU_YELLOW, 2);
        draw_text(wx + 430, wy + 485, "S / 10.0S CUTOFF", UBUNTU_TEXT, 1);

        fill_rect(wx + 80, wy + 525, 640, 20, 0xFF700000);
        int cut_bar = (puff_ticks * 632) / 200;
        fill_rect(wx + 84, wy + 529, cut_bar, 12, UBUNTU_YELLOW);
    } else {
        draw_rounded_card(wx + 40, wy + 425, 720, 160, UBUNTU_CARD, UBUNTU_BORDER);
        draw_text(wx + 280, wy + 450, "STATUS: SYSTEM READY", UBUNTU_GREEN, 2);
        draw_text(wx + 80, wy + 490, "HOLD [FIRE]: VAPE  |  [^/v]: ADJUST WATTS", UBUNTU_TEXT, 1);
        draw_text(wx + 80, wy + 520, "3x [FIRE]: MODE  |  [^]+[v]: LOCK  |  [FIRE]+[^]: STEALTH", UBUNTU_MUTED, 1);
    }

    flip_screen();
}

void kernel_main(struct multiboot_info* mbi) {
    if (mbi && (mbi->flags & (1 << 12)) && mbi->framebuffer_addr) {
        front_fb = (uint32_t*)((uint32_t)mbi->framebuffer_addr);
    } else {
        front_fb = (uint32_t*)0xE0000000;
    }

    show_boot_animation();

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

            if (scancode == 0x39) {
                if (!fire_held) {
                    fire_held = 1;
                    fire_click_count++;
                    fire_click_timer = 30;

                    if (down_held) {
                        puffs_count = 0;
                        fire_click_count = 0;
                    } else if (up_held) {
                        stealth = !stealth;
                        fire_click_count = 0;
                    }
                }
            } else if (scancode == 0xB9) {
                fire_held = 0;
                is_firing = 0;
                puff_ticks = 0;
            } else if (scancode == 0x48) {
                up_held = 1;
                if (down_held && !is_firing) {
                    locked = !locked;
                } else if (!locked && !is_firing) {
                    if (watts < 80) watts++;
                }
            } else if (scancode == 0xC8) {
                up_held = 0;
            } else if (scancode == 0x50) {
                down_held = 1;
                if (up_held && !is_firing) {
                    locked = !locked;
                } else if (!locked && !is_firing) {
                    if (watts > 5) watts--;
                }
            } else if (scancode == 0xD0) {
                down_held = 0;
            }
        }

        if (fire_click_timer > 0) {
            fire_click_timer--;
            if (fire_click_count >= 3) {
                mode = (mode + 1) % 3;
                fire_click_count = 0;
                fire_click_timer = 0;
            }
        } else {
            if (fire_held && fire_click_count == 1 && !up_held && !down_held) {
                if (!is_firing) {
                    is_firing = 1;
                    puff_ticks = 0;
                    puffs_count++;
                }
            }
            fire_click_count = 0;
        }

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

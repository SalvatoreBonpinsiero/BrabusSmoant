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

// Разрешение экрана Pasito 3 (80x160) с масштабированием x4 для читаемости (320x640)
#define PASITO_REAL_W 320
#define PASITO_REAL_H 640
#define PASITO_ORIG_X ((SCREEN_W - PASITO_REAL_W) / 2)
#define PASITO_ORIG_Y ((SCREEN_H - PASITO_REAL_H) / 2)

static uint32_t* front_fb = 0;
static uint32_t  back_buffer[SCREEN_W * SCREEN_H];

#define C_UBUNTU_DARK   0xFF1E0616
#define C_UBUNTU_MID    0xFF3B102F
#define C_UBUNTU_ORANGE 0xFFE95420
#define C_UBUNTU_CARD   0xFF2B1C28
#define C_UBUNTU_TEXT   0xFFF7F7F7
#define C_UBUNTU_MUTED  0xFF9E8D9B
#define C_GREEN         0xFF38B44A
#define C_RED           0xFFE93224
#define C_YELLOW        0xFFF6D32D
#define C_BLUE          0xFF19B6EE

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Аппаратный таймер задержки через порт 0x61 (Real Hardware Wait)
void sleep_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms; i++) {
        for (volatile int j = 0; j < 3500; j++) {
            inb(0x61);
        }
    }
}

uint32_t blend_color(uint32_t fg, uint32_t bg, uint8_t alpha) {
    if (alpha == 0) return bg;
    if (alpha == 255) return fg;

    uint32_t r_fg = (fg >> 16) & 0xFF;
    uint32_t g_fg = (fg >> 8)  & 0xFF;
    uint32_t b_fg = fg & 0xFF;

    uint32_t r_bg = (bg >> 16) & 0xFF;
    uint32_t g_bg = (bg >> 8)  & 0xFF;
    uint32_t b_bg = bg & 0xFF;

    uint32_t r = (r_fg * alpha + r_bg * (255 - alpha)) / 255;
    uint32_t g = (g_fg * alpha + g_bg * (255 - alpha)) / 255;
    uint32_t b = (b_fg * alpha + b_bg * (255 - alpha)) / 255;

    return 0xFF000000 | (r << 16) | (g << 8) | b;
}

void put_pixel_alpha(int x, int y, uint32_t color, uint8_t alpha) {
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H) {
        uint32_t bg = back_buffer[y * SCREEN_W + x];
        back_buffer[y * SCREEN_W + x] = blend_color(color, bg, alpha);
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

void draw_smooth_card(int x, int y, int w, int h, uint32_t bg) {
    fill_rect(x + 3, y, w - 6, h, bg);
    fill_rect(x, y + 3, w, h - 6, bg);
    put_pixel_alpha(x + 1, y + 1, bg, 180);
    put_pixel_alpha(x + w - 2, y + 1, bg, 180);
    put_pixel_alpha(x + 1, y + h - 2, bg, 180);
    put_pixel_alpha(x + w - 2, y + h - 2, bg, 180);
}

void flip_screen(void) {
    for (int i = 0; i < SCREEN_W * SCREEN_H; i++) {
        front_fb[i] = back_buffer[i];
    }
}

// Сглаженная матрица шрифта 8x12 с субпиксельными уровнями интенсивности
const uint8_t aa_font[43][12] = {
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

// Рендеринг текста со сглаживанием контуров (Anti-Aliasing)
void draw_smooth_text(int x, int y, const char* str, uint32_t color, int scale) {
    while (*str) {
        int idx = char_idx(*str);
        for (int row = 0; row < 12; row++) {
            uint8_t line = aa_font[idx][row];
            for (int col = 0; col < 8; col++) {
                if (line & (0x80 >> col)) {
                    fill_rect(x + col * scale, y + row * scale, scale, scale, color);

                    // Мягкое субпиксельное сглаживание по краям
                    put_pixel_alpha(x + col * scale - 1, y + row * scale, color, 90);
                    put_pixel_alpha(x + col * scale + scale, y + row * scale, color, 90);
                    put_pixel_alpha(x + col * scale, y + row * scale - 1, color, 90);
                    put_pixel_alpha(x + col * scale, y + row * scale + scale, color, 90);
                }
            }
        }
        x += (8 + 2) * scale;
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

// Гарантированная анимация загрузки с аппаратным таймером
void show_boot_screen(void) {
    for (int frame = 0; frame <= 100; frame += 2) {
        // Очистка стола и области Pasito
        fill_rect(0, 0, SCREEN_W, SCREEN_H, 0xFF0D0D0D);
        fill_rect(PASITO_ORIG_X - 10, PASITO_ORIG_Y - 10, PASITO_REAL_W + 20, PASITO_REAL_H + 20, 0xFF242424);
        fill_rect(PASITO_ORIG_X, PASITO_ORIG_Y, PASITO_REAL_W, PASITO_REAL_H, C_UBUNTU_DARK);

        // Логотип
        draw_smooth_text(PASITO_ORIG_X + 45, PASITO_ORIG_Y + 180, "BRABUS", C_UBUNTU_ORANGE, 3);
        draw_smooth_text(PASITO_ORIG_X + 65, PASITO_ORIG_Y + 240, "PASITO III OS", C_UBUNTU_TEXT, 2);

        // Полоса загрузки
        draw_smooth_card(PASITO_ORIG_X + 30, PASITO_ORIG_Y + 330, 260, 16, C_UBUNTU_CARD);
        int bar_len = (frame * 252) / 100;
        if (bar_len > 0) {
            fill_rect(PASITO_ORIG_X + 34, PASITO_ORIG_Y + 334, bar_len, 8, C_UBUNTU_ORANGE);
        }

        // Анимированные точки
        int dot = (frame / 12) % 3;
        for (int d = 0; d < 3; d++) {
            uint32_t c = (d == dot) ? C_UBUNTU_ORANGE : C_UBUNTU_MUTED;
            fill_rect(PASITO_ORIG_X + 140 + (d * 16), PASITO_ORIG_Y + 370, 6, 6, c);
        }

        draw_smooth_text(PASITO_ORIG_X + 70, PASITO_ORIG_Y + 420, "INITIALIZING...", C_UBUNTU_MUTED, 1);

        flip_screen();
        sleep_ms(30);
    }
    sleep_ms(400);
}

void render_pasito_ui(int watts, int is_firing, int puff_ticks, int puffs_count, 
                      int mode, int locked, int stealth) {
    if (stealth && !is_firing) {
        fill_rect(0, 0, SCREEN_W, SCREEN_H, 0xFF000000);
        flip_screen();
        return;
    }

    fill_rect(0, 0, SCREEN_W, SCREEN_H, 0xFF111111);

    // Рамка корпуса мода вокруг экрана 80x160
    fill_rect(PASITO_ORIG_X - 12, PASITO_ORIG_Y - 12, PASITO_REAL_W + 24, PASITO_REAL_H + 24, 0xFF2A2A2A);
    fill_rect(PASITO_ORIG_X - 4, PASITO_ORIG_Y - 4, PASITO_REAL_W + 8, PASITO_REAL_H + 8, 0xFF141414);

    // Экран мода с градиентом
    for (int y = 0; y < PASITO_REAL_H; y++) {
        uint32_t clr = (y < 320) ? C_UBUNTU_MID : C_UBUNTU_DARK;
        fill_rect(PASITO_ORIG_X, PASITO_ORIG_Y + y, PASITO_REAL_W, 1, clr);
    }

    // Верхний трей
    fill_rect(PASITO_ORIG_X, PASITO_ORIG_Y, PASITO_REAL_W, 36, 0xEE1E0616);
    draw_smooth_text(PASITO_ORIG_X + 12, PASITO_ORIG_Y + 10, "BRABUS", C_UBUNTU_ORANGE, 1);

    if (locked) draw_smooth_text(PASITO_ORIG_X + 140, PASITO_ORIG_Y + 10, "LOCK", C_YELLOW, 1);
    draw_smooth_text(PASITO_ORIG_X + 225, PASITO_ORIG_Y + 10, "98%", C_GREEN, 1);

    if (is_firing) {
        // Режим затяжки
        draw_smooth_card(PASITO_ORIG_X + 15, PASITO_ORIG_Y + 60, 290, 550, C_RED);
        draw_smooth_text(PASITO_ORIG_X + 45, PASITO_ORIG_Y + 140, ">>> VAPING <<<", C_UBUNTU_TEXT, 2);

        char sec_buf[8];
        int_to_str(puff_ticks / 20, sec_buf);
        draw_smooth_text(PASITO_ORIG_X + 110, PASITO_ORIG_Y + 230, sec_buf, C_YELLOW, 6);
        draw_smooth_text(PASITO_ORIG_X + 190, PASITO_ORIG_Y + 265, "SEC", C_UBUNTU_TEXT, 2);

        draw_smooth_text(PASITO_ORIG_X + 75, PASITO_ORIG_Y + 360, "MAX CUTOFF: 10S", C_UBUNTU_TEXT, 1);

        // Индикатор отсечки
        fill_rect(PASITO_ORIG_X + 35, PASITO_ORIG_Y + 410, 250, 16, 0xFF6B0000);
        int bar = (puff_ticks * 242) / 200;
        fill_rect(PASITO_ORIG_X + 39, PASITO_ORIG_Y + 414, bar, 8, C_YELLOW);
    } else {
        // Режим ожидания
        const char* mode_labels[] = { "MODE: VW", "MODE: DVW", "MODE: BYPASS" };
        draw_smooth_text(PASITO_ORIG_X + 20, PASITO_ORIG_Y + 55, mode_labels[mode], C_BLUE, 1);

        // Ватты
        draw_smooth_card(PASITO_ORIG_X + 15, PASITO_ORIG_Y + 80, 290, 140, C_UBUNTU_CARD);
        char w_str[8];
        int_to_str(watts, w_str);
        int wx = (watts < 10) ? (PASITO_ORIG_X + 80) : (PASITO_ORIG_X + 45);
        draw_smooth_text(wx, PASITO_ORIG_Y + 110, w_str, C_UBUNTU_ORANGE, 5);
        draw_smooth_text(PASITO_ORIG_X + 195, PASITO_ORIG_Y + 135, "W", C_UBUNTU_ORANGE, 3);

        // Шкала ватт
        fill_rect(PASITO_ORIG_X + 35, PASITO_ORIG_Y + 185, 250, 12, 0xFF181018);
        int p_bar = (watts * 242) / 80;
        fill_rect(PASITO_ORIG_X + 39, PASITO_ORIG_Y + 189, p_bar, 4, C_UBUNTU_ORANGE);

        // Карточка параметров атомайзера
        draw_smooth_card(PASITO_ORIG_X + 15, PASITO_ORIG_Y + 235, 290, 175, C_UBUNTU_CARD);
        draw_smooth_text(PASITO_ORIG_X + 30, PASITO_ORIG_Y + 250, "RESISTANCE: 0.60 OHM", C_UBUNTU_TEXT, 1);
        draw_smooth_text(PASITO_ORIG_X + 30, PASITO_ORIG_Y + 285, "VOLTAGE   : 3.84 V", C_UBUNTU_MUTED, 1);
        draw_smooth_text(PASITO_ORIG_X + 30, PASITO_ORIG_Y + 320, "CURRENT   : 6.40 A", C_UBUNTU_MUTED, 1);
        draw_smooth_text(PASITO_ORIG_X + 30, PASITO_ORIG_Y + 355, "TEMP CHIP : 31.4 C", C_GREEN, 1);

        // Статистика
        draw_smooth_card(PASITO_ORIG_X + 15, PASITO_ORIG_Y + 425, 290, 100, C_UBUNTU_CARD);
        draw_smooth_text(PASITO_ORIG_X + 30, PASITO_ORIG_Y + 445, "PUFFS: ", C_UBUNTU_MUTED, 1);
        char p_str[8];
        int_to_str(puffs_count, p_str);
        draw_smooth_text(PASITO_ORIG_X + 120, PASITO_ORIG_Y + 445, p_str, C_YELLOW, 2);

        draw_smooth_text(PASITO_ORIG_X + 30, PASITO_ORIG_Y + 485, "ANT-CHIP 3.0 READY", C_GREEN, 1);

        // Подсказки кнопок мода
        draw_smooth_text(PASITO_ORIG_X + 35, PASITO_ORIG_Y + 545, "HOLD [FIRE] TO VAPE", C_UBUNTU_TEXT, 1);
        draw_smooth_text(PASITO_ORIG_X + 25, PASITO_ORIG_Y + 575, "3xFIRE:MODE | ^+v:LOCK", C_UBUNTU_MUTED, 1);
    }

    flip_screen();
}

void kernel_main(struct multiboot_info* mbi) {
    if (mbi && (mbi->flags & (1 << 12)) && mbi->framebuffer_addr) {
        front_fb = (uint32_t*)((uint32_t)mbi->framebuffer_addr);
    } else {
        front_fb = (uint32_t*)0xE0000000;
    }

    show_boot_screen();

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

    render_pasito_ui(watts, is_firing, puff_ticks, puffs_count, mode, locked, stealth);

    while (1) {
        if (inb(0x64) & 0x01) {
            uint8_t scancode = inb(0x60);

            if (scancode == 0x39) {
                if (!fire_held) {
                    fire_held = 1;
                    fire_click_count++;
                    fire_click_timer = 25;

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

        render_pasito_ui(watts, is_firing, puff_ticks, puffs_count, mode, locked, stealth);
        sleep_ms(16);
    }
}

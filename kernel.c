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

static uint32_t* fb = 0;
static uint32_t fb_pitch = 0;
static uint32_t fb_w = 1024;
static uint32_t fb_h = 768;

#define PASITO_W 80
#define PASITO_H 160
#define SCALE    3

#define DISP_W (PASITO_W * SCALE)
#define DISP_H (PASITO_H * SCALE)
#define DISP_X ((1024 - DISP_W) / 2)
#define DISP_Y ((768 - DISP_H) / 2)

#define C_BODY_METAL   0xFF2B2B2B
#define C_BODY_EDGE    0xFF404040
#define C_CARBON       0xFF1A1A1A
#define C_SCREEN_BEZEL 0xFF0D0D0D

#define C_UBUNTU_BG    0xFF300A24
#define C_UBUNTU_BAR   0xFF1E0616
#define C_ORANGE       0xFFE95420
#define C_WHITE        0xFFFFFFFF
#define C_GRAY         0xFFAAAAAA
#define C_DARK_GRAY    0xFF555555
#define C_GREEN        0xFF38B44A
#define C_RED          0xFFDF382C
#define C_YELLOW       0xFFF6D32D
#define C_CYAN         0xFF19B6EE

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void fill_rect(int x, int y, int w, int h, uint32_t color) {
    for (int j = y; j < y + h; j++) {
        if (j < 0 || j >= (int)fb_h) continue;
        uint32_t* row = &fb[j * (fb_pitch / 4)];
        for (int i = x; i < x + w; i++) {
            if (i >= 0 && i < (int)fb_w) {
                row[i] = color;
            }
        }
    }
}

void p_pixel(int x, int y, uint32_t color) {
    if (x >= 0 && x < PASITO_W && y >= 0 && y < PASITO_H) {
        fill_rect(DISP_X + x * SCALE, DISP_Y + y * SCALE, SCALE, SCALE, color);
    }
}

void p_rect(int x, int y, int w, int h, uint32_t color) {
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

void p_char(int x, int y, char c, uint32_t color, int scale) {
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

void p_string(int x, int y, const char* str, uint32_t color, int scale) {
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

void render_device_shell(void) {
    fill_rect(0, 0, fb_w, fb_h, 0xFF141414);

    int body_x = DISP_X - 60;
    int body_y = DISP_Y - 90;
    int body_w = DISP_W + 120;
    int body_h = DISP_H + 180;

    fill_rect(body_x, body_y, body_w, body_h, C_BODY_METAL);
    fill_rect(body_x + 4, body_y + 4, body_w - 8, body_h - 8, C_CARBON);
    fill_rect(body_x + 14, body_y + 14, body_w - 28, body_h - 28, C_BODY_METAL);

    fill_rect(DISP_X + (DISP_W / 2) - 40, body_y + 25, 80, 40, C_BODY_EDGE);
    fill_rect(DISP_X + (DISP_W / 2) - 36, body_y + 29, 72, 32, C_CARBON);

    fill_rect(DISP_X - 8, DISP_Y - 8, DISP_W + 16, DISP_H + 16, C_SCREEN_BEZEL);

    fill_rect(DISP_X + 20, DISP_Y + DISP_H + 25, 60, 24, C_BODY_EDGE);
    fill_rect(DISP_X + DISP_W - 80, DISP_Y + DISP_H + 25, 60, 24, C_BODY_EDGE);
}

void render_pasito_display(int watts, int is_firing, int puff_ticks, int puffs_count) {
    for (int y = 0; y < PASITO_H; y++) {
        uint32_t clr = (y < 80) ? C_UBUNTU_BG : 0xFF1E0616;
        p_rect(0, y, PASITO_W, 1, clr);
    }

    p_rect(0, 0, PASITO_W, 12, C_UBUNTU_BAR);
    p_string(3, 3, "BRABUS", C_ORANGE, 1);
    
    p_rect(58, 4, 16, 5, C_DARK_GRAY);
    p_rect(59, 5, 12, 3, C_GREEN);
    p_pixel(74, 5, C_DARK_GRAY);

    p_rect(0, 12, PASITO_W, 1, C_ORANGE);

    if (is_firing) {
        p_rect(4, 18, 72, 136, C_RED);
        p_rect(6, 20, 68, 132, 0xFF600B0B);

        p_string(14, 30, "VAPING", C_WHITE, 2);
        p_string(16, 50, "ACTIVE", C_ORANGE, 2);

        char s_buf[8];
        int_to_str(puff_ticks / 20, s_buf);
        p_string(28, 80, s_buf, C_YELLOW, 3);
        p_string(48, 92, "S", C_WHITE, 1);

        p_string(12, 115, "MAX 10.0S", C_GRAY, 1);

        p_rect(8, 135, 64, 8, C_SCREEN_BEZEL);
        int bar_w = (puff_ticks * 60) / 200;
        p_rect(10, 137, bar_w, 4, C_YELLOW);
    } else {
        p_string(6, 18, "MODE: SPORT", C_CYAN, 1);

        char w_buf[8];
        int_to_str(watts, w_buf);
        int w_x = (watts < 10) ? 22 : 12;
        p_string(w_x, 30, w_buf, C_WHITE, 4);
        p_string(54, 48, "W", C_ORANGE, 2);

        p_rect(4, 66, 72, 5, C_SCREEN_BEZEL);
        int p_bar = (watts * 68) / 80;
        p_rect(6, 67, p_bar, 3, C_ORANGE);

        p_rect(4, 76, 72, 44, 0x881E0616);
        p_rect(4, 76, 72, 1, C_DARK_GRAY);

        p_string(8, 81, "RES : 0.60 O", C_WHITE, 1);
        p_string(8, 93, "VOLT: 3.84 V", C_GRAY, 1);
        p_string(8, 105, "AMP : 6.40 A", C_GRAY, 1);

        p_rect(4, 124, 72, 30, 0x881E0616);
        p_rect(4, 124, 72, 1, C_DARK_GRAY);

        p_string(8, 128, "PUFF: ", C_GRAY, 1);
        char p_buf[8];
        int_to_str(puffs_count, p_buf);
        p_string(38, 128, p_buf, C_YELLOW, 1);

        p_string(8, 140, "STATUS: READY", C_GREEN, 1);
    }
}

void kernel_main(struct multiboot_info* mbi) {
    if (mbi && (mbi->flags & (1 << 12)) && mbi->framebuffer_addr) {
        fb = (uint32_t*)((uint32_t)mbi->framebuffer_addr);
        fb_pitch = mbi->framebuffer_pitch;
        fb_w = mbi->framebuffer_width;
        fb_h = mbi->framebuffer_height;
    } else {
        fb = (uint32_t*)0xE0000000;
        fb_pitch = 1024 * 4;
        fb_w = 1024;
        fb_h = 768;
    }

    render_device_shell();

    int watts = 35;
    int is_firing = 0;
    int puff_ticks = 0;
    int puffs_count = 0;
    int space_held = 0;

    render_pasito_display(watts, is_firing, puff_ticks, puffs_count);

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

        render_pasito_display(watts, is_firing, puff_ticks, puffs_count);
        for (volatile int d = 0; d < 30000; d++);
    }
}

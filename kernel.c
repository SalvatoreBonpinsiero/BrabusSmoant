typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

volatile uint16_t* vga_buffer = (volatile uint16_t*)0xB8000;

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline uint16_t vga_entry(char ch, uint8_t color) {
    return (uint16_t)ch | ((uint16_t)color << 8);
}

void clear_screen(uint8_t color) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = vga_entry(' ', color);
    }
}

void draw_text(int x, int y, const char* str, uint8_t color) {
    int offset = y * VGA_WIDTH + x;
    while (*str) {
        vga_buffer[offset++] = vga_entry(*str++, color);
    }
}

void draw_rect(int x, int y, int w, int h, uint8_t color) {
    for (int i = y; i < y + h; i++) {
        for (int j = x; j < x + w; j++) {
            vga_buffer[i * VGA_WIDTH + j] = vga_entry(' ', color);
        }
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

void render_ui(int watts, int is_firing, int puff_ticks, int puffs_count) {
    draw_rect(18, 1, 44, 23, 0x55);
    draw_rect(20, 2, 40, 21, 0x05);

    draw_text(34, 3, "[ BRABUS ]", 0x5F);
    draw_text(26, 4, "PASITO III SPORT EDITION", 0x5D);
    draw_text(22, 5, "====================================", 0x55);

    draw_text(22, 7,  "CHIP    : ANT-CHIP GEN 3", 0x5F);
    draw_text(22, 8,  "MCU     : 32-BIT RISC CORTEX", 0x5D);
    draw_text(22, 9,  "BATTERY : 2500 MAH [4.12V] 94%", 0x5F);
    draw_text(22, 10, "COIL RES: 0.60 OHM (MESH DTL)", 0x5D);

    draw_text(22, 12, "POWER   : ", 0x5F);
    char w_str[8];
    int_to_str(watts, w_str);
    draw_text(32, 12, w_str, 0x5E);
    draw_text(35, 12, "WATTS", 0x5E);

    draw_text(22, 13, "VOLTAGE : 3.87 V", 0x5D);
    draw_text(22, 14, "CURRENT : 6.45 A", 0x5D);

    draw_text(22, 16, "PUFFS   : ", 0x5F);
    char p_str[8];
    int_to_str(puffs_count, p_str);
    draw_text(32, 16, p_str, 0x5F);

    draw_text(22, 17, "LIMIT   : 10.0 SEC CUTOFF", 0x55);
    draw_text(22, 18, "------------------------------------", 0x55);

    if (is_firing) {
        draw_rect(22, 19, 36, 3, 0x5E);
        draw_text(35, 19, ">> FIRING <<", 0x5E);
        draw_text(35, 20, "TIME: ", 0x5F);
        char sec_str[8];
        int_to_str(puff_ticks / 20, sec_str);
        draw_text(41, 20, sec_str, 0x5F);
        draw_text(43, 20, "S", 0x5F);
    } else {
        draw_rect(22, 19, 36, 3, 0x05);
        draw_text(34, 20, "[ STANDBY MODE ]", 0x5D);
    }
}

void kernel_main(void) {
    clear_screen(0x50);

    int watts = 25;
    int is_firing = 0;
    int puff_ticks = 0;
    int puffs_count = 0;
    int space_held = 0;

    render_ui(watts, is_firing, puff_ticks, puffs_count);

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

        render_ui(watts, is_firing, puff_ticks, puffs_count);

        for (volatile int d = 0; d < 40000; d++);
    }
}

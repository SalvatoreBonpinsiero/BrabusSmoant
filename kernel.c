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

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint16_t vga_entry(char ch, uint8_t color) {
    return (uint16_t)(uint8_t)ch | ((uint16_t)color << 8);
}

void delay(volatile uint32_t count) {
    while (count--) {
        __asm__ volatile ("nop");
    }
}

void sound_on(uint32_t freq) {
    uint32_t div = 1193180 / freq;
    outb(0x43, 0xB6);
    outb(0x42, (uint8_t)(div & 0xFF));
    outb(0x42, (uint8_t)((div >> 8) & 0xFF));
    uint8_t tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }
}

void sound_off(void) {
    uint8_t tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}

void clear_screen(uint8_t color) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = vga_entry(' ', color);
    }
}

void draw_char(int x, int y, char ch, uint8_t color) {
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        vga_buffer[y * VGA_WIDTH + x] = vga_entry(ch, color);
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

void draw_double_box(int x, int y, int w, int h, uint8_t color) {
    draw_char(x, y, (char)201, color);
    draw_char(x + w - 1, y, (char)187, color);
    draw_char(x, y + h - 1, (char)200, color);
    draw_char(x + w - 1, y + h - 1, (char)188, color);

    for (int i = x + 1; i < x + w - 1; i++) {
        draw_char(i, y, (char)205, color);
        draw_char(i, y + h - 1, (char)205, color);
    }
    for (int j = y + 1; j < y + h - 1; j++) {
        draw_char(x, j, (char)186, color);
        draw_char(x + w - 1, j, (char)186, color);
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

void draw_battery(int x, int y, int percent) {
    draw_char(x, y, '[', 0x05);
    int bars = (percent * 6) / 100;
    for (int i = 0; i < 6; i++) {
        if (i < bars) {
            draw_char(x + 1 + i, y, (char)219, (percent > 20) ? 0x0A : 0x0C);
        } else {
            draw_char(x + 1 + i, y, (char)176, 0x08);
        }
    }
    draw_char(x + 7, y, ']', 0x05);
    draw_char(x + 8, y, (char)243, 0x0A);
}

void show_boot_animation(void) {
    clear_screen(0x00);

    for (int step = 0; step < 4; step++) {
        uint8_t clr = (step % 2 == 0) ? 0x05 : 0x0D;
        draw_text(22, 5, " ____  ____    _    ____  _   _ ____  ", clr);
        draw_text(22, 6, "| __ )|  _ \\  / \\  | __ )| | | / ___| ", clr);
        draw_text(22, 7, "|  _ \\| |_) |/ _ \\ |  _ \\| | | \\___ \\ ", 0x0F);
        draw_text(22, 8, "| |_) |  _ </ ___ \\| |_) | |_| |___) |", 0x0D);
        draw_text(22, 9, "|____/|_| \\_/_/   \\_\\____/ \\___/|____/ ", 0x05);
        delay(4000000);
    }

    draw_text(29, 11, "-= PASITO III SPORT OS =-", 0x5F);
    draw_double_box(20, 14, 40, 3, 0x05);

    const char* tasks[] = {
        "INIT SYSTEM BUS...",
        "POLLING I2C OLED DISPLAY...",
        "CALIBRATING SHUNT RESISTOR...",
        "MOUNTING SPI FLASH STORAGE...",
        "READY TO VAPE!"
    };

    for (int p = 0; p <= 36; p++) {
        draw_char(22 + p, 15, (char)219, 0x0D);

        int task_id = (p * 5) / 37;
        draw_rect(21, 18, 38, 1, 0x00);
        draw_text(22, 18, tasks[task_id], 0x0B);

        delay(1200000);
    }
    delay(5000000);
}

void render_dashboard(int watts, int is_firing, int puff_ticks, int puffs_count, int mode_idx) {
    clear_screen(0x05);

    draw_rect(12, 0, 56, 25, 0x00);
    draw_double_box(12, 0, 56, 25, 0x05);

    draw_text(15, 1, "[B]", 0x5E);
    draw_text(19, 1, "BRABUS ANT-GEN3", 0x0F);
    draw_char(43, 1, (char)13, 0x0E);
    draw_battery(45, 1, 95);
    draw_text(55, 1, "95%", 0x0A);
    draw_text(60, 1, "[LOCKED]", 0x08);

    for (int i = 13; i < 67; i++) {
        draw_char(i, 2, (char)205, 0x05);
    }

    draw_double_box(14, 4, 30, 8, 0x0D);
    draw_text(16, 4, "[ POWER SPEEDO ]", 0x0F);

    char w_buf[8];
    int_to_str(watts, w_buf);
    draw_text(17, 6, w_buf, 0x0E);
    draw_text(23, 6, "WATTS", 0x0D);

    const char* modes[] = { "SPORT ", "ECO   ", "CUSTOM", "BYPASS" };
    draw_text(17, 8, "MODE:", 0x07);
    draw_text(23, 8, modes[mode_idx], 0x0B);

    int scale_fill = (watts * 24) / 80;
    for (int s = 0; s < 24; s++) {
        char bar_sym = (s < scale_fill) ? (char)219 : (char)176;
        uint8_t bar_clr = (s > 18) ? 0x0C : ((s > 10) ? 0x0E : 0x0A);
        draw_char(16 + s, 10, bar_sym, bar_clr);
    }

    draw_double_box(46, 4, 20, 8, 0x05);
    draw_text(48, 4, "[ METRICS ]", 0x0F);

    draw_char(48, 6, (char)234, 0x0B);
    draw_text(50, 6, "0.60 OHM", 0x0F);

    draw_char(48, 7, (char)244, 0x0E);
    draw_text(50, 7, "4.15 VOLT", 0x07);

    draw_char(48, 8, (char)227, 0x0C);
    draw_text(50, 8, "7.20 AMP", 0x07);

    draw_char(48, 9, (char)127, 0x0D);
    draw_text(50, 9, "PUFF:", 0x07);
    char p_buf[8];
    int_to_str(puffs_count, p_buf);
    draw_text(56, 9, p_buf, 0x0E);

    for (int i = 13; i < 67; i++) {
        draw_char(i, 13, (char)196, 0x05);
    }

    if (is_firing) {
        draw_rect(14, 15, 52, 6, 0x40);
        draw_double_box(14, 15, 52, 6, 0x4E);

        draw_char(22, 16, (char)15, 0x4E);
        draw_text(24, 16, ">>> S P O R T   F I R I N G <<<", 0x4F);
        draw_char(55, 16, (char)15, 0x4E);

        draw_text(18, 18, "TIMER:", 0x4F);
        char sec_buf[8];
        int_to_str(puff_ticks / 20, sec_buf);
        draw_text(25, 18, sec_buf, 0x4E);
        draw_text(27, 18, "S / 10.0S", 0x4F);

        int cutoff_progress = (puff_ticks * 46) / 200;
        for (int b = 0; b < 46; b++) {
            draw_char(17 + b, 19, (b < cutoff_progress) ? (char)219 : (char)177, 0x4C);
        }
    } else {
        draw_rect(14, 15, 52, 6, 0x00);
        draw_double_box(14, 15, 52, 6, 0x08);

        draw_text(28, 17, "[ COIL STATUS: READY ]", 0x0A);
        draw_text(22, 19, "SMOANT SMART ANT PROTECTION ACTIVE", 0x05);
    }

    draw_rect(13, 22, 54, 2, 0x05);
    draw_text(14, 22, "SPACE: VAPE | ^/v: WATTS | TAB: MODE", 0x5F);
    draw_text(14, 23, "CUTOFF: 10 SEC | TEMP CHIP: 31.4 C", 0x5D);
}

void kernel_main(void) {
    show_boot_animation();

    int watts = 35;
    int is_firing = 0;
    int puff_ticks = 0;
    int puffs_count = 0;
    int space_held = 0;
    int mode_idx = 0;

    render_dashboard(watts, is_firing, puff_ticks, puffs_count, mode_idx);

    while (1) {
        if (inb(0x64) & 0x01) {
            uint8_t scancode = inb(0x60);

            if (scancode == 0x39) {
                if (!space_held) {
                    space_held = 1;
                    is_firing = 1;
                    puff_ticks = 0;
                    puffs_count++;
                    sound_on(180);
                }
            } else if (scancode == 0xB9) {
                space_held = 0;
                is_firing = 0;
                puff_ticks = 0;
                sound_off();
            } else if (scancode == 0x48 && !is_firing) {
                if (watts < 80) watts++;
            } else if (scancode == 0x50 && !is_firing) {
                if (watts > 5) watts--;
            } else if (scancode == 0x0F && !is_firing) {
                mode_idx = (mode_idx + 1) % 4;
            }
        }

        if (is_firing) {
            puff_ticks++;
            if (puff_ticks > 200) {
                is_firing = 0;
                sound_off();
            }
        }

        render_dashboard(watts, is_firing, puff_ticks, puffs_count, mode_idx);
        for (volatile int d = 0; d < 30000; d++);
    }
}

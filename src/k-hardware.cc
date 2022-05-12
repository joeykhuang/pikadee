#include "utils.hh"
#include "k-cpu.hh"
#include "printf.hh"
#include "rpi3peripherals.hh"

volatile unsigned int __attribute__((aligned(16))) mbox[36];

void uart_send(char c)
{
    while (1)
    {
        if (get32(AUX_MU_LSR_REG) & 0x20)
            break;
    }
    put32(AUX_MU_IO_REG, c);
}

char uart_recv(void)
{
    while (1)
    {
        if (get32(AUX_MU_LSR_REG) & 0x01)
            break;
    }
    return (get32(AUX_MU_IO_REG) & 0xFF);
}

void uart_send_string(char *str)
{
    for (int i = 0; str[i] != '\0'; i++)
    {
        uart_send((char)str[i]);
    }
}

// This function is required by printf function
void putc(void *p, char c)
{
    uart_send(c);
}

const unsigned int interval = 200000;
unsigned int curVal = 0;

void init_timer(void)
{
    curVal = get32(TIMER_CLO);
    curVal += interval;
    put32(TIMER_C1, curVal);
}

void handle_timer_irq(void)
{
    curVal += interval;
    put32(TIMER_C1, curVal);
    put32(TIMER_CS, TIMER_CS_M1);
    timer_tick();
}

int mbox_call(unsigned char ch)
{
    unsigned int r = (((unsigned int)((unsigned long)&mbox) & ~0xF) | (ch & 0xF));
    /* wait until we can write to the mailbox */
    do
    {
        asm volatile("nop");
    } while (get32(MBOX_STATUS) & MBOX_FULL);
    /* write the address of our message to the mailbox with channel identifier */
    put32(MBOX_WRITE, r);
    /* now wait for the response */
    while (1)
    {
        /* is there a response? */
        do
        {
            asm volatile("nop");
        } while (get32(MBOX_STATUS) & MBOX_EMPTY);
        /* is it a response to our message? */
        if (r == get32(MBOX_READ))
            /* is it a valid successful response? */
            return mbox[1] == MBOX_RESPONSE;
    }
    return 0;
}

typedef struct
{
    unsigned int magic;
    unsigned int version;
    unsigned int headersize;
    unsigned int flags;
    unsigned int numglyph;
    unsigned int bytesperglyph;
    unsigned int height;
    unsigned int width;
    unsigned char glyphs;
} __attribute__((packed)) psf_t;
extern volatile unsigned char _binary_src_font_psf_start;

unsigned int width, height, pitch;
unsigned char *console;

/**
 * Set screen resolution
 */
void init_console()
{
    mbox[0] = 35 * 4;
    mbox[1] = MBOX_REQUEST;

    mbox[2] = 0x48003; // set phy wh
    mbox[3] = 8;
    mbox[4] = 8;
    mbox[5] = CONSOLE_COLUMNS; // FrameBufferInfo.width
    mbox[6] = CONSOLE_ROWS;    // FrameBufferInfo.height

    mbox[7] = 0x48004; // set virt wh
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = CONSOLE_COLUMNS; // FrameBufferInfo.virtual_width
    mbox[11] = CONSOLE_ROWS;    // FrameBufferInfo.virtual_height

    mbox[12] = 0x48009; // set virt offset
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0; // FrameBufferInfo.x_offset
    mbox[16] = 0; // FrameBufferInfo.y.offset

    mbox[17] = 0x48005; // set depth
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32; // FrameBufferInfo.depth

    mbox[21] = 0x48006; // set pixel order
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 1; // RGB, not BGR preferably

    mbox[25] = 0x40001; // get framebuffer, gets alignment on request
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = 4096; // FrameBufferInfo.pointer
    mbox[29] = 0;    // FrameBufferInfo.size

    mbox[30] = 0x40008; // get pitch
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0; // FrameBufferInfo.pitch

    mbox[34] = MBOX_TAG_LAST;

    if (mbox_call(MBOX_CH_PROP) && mbox[20] == 32 && mbox[28] != 0)
    {
        mbox[28] &= 0x3FFFFFFF;
        width = mbox[5];
        height = mbox[6];
        pitch = mbox[33];
        console = reinterpret_cast<unsigned char *>((unsigned long)mbox[28]) + VA_START;
    }
    else
    {
        printf("Unable to set screen resolution\n");
    }
}

/**
 * Display a string using fixed size PSF
 */
void console_print(int x, int y, char *s, uint32_t fg, uint32_t bg)
{
    // get our font
    psf_t *font = (psf_t *)&_binary_src_font_psf_start;
    // draw next character if it's not zero
    while (*s)
    {
        // get the offset of the glyph. Need to adjust this to support unicode table
        unsigned char *glyph = (unsigned char *)&_binary_src_font_psf_start +
                               font->headersize + (*((unsigned char *)s) < font->numglyph ? *s : 0) * font->bytesperglyph;
        // calculate the offset on screen
        int offs = (y * pitch) + (x * 4);
        // variables
        unsigned int i, j, line, mask, bytesperline = (font->width + 7) / 8;
        // handle carrige return
        if (*s == '\r')
        {
            x = 0;
        }
        else
            // new line
            if (*s == '\n')
            {
                x = 0;
                y += font->height;
            }
            else
            {
                // display a character
                for (j = 0; j < font->height; j++)
                {
                    // display one row
                    line = offs;
                    mask = 1 << (font->width - 1);
                    for (i = 0; i < font->width; i++)
                    {
                        // if bit set, we use white color, otherwise black
                        *((unsigned int *)(console + line)) = ((int)*glyph) & mask ? fg: bg;
                        mask >>= 1;
                        line += 4;
                    }
                    // adjust to next line
                    glyph += bytesperline;
                    offs += pitch;
                }
                x += (font->width + 1);
            }
        // next character
        s++;
    }
}
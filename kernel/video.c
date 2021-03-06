#include "memorypool.h"
#include "video.h"
#include "../memorymap.h"
#include "utils.h"

extern char* kernelAllocPages(uint64_t pageCount);
extern void memcpy64(char* source, char* dest, uint64_t size);
extern void memclear64(char* dest, uint64_t size);
extern void mutexLock(uint64_t*);
extern void mutexUnlock(uint64_t*);
extern void rwlockWriteLock(uint64_t*);
extern void rwlockWriteUnlock(uint64_t*);
extern void rwlockReadLock(uint64_t*);
extern void rwlockReadUnlock(uint64_t*);
extern void mapPhysOnVirtualAddressSpace(uint64_t pageTableBase, uint64_t virtualAddress, uint64_t physicalAddress, uint64_t pageCount, uint64_t mask);

#define MAX_SCREEN_COUNT 128

volatile char* current_video_back_buffer;
static uint64_t memorypool;


void enable_cursor(bool enabled)
{
    char tmp;
    OUTPORTB(0x0A,0x3D4);
    INPORTB(tmp,0x3D5)
    if (enabled)
    {
        tmp = (tmp & 0b11111);
    }
    else
    {
        tmp = (tmp | 0b100000);
    }
    OUTPORTB(0x3D4, 0x0A);
    OUTPORTB(tmp,0x3D5);
}

void video_update_cursor(Screen* scr, bool on, uint16_t position)
{
    if (!scr->active) return;
    enable_cursor(on);
    if (on)
    {
        OUTPORTB(0x0F,0x3D4);
        OUTPORTB((unsigned char)position, 0x3D5);
        OUTPORTB(0x0E, 0x3D4);
        OUTPORTB((unsigned char )(position>>8), 0x3D5);
    }
}

void video_get_dimensions(Screen* src, uint32_t* width, uint32_t* height)
{
    *width=80;
    *height=25;
}

void video_init()
{
    uint64_t i;

    memorypool = create_memory_pool(sizeof(Screen));
    enable_cursor(false);
}

Screen* video_create_screen()
{
    uint64_t i;
    Screen* screen = (Screen*)reserve_object(memorypool);
    memclear64((void*)screen,sizeof(Screen));

    screen->backBuffer = kernelAllocPages(1);
    screen->active = false;

    memclear64((void*)screen->backBuffer,4096);
    return screen;
}

char* video_get_buffer(Screen* scr)
{
    return scr->backBuffer;
}

void video_change_active_screen(Screen* oldscr, Screen* newscr)
{
    if (oldscr)
    {
        oldscr->active = false;
    }
    newscr->active = true;
    current_video_back_buffer = newscr->backBuffer;
}

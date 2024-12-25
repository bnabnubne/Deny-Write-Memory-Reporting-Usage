#include "threads/palloc.h"
#include <bitmap.h>
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "threads/loader.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "devices/timer.h"

// Global variables for tracking memory usage
static size_t total_pages = 0;  // Total pages available
static size_t free_pages = 0;   // Free pages available

// Returns the total number of pages
size_t palloc_get_total_pages(void) {
    return total_pages;
}

// Returns the number of free pages
size_t palloc_get_free_pages(void) {
    return free_pages;
}

/* Memory pool structure */
struct pool {
    struct lock lock;                   /* Mutual exclusion */
    struct bitmap *used_map;            /* Bitmap of free pages */
    uint8_t *base;                      /* Base of pool */
};

// Pools for kernel and user memory
static struct pool kernel_pool, user_pool;

// Function prototypes
static void init_pool(struct pool *p, void *base, size_t page_cnt, const char *name);
static bool page_from_pool(const struct pool *pool, void *page);

/* Initializes the page allocator. */
void palloc_init(size_t user_page_limit) {
    // Define the memory range
    uint8_t *free_start = ptov(1024 * 1024);
    uint8_t *free_end = ptov(init_ram_pages * PGSIZE);

    // Calculate total pages and allocate half for kernel and user pools
    size_t total_free_pages = (free_end - free_start) / PGSIZE;
    size_t user_pages = total_free_pages / 2;
    if (user_pages > user_page_limit) user_pages = user_page_limit;
    size_t kernel_pages = total_free_pages - user_pages;

    total_pages = total_free_pages;
    free_pages = total_free_pages;

    // Initialize pools
    init_pool(&kernel_pool, free_start, kernel_pages, "kernel pool");
    init_pool(&user_pool, free_start + kernel_pages * PGSIZE, user_pages, "user pool");
}

/* Allocates a contiguous group of pages */
void *palloc_get_multiple(enum palloc_flags flags, size_t page_cnt) {
    struct pool *pool = (flags & PAL_USER) ? &user_pool : &kernel_pool;
    void *pages;
    size_t page_idx;

    if (page_cnt == 0) return NULL;

    lock_acquire(&pool->lock);
    page_idx = bitmap_scan_and_flip(pool->used_map, 0, page_cnt, false);
    lock_release(&pool->lock);

    if (page_idx != BITMAP_ERROR) {
        pages = pool->base + PGSIZE * page_idx;
        free_pages -= page_cnt;  // Update free pages
        if (flags & PAL_ZERO) memset(pages, 0, PGSIZE * page_cnt);
    } else {
        pages = NULL;
        if (flags & PAL_ASSERT) PANIC("palloc_get: out of pages");
    }

    return pages;
}

/* Allocates a single page */
void *palloc_get_page(enum palloc_flags flags) {
    return palloc_get_multiple(flags, 1);
}

/* Frees a group of pages */
void palloc_free_multiple(void *pages, size_t page_cnt) {
    struct pool *pool;
    size_t page_idx;

    ASSERT(pg_ofs(pages) == 0);
    if (pages == NULL || page_cnt == 0) return;

    if (page_from_pool(&kernel_pool, pages)) {
        pool = &kernel_pool;
    } else if (page_from_pool(&user_pool, pages)) {
        pool = &user_pool;
    } else {
        NOT_REACHED();
    }

    page_idx = pg_no(pages) - pg_no(pool->base);
#ifndef NDEBUG
    memset(pages, 0xcc, PGSIZE * page_cnt);
#endif
    bitmap_set_multiple(pool->used_map, page_idx, page_cnt, false);
    free_pages += page_cnt;  // Update free pages
}

/* Frees a single page */
void palloc_free_page(void *page) {
    palloc_free_multiple(page, 1);
}

/* Initializes a memory pool */
static void init_pool(struct pool *p, void *base, size_t page_cnt, const char *name) {
    size_t bm_pages = DIV_ROUND_UP(bitmap_buf_size(page_cnt), PGSIZE);
    if (bm_pages > page_cnt) PANIC("Not enough memory in %s for bitmap.", name);

    page_cnt -= bm_pages;
    printf("%zu pages available in %s.\n", page_cnt, name);

    lock_init(&p->lock);
    p->used_map = bitmap_create_in_buf(page_cnt, base, bm_pages * PGSIZE);
    p->base = base + bm_pages * PGSIZE;
}

/* Checks if a page belongs to a specific pool */
static bool page_from_pool(const struct pool *pool, void *page) {
    size_t page_no = pg_no(page);
    size_t start_page = pg_no(pool->base);
    size_t end_page = start_page + bitmap_size(pool->used_map);

    return page_no >= start_page && page_no < end_page;
}

/* Reports memory usage */
void report_memory_usage(void) {
    printf("Total pages: %zu\n", total_pages);
    printf("Free pages: %zu\n", free_pages);
    printf("Used pages: %zu\n", total_pages - free_pages);
}


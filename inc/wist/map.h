/* === inc/wist/map.h - Generic map === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_MAP_H
#define _WIST_MAP_H

#include <wist/defs.h>
#include <wist/sym.h>
#include <wist/ctx.h>

#define INIT_BUCKETS_LEN 16

typedef bool (*wist_map_eq_fn)(void *k1, void *k2);
typedef uint32_t (*wist_map_hash_fn)(void *k1);

struct wist_map_entry {
    struct wist_map_entry *next;
    uint8_t data[];
};

struct wist_map {
    wist_map_eq_fn key_eq;
    wist_map_hash_fn key_hash;
    size_t key_size, val_size;
    size_t buckets_len, buckets_filled;
    struct wist_map_entry **buckets;
};

static void wist_map_finish(struct wist_ctx *ctx, struct wist_map *map) {
    size_t entry_size = sizeof(struct wist_map_entry) + map->key_size + 
        map->val_size;
    for (size_t i = 0; i < map->buckets_len; i++) {
        struct wist_map_entry *entry = map->buckets[i], *follow;;
        while (entry != NULL) {
            follow = entry;
            entry = entry->next;
            WIST_CTX_FREE_ARR(ctx, follow, uint8_t, entry_size);
        }
    }
}

static struct wist_map_entry *wist_map_einsert(struct wist_ctx *ctx, 
        struct wist_map *map, void *key, void *val) {
    uint32_t hash = map->key_hash(key);
    size_t idx = (size_t) hash % map->buckets_len;

    struct wist_map_entry *entry = map->buckets[idx];
    while (entry != NULL) {
        if (map->key_eq(entry->data, key)) {
            return NULL;
        }
    }

    entry = 
        (struct wist_map_entry *) WIST_CTX_NEW_ARR(ctx, uint8_t, 
                sizeof(struct wist_map_entry) + map->key_size + map->val_size);
    entry->next = map->buckets[idx];
    map->buckets[idx] = entry;

    memcpy(entry->data, key, map->key_size);
    memcpy(entry->data + map->key_size, val, map->val_size);

    return entry;
}

static struct wist_map_entry *wist_map_efind(struct wist_ctx *ctx, 
        struct wist_map *map, void *key) {
    (void) ctx;

    uint32_t hash = map->key_hash(key);
    size_t idx = (size_t) hash % map->buckets_len;

    struct wist_map_entry *entry = map->buckets[idx];
    while (entry != NULL) {
        if (map->key_eq(entry->data, key)) {
            return entry;
        }
    }

    return NULL;
}

static void *_wist_map_insert(struct wist_ctx *ctx, struct wist_map *map,
        void *key, void *val) {
    struct wist_map_entry *entry = wist_map_einsert(ctx, map, key, val);
    if (entry == NULL) {
        return NULL;
    }

    return entry->data + map->key_size;
}

static void *_wist_map_find(struct wist_ctx *ctx, struct wist_map *map,
        void *key) {
    struct wist_map_entry *entry = wist_map_efind(ctx, map, key);
    if (entry == NULL) {
        return NULL;
    }
    return entry->data + map->key_size;
}

static uint32_t _sym_hash(void *_sym) {
    struct wist_sym *sym = *((struct wist_sym **) _sym);
    return sym->str_len; /* IDGAF im bladee. */
}

static void _wist_map_init(struct wist_ctx *ctx, struct wist_map *map, 
        size_t key_size, size_t val_size, wist_map_hash_fn key_hash, 
        wist_map_eq_fn key_eq);

static bool _sym_eq(void *_s1, void *_s2) {
    struct wist_sym *s1 = *((struct wist_sym **) _s1), 
                    *s2 = *((struct wist_sym **) _s2);
    return s1 == s2;

    (void) _wist_map_init;
}

static void _wist_map_init(struct wist_ctx *ctx, struct wist_map *map, 
        size_t key_size, size_t val_size, wist_map_hash_fn key_hash, 
        wist_map_eq_fn key_eq) {
    map->key_size = key_size;
    map->val_size = val_size;
    map->buckets_len = INIT_BUCKETS_LEN;
    map->buckets_filled = 0;
    map->key_hash = key_hash;
    map->key_eq = key_eq;
    map->buckets = WIST_CTX_NEW_ARR(ctx, struct wist_map_entry *, INIT_BUCKETS_LEN);

    /* Required to avoid compiler warnings. */
    (void) wist_map_finish;
    (void) wist_map_einsert;
    (void) wist_map_efind;
    (void) _wist_map_insert;
    (void) _wist_map_find;
    (void) _sym_hash;
    (void) _sym_eq;
}

#define WIST_MAP_INIT(_ctx, _map, _key_type, _val_type, _key_hash, _key_eq)    \
    _wist_map_init(_ctx, _map, sizeof(_key_type), sizeof(_val_type), _key_hash,\
            _key_eq)
#define WIST_MAP_FINISH(_ctx, _map) wist_map_finish(_ctx, _map)

#define WIST_MAP_INSERT(_ctx, _map, _key, _val, _val_type)                     \
    ((_val_type *) _wist_map_insert(_ctx, _map, _key, _val))
#define WIST_MAP_FIND(_ctx, _map, _key, _val_type)                             \
    ((_val_type *) _wist_map_find(_ctx, _map, _key))

#define WIST_SYM_MAP_INIT(_ctx, _map, _val_type) \
    _wist_map_init(_ctx, _map, sizeof(struct wist_sym *), sizeof(_val_type),   \
            _sym_hash, _sym_eq)

#endif /* _WIST_MAP_H */

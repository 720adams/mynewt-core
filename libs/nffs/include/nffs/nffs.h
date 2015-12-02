/**
 * Copyright (c) 2015 Runtime Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef H_NFFS_
#define H_NFFS_

#include <stddef.h>
#include <inttypes.h>

#if 0
#define NFFS_ACCESS_READ        0x01
#define NFFS_ACCESS_WRITE       0x02
#define NFFS_ACCESS_APPEND      0x04
#define NFFS_ACCESS_TRUNCATE    0x08
#endif

#define NFFS_FILENAME_MAX_LEN   256  /* Does not require null terminator. */

#define NFFS_MAX_AREAS          256
#if 0
#define NFFS_EOK                0
#define NFFS_ECORRUPT           1
#define NFFS_EFLASH_ERROR       2
#define NFFS_ERANGE             3
#define NFFS_EINVAL             4
#define NFFS_ENOMEM             5
#define NFFS_ENOENT             6
#define NFFS_EEMPTY             7
#define NFFS_EFULL              8
#define NFFS_EUNEXP             9
#define NFFS_EOS                10
#define NFFS_EEXIST             11
#define NFFS_EACCESS            12
#define NFFS_EUNINIT            13
#endif
struct nffs_config {
    /** Maximum number of inodes; default=1024. */
    uint32_t nc_num_inodes;

    /** Maximum number of data blocks; default=4096. */
    uint32_t nc_num_blocks;

    /** Maximum number of open files; default=4. */
    uint32_t nc_num_files;

    /** Maximum number of open directories; default=4. */
    uint32_t nc_num_dirs;

    /** Inode cache size; default=4. */
    uint32_t nc_num_cache_inodes;

    /** Data block cache size; default=64. */
    uint32_t nc_num_cache_blocks;
};

extern struct nffs_config nffs_config;

struct nffs_area_desc {
    uint32_t nad_offset;    /* Flash offset of start of area. */
    uint32_t nad_length;    /* Size of area, in bytes. */
    uint8_t nad_flash_id;   /* Logical flash id */
};

int nffs_init(void);
int nffs_detect(const struct nffs_area_desc *area_descs);
int nffs_format(const struct nffs_area_desc *area_descs);
int nffs_ready(void);


#endif

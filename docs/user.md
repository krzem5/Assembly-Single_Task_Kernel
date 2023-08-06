# User library

The user library consists of multiple header files:
- [`user/clock.h`](#time-measurement-userclockh)
- [`user/cpu.h`](#cpu-usercpuh)
- [`user/drive.h`](#drive-enumeration-userdriveh)
- [`user/elf.h`](#elf-loading-userelfh)
- [`user/fs.h`](#file-system-operations-userfsh)
- [`user/io.h`](#i/o-userioh)
- [`user/memory.h`](#memory-allocation-usermemoryh)
- [`user/network.h`](#networking-usernetworkh)
- [`user/partition.h`](#partition-enumeration-userpartitionh)
- [`user/system.h`](#system-state-manipulation-usersystemh)
- [`user/types.h`](#common-types-usertypesh)

## Time measurement (`user/clock.h`)

### Variables

```c
extern u64 clock_cpu_frequency;
```

Current clock frequency of the CPU.

### Functions

```c
void clock_init(void);
```

```c
u64 clock_get_ticks(void);
```

Returns the number of clock cycles since boot.

```c
u64 clock_get_time(void);
```

Returns the elapsed time (in nanoseconds) since boot.

```c
u64 clock_ticks_to_time(u64 ticks);
```

Converts clock cycles to nanoseconds using the current CPU clock frequency.

## CPU (`user/cpu.h`)

### Variables

```c
extern u32 cpu_count;
```

```c
extern u32 cpu_bsp_id;
```

### Functions

```c
void cpu_init(void);
```

```c
void cpu_core_start(u32 core,void* func,void* arg);
```

```c
void cpu_core_stop(void);
```

## Drive enumeration (`user/drive.h`)

### Constants

```c
#define DRIVE_FLAG_PRESENT 1
#define DRIVE_FLAG_BOOT 2
```

```c
#define DRIVE_TYPE_AHCI 0
#define DRIVE_TYPE_ATA 1
#define DRIVE_TYPE_ATAPI 2
#define DRIVE_TYPE_NVME 3
```

### Types

```c
typedef struct _DRIVE_STATS{
	u64 root_block_count;
	u64 batc_block_count;
	u64 nda3_block_count;
	u64 nda2_block_count;
	u64 nda1_block_count;
	u64 nfda_block_count;
	u64 data_block_count;
} drive_stats_t;
```

```c
typedef struct _DRIVE{
	u8 flags;
	u8 type;
	u8 index;
	char name[16];
	char serial_number[32];
	char model_number[64];
	u64 block_count;
	u64 block_size;
} drive_t;
```

### Variables

```c
extern drive_t drives[];
```

```c
extern u32 drive_count;
```

```c
extern u32 drive_boot_index;
```

### Functions

```c
void drive_init(void);
```

```c
_Bool drive_format(u32 index,const void* boot,u32 boot_length);
```

```c
_Bool drive_get_stats(u32 index,drive_stats_t* stats);
```

## ELF loading (`user/elf.h`)

### Functions

```c
void elf_load(const char* path);
```

## File system operations (`user/fs.h`)

### Constants

```c
#define FS_ERROR_INVALID_FLAGS -1
#define FS_ERROR_INVALID_FD -2
#define FS_ERROR_INVALID_POINTER -3
#define FS_ERROR_OUT_OF_FDS -4
#define FS_ERROR_NOT_FOUND -5
#define FS_ERROR_UNSUPPORTED_OPERATION -6
#define FS_ERROR_NO_RELATIVE -7
#define FS_ERROR_NOT_EMPTY -8
#define FS_ERROR_DIFFERENT_FS -9
#define FS_ERROR_DIFFERENT_TYPE -10
```

```c
#define FS_FLAG_READ 1
#define FS_FLAG_WRITE 2
#define FS_FLAG_APPEND 4
#define FS_FLAG_CREATE 8
#define FS_FLAG_DIRECTORY 16
```

```c
#define FS_SEEK_SET 0
#define FS_SEEK_ADD 1
#define FS_SEEK_END 2
```

```c
#define FS_RELATIVE_PARENT 0
#define FS_RELATIVE_PREV_SIBLING 1
#define FS_RELATIVE_NEXT_SIBLING 2
#define FS_RELATIVE_FIRST_CHILD 3
```

```c
#define FS_STAT_TYPE_FILE 1
#define FS_STAT_TYPE_DIRECTORY 2
```

### Types

```c
typedef struct _FS_STAT{
	u64 node_id;
	u8 type;
	u8 fs_index;
	u8 name_length;
	char name[64];
	u64 size;
} fs_stat_t;
```

### Functions

```c
int fs_open(int fd,const char* path,u8 flags);
```

```c
int fs_close(int fd);
```

```c
int fs_delete(int fd);
```

```c
s64 fs_read(int fd,void* buffer,u64 count);
```

```c
s64 fs_write(int fd,const void* buffer,u64 count);
```

```c
s64 fs_seek(int fd,u64 offset,u8 type);
```

```c
int fs_absolute_path(int fd,char* buffer,u32 buffer_length);
```

```c
int fs_stat(int fd,fs_stat_t* stat);
```

```c
int fs_get_relative(int fd,u8 relative,u8 flags);
```

```c
int fs_move(int fd,int dst_fd);
```

## I/O (`user/io.h`)

### Functions

```c
void printf(const char* template,...);
```

```c
void print_buffer(const void* buffer,u32 length);
```

```c
void putchar(char c);
```

```c
char getchar(void);
```

```c
int getchar_timeout(u64 timeout);
```

## Memory allocation (`user/memory.h`)

### Constants

```c
#define MEMORY_FLAG_LARGE 1
#define MEMORY_FLAG_EXTRA_LARGE 2
```

### Types

```c
typedef struct _MEMORY_STATS{
	u64 counter_total;
	u64 counter_cpu;
	u64 counter_drive_list;
	u64 counter_driver_ahci;
	u64 counter_driver_i82540;
	u64 counter_fd;
	u64 counter_fs;
	u64 counter_kernel_stack;
	u64 counter_kfs;
	u64 counter_network;
	u64 counter_node_allocator;
	u64 counter_pmm;
	u64 counter_user;
	u64 counter_user_stack;
	u64 counter_vmm;
} memory_stats_t;
```

### Functions

```c
void* memory_map(u64 length,u8 flags);
```

```c
_Bool memory_unmap(void* address,u64 length);
```

```c
_Bool memory_stats(memory_stats_t* out);
```

## Networking (`user/network.h`)

### Types

```c
typedef struct _NETWORK_PACKET{
	u8 address[6];
	u16 protocol;
	u16 buffer_length;
	void* buffer;
} network_packet_t;
```

### Functions

```c
_Bool network_send(const network_packet_t* packet);
```

```c
_Bool network_poll(network_packet_t* packet);
```

## Partition enumeration (`user/partition.h`)

### Constants

```c
#define PARTITION_FLAG_PRESENT 1
#define PARTITION_FLAG_BOOT 2
#define PARTITION_FLAG_HALF_INSTALLED 4
#define PARTITION_FLAG_PREVIOUS_BOOT 8
```

```c
#define PARTITION_TYPE_DRIVE 0
#define PARTITION_TYPE_EMPTY 1
#define PARTITION_TYPE_ISO9660 2
#define PARTITION_TYPE_GPT 3
#define PARTITION_TYPE_KFS 4
```

### Types

```c
typedef struct _PARTITION{
	u8 flags;
	u8 type;
	u8 index;
	u64 first_block_index;
	u64 last_block_index;
	char name[16];
	u32 drive_index;
} partition_t;
```

### Variables

```c
extern partition_t partitions[];
```

```c
extern u32 partition_count;
```

```c
extern u32 partition_boot_index;
```

### Functions

```c
void partition_init(void);
```

## System state manipulation (`user/system.h`)

### Constants

```c
#define SHUTDOWN_FLAG_RESTART 1
```

### Functions

```c
void shutdown(u8 flags);
```

## Common types (`user/types.h`)

### Types

```c
typedef unsigned char u8;
```

```c
typedef unsigned short int u16;
```

```c
typedef unsigned int u32;
```

```c
typedef unsigned long long int u64;
```

```c
typedef signed char s8;
```

```c
typedef signed short int s16;
```

```c
typedef signed int s32;
```

```c
typedef signed long long int s64;
```

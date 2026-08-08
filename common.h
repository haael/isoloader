#ifndef COMMON_H
#define COMMON_H

#include <efi.h>
#include <efilib.h>

/**
 * @file common.h
 * @brief Common definitions, logging macros, and global variables.
 */

/* Logging macros */
#define LOG_DEBUG(fmt, ...) Print(L"[DEBUG] " fmt L"\n", ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  Print(L"[INFO]  " fmt L"\n", ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Print(L"[ERROR] " fmt L"\n", ##__VA_ARGS__)

/* Invalid parameter error code. TODO: eliminate this symbol. */
#ifndef INVALID_PARAMETER_ERROR
#define INVALID_PARAMETER_ERROR EFI_INVALID_PARAMETER
#endif

/* Global variables */
extern EFI_HANDLE gAppImageHandle;

/* Forward declaration for partition info */
typedef struct _PARTITION_INFO PARTITION_INFO;

#endif /* COMMON_H */

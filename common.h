#ifndef COMMON_H
#define COMMON_H

#include <efi.h>
#include <efilib.h>

/**
 * @file common.h
 * @brief Common definitions, logging macros, global variables, and utility functions.
 */

/* Logging macros */
#define LOG_DEBUG(fmt, ...) Print(L"[DEBUG] " fmt L"\n", ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  Print(L"[INFO]  " fmt L"\n", ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Print(L"[ERROR] " fmt L"\n", ##__VA_ARGS__)

/* Global variables */
extern EFI_HANDLE gAppImageHandle;

/**
 * @brief String and array utility functions.
 */

/**
 * @brief Frees an array of CHAR16 strings and the array itself.
 * @param[in] array Pointer to array handle.
 * @param[in] count Number of elements in array.
 * @return EFI_STATUS
 */
EFI_STATUS FreeCHAR16Array(IN CHAR16 ***array, IN UINTN count);

/**
 * @brief Converts a UCS-2 string to UTF-8.
 * @param[in]  src Source UCS-2 string.
 * @param[out] dst Pointer to receive UTF-8 string.
 * @return EFI_STATUS
 */
EFI_STATUS CopyUCS2toUTF8(IN CHAR16 *src, OUT CHAR8 **dst);

/**
 * @brief Converts a UTF-8 string to UCS-2.
 * @param[in]  src Source UTF-8 string.
 * @param[out] dst Pointer to receive UCS-2 string.
 * @return EFI_STATUS
 */
EFI_STATUS CopyUTF8toUCS2(IN CHAR8 *src, OUT CHAR16 **dst);

EFI_STATUS UTF8StrLen(IN CHAR8 *src, OUT UINTN *len);

EFI_STATUS UTF8StrCmp(IN CHAR8 *left, IN CHAR8 *right, IN UINTN maxlen, OUT CHAR8 *relation);

#endif /* COMMON_H */

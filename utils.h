#ifndef UTILS_H
#define UTILS_H

#include "common.h"

/**
 * @file utils.h
 * @brief String and array utility functions.
 */

/**
 * @brief Duplicates a CHAR16 string.
 * @param[in]  src Source string.
 * @param[out] dst Pointer to receive duplicated string.
 * @return EFI_STATUS
 */
//EFI_STATUS StrDuplicate(IN CHAR16 *src, OUT CHAR16 **dst);

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


#endif /* UTILS_H */

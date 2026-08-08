#ifndef FILES_H
#define FILES_H

#include "common.h"

/**
 * @file files.h
 * @brief File reading utilities.
 */

/**
 * @brief Reads a file into a newly allocated buffer.
 * @param[in]  Path       File path to read (NULL-terminated USC-2 string).
 * @param[out] Buffer     Pointer to receive allocated buffer. Raw byte content of the file plus one NULL byte.
 * @param[out] BufferSize Pointer to receive buffer size. Does not include the final NULL byte.
 * @return EFI_STATUS
 */
EFI_STATUS ReadFile(IN CHAR16 *Path, OUT CHAR8 **Buffer, OUT UINTN *BufferSize);

#endif /* FILES_H */

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

/**
 * @brief Builds a path from null-terminated array of path segments.
 * @param[in]  Segments      NULL-terminated array of path segments (may start/end with backslash).
 * @param[in]  SegmentCount  Number of segments (does not include the final NULL item).
 * @param[out] Path          Pointer to receive allocated NULL-terminated path string.
 * @param[out] PathLength    Pointer to receive path length (does not include the final NULL).
 * @return EFI_STATUS
 * @note Initial and final backslashes are removed from each segment. Dot elements (.) are removed.
 *       Double dot elements (..) remove the previous element. If there are too many double dots,
 *       the function fails with EFI_INVALID_PARAMETER.
 */
EFI_STATUS BuildPath(IN CHAR16 **Segments, IN UINTN SegmentCount, OUT CHAR16 **Path, OUT UINTN *PathLength);

#endif /* FILES_H */

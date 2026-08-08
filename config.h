#ifndef CONFIG_H
#define CONFIG_H

#include "common.h"

/**
 * @file config.h
 * @brief Configuration file parser.
 */

/**
 * @brief Parses a configuration file buffer for a given key.
 * @param[in]  Buffer  NULL-terminated UTF-8 configuration text.
 * @param[in]  Key     NULL-terminated UCS-2 Key to search for.
 * @param[out] Values  Pointer to receive array of value strings - NULL-terminated array of NULL-terminated (CHAR16*) UCS-2 strings.
 * @param[out] Count   Pointer to receive number of values (does not include the final NULL item).
 * @return EFI_STATUS
 */
EFI_STATUS ParseConfig(IN CHAR8 *Buffer, IN CHAR16 *Key, OUT CHAR16 ***Values, OUT UINTN *Count);

#endif /* CONFIG_H */

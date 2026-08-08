#include "common.h"

/**
 * @brief Parses a configuration file buffer for a given key.
 * @param[in]  Buffer  UTF-8 configuration text.
 * @param[in]  Key     Key to search for.
 * @param[out] Values  Pointer to receive NULL-terminated array of value strings - NULL-terminated array of NULL-terminated (CHAR16*) UCS-2 strings.
 * @param[out] Count   Pointer to receive number of values (does not include the final NULL item).
 * @return EFI_STATUS
 */
EFI_STATUS ParseConfig(IN CHAR8 *Buffer, IN CHAR16 *Key, OUT CHAR16 ***Values, OUT UINTN *Count)
{
    EFI_STATUS Status = EFI_SUCCESS;
    CHAR8 *Ptr = NULL;
    CHAR8 *LineStart = NULL;
    UINTN ValueArraySize = 0;
    BOOLEAN FoundKey = FALSE;
    CHAR8 *KeyUTF8 = NULL;
    UINTN KeyLen = 0;

    LOG_DEBUG(L"ParseConfig: Buffer=%p, Key=%s, Values=%p, Count=%p",
              (VOID*)Buffer, Key, (VOID*)Values, (VOID*)Count);

    /* Validate input parameters */
    if (!Buffer) {
        LOG_ERROR(L"ParseConfig: Buffer is NULL");
        return EFI_INVALID_PARAMETER;
    }
    if (!Key) {
        LOG_ERROR(L"ParseConfig: Key is NULL");
        return EFI_INVALID_PARAMETER;
    }

    /* Validate output parameters */
    if (!Values) {
        LOG_ERROR(L"ParseConfig: Values is NULL");
        return EFI_INVALID_PARAMETER;
    }
    if (*Values) {
        LOG_ERROR(L"ParseConfig: *Values is not NULL");
        return EFI_INVALID_PARAMETER;
    }
    if (!Count) {
        LOG_ERROR(L"ParseConfig: Count is NULL");
        return EFI_INVALID_PARAMETER;
    }

    /* Convert Key to UTF-8 for comparison */
    Status = CopyUCS2toUTF8(Key, &KeyUTF8);
    if (EFI_ERROR(Status)) {
        LOG_ERROR(L"ParseConfig: CopyUCS2toUTF8 failed, Status=%r", Status);
        goto Error;
    }

    Status = UTF8StrLen(KeyUTF8, &KeyLen);
    if (EFI_ERROR(Status)) {
        LOG_ERROR(L"ParseConfig: UTF8StrLen failed, Status=%r", Status);
        goto Error;
    }

    /* Parse the buffer */
    Ptr = Buffer;
    while (*Ptr) {
        LineStart = Ptr;

        /* Skip whitespace */
        while (*Ptr == ' ' || *Ptr == '\t') {
            Ptr++;
        }

        /* Check for top-level key */
        if (*Ptr && *Ptr != '\n' && *Ptr != '\r') {
            /* Check if this is the key we're looking for */
            CHAR8 Relation;
            Status = UTF8StrCmp(Ptr, KeyUTF8, KeyLen, &Relation);
			if (EFI_ERROR(Status)) {
				LOG_ERROR(L"ParseConfig: UTF8StrCmp failed, Status=%r", Status);
				goto Error;
			}

            if (Relation == 0 && Ptr[KeyLen] == ':') {
                FoundKey = TRUE;
                Ptr += KeyLen + 1; /* Skip past the colon */
                continue;
            }

            /* If we found our key and now see another top-level key, stop */
            if (FoundKey && *Ptr != ' ' && *Ptr != '\t') {
                break;
            }
        }

        /* If we're in the target key's section, collect values */
        if (FoundKey) {
            /* Skip whitespace and newlines */
            while (*Ptr == ' ' || *Ptr == '\n' || *Ptr == '\r') {
                Ptr++;
            }

            /* Check for tab-prefixed values */
            if (*Ptr == '\t') {
                Ptr++; /* Skip the tab */
                CHAR8 *ValueStart = Ptr;
                while (*Ptr && *Ptr != '\n' && *Ptr != '\r') {
                    Ptr++;
                }

                /* Allocate or resize the value array (extra slot for NULL terminator) */
                if (*Count + 1 >= ValueArraySize) {
                    UINTN NewSize = ValueArraySize == 0 ? 4 : ValueArraySize * 2;
                    CHAR16 **NewArray = NULL;

                    if (ValueArraySize == 0) {
                        NewArray = AllocatePool(NewSize * sizeof(CHAR16 *));
                    } else {
                        NewArray = ReallocatePool(*Values, ValueArraySize * sizeof(CHAR16 *), NewSize * sizeof(CHAR16 *));
                    }

                    if (!NewArray) {
                        LOG_ERROR(L"ParseConfig: failed to allocate value array");
                        Status = EFI_OUT_OF_RESOURCES;
                        goto Error;
                    }

                    *Values = NewArray;
                    ValueArraySize = NewSize;
                }

                /* Null-terminate the value */
                CHAR8 TmpChar = *Ptr;
                *Ptr = '\0';
                /* Convert UTF-8 value to UCS-2 and store */
                Status = CopyUTF8toUCS2(ValueStart, &(*Values)[*Count]);
                *Ptr = TmpChar;
                if (EFI_ERROR(Status)) {
                    LOG_ERROR(L"ParseConfig: CopyUTF8toUCS2 failed, Status=%r", Status);
                    goto Error;
                }

                (*Count)++;
                continue;
            }
        }

        /* Move to next line */
        while (*Ptr && *Ptr != '\n' && *Ptr != '\r') {
            Ptr++;
        }

        if (*Ptr) {
            Ptr++;
        }
    }

    /* Null-terminate the array */
    if (*Values && *Count > 0) {
        (*Values)[*Count] = NULL;
    }

    Status = EFI_SUCCESS;
    goto Success;

Error:
    if (Values && *Values) {
        LOG_DEBUG(L"ParseConfig: Error label freeing output Values");
        FreeCHAR16Array(Values, *Count);
    }
    if (Count) {
        *Count = 0;
    }

Success:
    if (KeyUTF8) {
        LOG_DEBUG(L"ParseConfig: Success label freeing temporary KeyUTF8");
        FreePool(KeyUTF8);
    }

    LOG_DEBUG(L"ParseConfig: exit with Status=%r", Status);
    return Status;
}

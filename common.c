#include "common.h"

/**
 * @brief Frees an array of CHAR16 strings and the array itself.
 * @param[in] array Pointer to array handle.
 * @param[in] count Number of elements in array.
 * @return EFI_STATUS
 */
EFI_STATUS FreeCHAR16Array(IN CHAR16 ***array, IN UINTN count)
{
    UINTN i = 0;
    EFI_STATUS Status = EFI_SUCCESS;

    LOG_DEBUG(L"FreeCHAR16Array: array=%p, count=%lu", (VOID*)array, (UINT64)count);

    /* Validate input parameters */
    if (!array) {
        LOG_ERROR(L"FreeCHAR16Array: array is NULL");
        Status = EFI_INVALID_PARAMETER;
        goto Error;
    }
    if (!*array) {
        LOG_ERROR(L"FreeCHAR16Array: *array is NULL");
        Status = EFI_INVALID_PARAMETER;
        goto Error;
    }

    for (i = 0; i < count; i++) {
        if ((*array)[i]) {
            LOG_DEBUG(L"FreeCHAR16Array: freeing element %lu", (UINT64)i);
            FreePool((*array)[i]);
        }
    }

    LOG_DEBUG(L"FreeCHAR16Array: freeing array");
    FreePool(*array);
    *array = NULL;

    Status = EFI_SUCCESS;
    goto Success;

Error:
    /* Nothing to free on parameter error */

Success:
    LOG_DEBUG(L"FreeCHAR16Array: exit with Status=%r", Status);
    return Status;
}

/**
 * @brief Converts a UCS-2 string to UTF-8.
 * @param[in]  src Source UCS-2 string.
 * @param[out] dst Pointer to receive UTF-8 string.
 * @return EFI_STATUS
 */
EFI_STATUS CopyUCS2toUTF8(IN CHAR16 *src, OUT CHAR8 **dst)
{
    EFI_STATUS Status;
    UINTN len = 0;
    UINTN i = 0;

    LOG_DEBUG(L"CopyUCS2toUTF8: src=%p, dst=%p", (VOID*)src, (VOID*)dst);

    /* Validate input parameters */
    if (!src) {
        LOG_ERROR(L"CopyUCS2toUTF8: src is NULL");
        return EFI_INVALID_PARAMETER;
    }

    /* Validate output parameters */
    if (!dst) {
        LOG_ERROR(L"CopyUCS2toUTF8: dst is NULL");
        return EFI_INVALID_PARAMETER;
    }
    if (*dst) {
        LOG_ERROR(L"CopyUCS2toUTF8: *dst is not NULL");
        return EFI_INVALID_PARAMETER;
    }

    len = 0;
    while (src[len]) {
        len++;
    }

    *dst = AllocatePool((len + 1) * sizeof(CHAR8));
    if (!*dst) {
        LOG_ERROR(L"CopyUCS2toUTF8: AllocatePool failed");
        Status = EFI_OUT_OF_RESOURCES;
        goto Error;
    }

    for (i = 0; i < len; i++) {
        (*dst)[i] = (CHAR8)src[i]; /* TODO: proper UCS-2 -> UTF-8 */
    }
    (*dst)[len] = '\0';

    Status = EFI_SUCCESS;
    goto Success;

Error:
    if (dst && *dst) {
        LOG_DEBUG(L"CopyUCS2toUTF8: Error label freeing output *dst");
        FreePool(*dst);
        *dst = NULL;
    }

Success:
    LOG_DEBUG(L"CopyUCS2toUTF8: exit with Status=%r", Status);
    return Status;
}

/**
 * @brief Converts a UTF-8 string to UCS-2.
 * @param[in]  src Source UTF-8 string.
 * @param[out] dst Pointer to receive UCS-2 string.
 * @return EFI_STATUS
 */
EFI_STATUS CopyUTF8toUCS2(IN CHAR8 *src, OUT CHAR16 **dst)
{
    EFI_STATUS Status;
    UINTN len = 0;
    UINTN i = 0;

    LOG_DEBUG(L"CopyUTF8toUCS2: src=%p, dst=%p", (VOID*)src, (VOID*)dst);

    /* Validate input parameters */
    if (!src) {
        LOG_ERROR(L"CopyUTF8toUCS2: src is NULL");
        return EFI_INVALID_PARAMETER;
    }

    /* Validate output parameters */
    if (!dst) {
        LOG_ERROR(L"CopyUTF8toUCS2: dst is NULL");
        return EFI_INVALID_PARAMETER;
    }
    if (*dst) {
        LOG_ERROR(L"CopyUTF8toUCS2: *dst is not NULL");
        return EFI_INVALID_PARAMETER;
    }

    len = 0;
    while (src[len]) {
        len++;
    }

    *dst = AllocatePool((len + 1) * sizeof(CHAR16));
    if (!*dst) {
        LOG_ERROR(L"CopyUTF8toUCS2: AllocatePool failed");
        Status = EFI_OUT_OF_RESOURCES;
        goto Error;
    }

    for (i = 0; i < len; i++) {
        (*dst)[i] = (CHAR16)src[i]; /* TODO: proper UTF-8 -> UCS-2 */
    }
    (*dst)[len] = L'\0';

    Status = EFI_SUCCESS;
    goto Success;

Error:
    if (dst && *dst) {
        LOG_DEBUG(L"CopyUTF8toUCS2: Error label freeing output *dst");
        FreePool(*dst);
        *dst = NULL;
    }

Success:
    LOG_DEBUG(L"CopyUTF8toUCS2: exit with Status=%r", Status);
    return Status;
}

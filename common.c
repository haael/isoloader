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
EFI_STATUS CopyUCS2toUTF8(IN CHAR16 *src, OUT CHAR8 **dst, IN UINTN MaxLen)
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

    //len = 0;
    //while (src[len]) {
    //    len++;
    //}

    Status = UCS2StrLen(src, &len, MaxLen);
    if (EFI_ERROR(Status)) {
        LOG_ERROR(L"CopyUCS2toUTF8: UCS2StrLen failed, Status=%r", Status);
        goto Error;
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
        LOG_DEBUG(L"CopyUCS2toUTF8: freeing output *dst");
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
EFI_STATUS CopyUTF8toUCS2(IN CHAR8 *src, OUT CHAR16 **dst, IN UINTN MaxLen)
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

    //len = 0;
    //while (src[len]) {
    //    len++;
    //}

    Status = UTF8StrLen(src, &len, MaxLen);
    if (EFI_ERROR(Status)) {
        LOG_ERROR(L"CopyUTF8toUCS2: UTF8StrLen failed, Status=%r", Status);
        goto Error;
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
        LOG_DEBUG(L"CopyUTF8toUCS2: freeing output *dst");
        FreePool(*dst);
        *dst = NULL;
    }

Success:
    LOG_DEBUG(L"CopyUTF8toUCS2: exit with Status=%r", Status);
    return Status;
}


EFI_STATUS UTF8StrLen(IN CHAR8 *src, OUT UINTN *len, IN UINTN MaxLen)
{
    EFI_STATUS Status;
    UINTN l = 0;

    LOG_DEBUG(L"UTF8StrLen: src=%p, len=%p, MaxLen=%u", (VOID*)src, (VOID*)len, MaxLen);

    /* Validate input parameters */
    if (!src) {
        LOG_ERROR(L"UTF8StrLen: src is NULL");
        return EFI_INVALID_PARAMETER;
    }

    /* Validate output parameters */
    if (!len) {
        LOG_ERROR(L"UTF8StrLen: len is NULL");
        return EFI_INVALID_PARAMETER;
    }

    l = 0;
    while (l < MaxLen && src[l]) {
        l++;
    }

    if (l >= MaxLen) {
        LOG_ERROR(L"UTF8StrLen: MaxLen exceeded");
        Status = EFI_INVALID_PARAMETER;
        goto Error;
    }

    *len = l;

    Status = EFI_SUCCESS;
    goto Success;

Error:
Success:
    LOG_DEBUG(L"UTF8StrLen: exit with Status=%r", Status);
    return Status;
}


EFI_STATUS UCS2StrLen(IN CHAR16 *src, OUT UINTN *len, IN UINTN MaxLen)
{
    EFI_STATUS Status;
    UINTN l = 0;

    LOG_DEBUG(L"UTF8StrLen: src=%p, len=%p, MaxLen=%u", (VOID*)src, (VOID*)len, MaxLen);

    /* Validate input parameters */
    if (!src) {
        LOG_ERROR(L"UCS2StrLen: src is NULL");
        return EFI_INVALID_PARAMETER;
    }

    /* Validate output parameters */
    if (!len) {
        LOG_ERROR(L"UCS2StrLen: len is NULL");
        return EFI_INVALID_PARAMETER;
    }

    l = 0;
    while (l < MaxLen && src[l]) {
        l++;
    }

    if (l >= MaxLen) {
        LOG_ERROR(L"UCS2StrLen: MaxLen exceeded");
        Status = EFI_INVALID_PARAMETER;
        goto Error;
    }

    *len = l;

    Status = EFI_SUCCESS;
    goto Success;

Error:
Success:
    LOG_DEBUG(L"UCS2StrLen: exit with Status=%r", Status);
    return Status;
}


EFI_STATUS UTF8StrCmp(IN CHAR8 *left, IN CHAR8 *right, OUT CHAR8 *relation, IN UINTN MaxLen)
{
    EFI_STATUS Status;
    UINTN l = 0;

    LOG_DEBUG(L"UTF8StrCmp: left=%p, right=%p, relation=%p, MaxLen=%u", (VOID*)left, (VOID*)right, (VOID*)relation, MaxLen);

    /* Validate input parameters */
    if (!left) {
        LOG_ERROR(L"UTF8StrCmp: left is NULL");
        return EFI_INVALID_PARAMETER;
    }

    if (!right) {
        LOG_ERROR(L"UTF8StrCmp: right is NULL");
        return EFI_INVALID_PARAMETER;
    }

    if (!relation) {
        LOG_ERROR(L"UTF8StrCmp: relation is NULL");
        return EFI_INVALID_PARAMETER;
    }

    l = 0;
    while (l < MaxLen && left[l] && right[l] && (left[l] == right[l])) {
        l++;
    }

    if (l >= MaxLen) {
        LOG_ERROR(L"UTF8StrCmp: MaxLen exceeded");
        Status = EFI_INVALID_PARAMETER;
        goto Error;
    }

    if (left[l] < right[l])
        *relation = 1;
    else if (left[l] > right[l])
        *relation = -1;
    else
        *relation = 0;

    Status = EFI_SUCCESS;
    goto Success;

Error:
Success:
    LOG_DEBUG(L"UTF8StrCmp: exit with Status=%r", Status);
    return Status;
}


EFI_STATUS UCS2StrCmp(IN CHAR16 *left, IN CHAR16 *right, OUT CHAR8 *relation, IN UINTN MaxLen)
{
    EFI_STATUS Status;
    UINTN l = 0;

    LOG_DEBUG(L"UCS2StrCmp: left=%p, right=%p, relation=%p, MaxLen=%u", (VOID*)left, (VOID*)right, (VOID*)relation, MaxLen);

    /* Validate input parameters */
    if (!left) {
        LOG_ERROR(L"UCS2StrCmp: left is NULL");
        return EFI_INVALID_PARAMETER;
    }

    if (!right) {
        LOG_ERROR(L"UCS2StrCmp: right is NULL");
        return EFI_INVALID_PARAMETER;
    }

    if (!relation) {
        LOG_ERROR(L"UCS2StrCmp: relation is NULL");
        return EFI_INVALID_PARAMETER;
    }

    l = 0;
    while (l < MaxLen && left[l] && right[l] && (left[l] == right[l])) {
        l++;
    }

    if (l >= MaxLen) {
        LOG_ERROR(L"UCS2StrCmp: MaxLen exceeded");
        Status = EFI_INVALID_PARAMETER;
        goto Error;
    }

    if (left[l] < right[l])
        *relation = 1;
    else if (left[l] > right[l])
        *relation = -1;
    else
        *relation = 0;

    Status = EFI_SUCCESS;
    goto Success;

Error:
Success:
    LOG_DEBUG(L"UCS2StrCmp: exit with Status=%r", Status);
    return Status;
}




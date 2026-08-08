#include "common.h"
#include "files.h"

/**
 * @brief Reads a file into a newly allocated buffer.
 * @param[in]  Path       File path to read.
 * @param[out] Buffer     Pointer to receive allocated buffer.
 * @param[out] BufferSize Pointer to receive buffer size.
 * @return EFI_STATUS
 */
EFI_STATUS ReadFile(IN CHAR16 *Path, OUT CHAR8 **Buffer, OUT UINTN *BufferSize)
{
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_FILE_HANDLE FileHandle = NULL;
    EFI_FILE_INFO *FileInfo = NULL;
    UINTN FileSize = 0;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem = NULL;
    EFI_FILE_HANDLE Root = NULL;
    EFI_LOADED_IMAGE_PROTOCOL *LoadedImage = NULL;
    EFI_HANDLE DeviceHandle = NULL;

    LOG_DEBUG(L"ReadFile: Path=%s, Buffer=%p, BufferSize=%p",
              Path, (VOID*)Buffer, (VOID*)BufferSize);

    /* Validate input parameters */
    if (!Path) {
        LOG_ERROR(L"ReadFile: Path is NULL");
        return EFI_INVALID_PARAMETER;
    }

    /* Validate output parameters */
    if (!Buffer) {
        LOG_ERROR(L"ReadFile: Buffer is NULL");
        return EFI_INVALID_PARAMETER;
    }
    if (*Buffer) {
        LOG_ERROR(L"ReadFile: *Buffer is not NULL");
        return EFI_INVALID_PARAMETER;
    }
    if (!BufferSize) {
        LOG_ERROR(L"ReadFile: BufferSize is NULL");
        return EFI_INVALID_PARAMETER;
    }

    /* Open loaded image protocol */
    Status = gBS->OpenProtocol(gAppImageHandle, &gEfiLoadedImageProtocolGuid, (VOID **)&LoadedImage, gAppImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (EFI_ERROR(Status)) {
        LOG_ERROR(L"ReadFile: OpenProtocol loaded image failed, Status=%r", Status);
        goto Error;
    }

    DeviceHandle = LoadedImage->DeviceHandle;

    /* Open file system protocol */
    Status = gBS->OpenProtocol(DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (VOID **)&FileSystem, gAppImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
    if (EFI_ERROR(Status)) {
        LOG_ERROR(L"ReadFile: OpenProtocol file system failed, Status=%r", Status);
        goto Error;
    }

    /* Open the volume */
    Status = FileSystem->OpenVolume(FileSystem, &Root);
    if (EFI_ERROR(Status)) {
        LOG_ERROR(L"ReadFile: OpenVolume failed, Status=%r", Status);
        goto Error;
    }

    /* Open the file */
    Status = Root->Open(Root, &FileHandle, Path, EFI_FILE_MODE_READ, 0);
    if (EFI_ERROR(Status)) {
        LOG_ERROR(L"ReadFile: Open file %s failed, Status=%r", Path, Status);
        goto Error;
    }

    /* Get file size */
    Status = FileHandle->GetInfo(FileHandle, &gEfiFileInfoGuid, &FileSize, NULL);
    if (Status != EFI_BUFFER_TOO_SMALL) {
        if (Status == EFI_SUCCESS) {
            Status = EFI_INVALID_PARAMETER;
        }
        LOG_ERROR(L"ReadFile: GetInfo size query failed, Status=%r", Status);
        goto Error;
    }

    /* Allocate buffer for FileInfo */
    FileInfo = AllocatePool(FileSize);
    if (!FileInfo) {
        LOG_ERROR(L"ReadFile: AllocatePool FileInfo failed");
        Status = EFI_OUT_OF_RESOURCES;
        goto Error;
    }

    /* Get file info */
    Status = FileHandle->GetInfo(FileHandle, &gEfiFileInfoGuid, &FileSize, FileInfo);
    if (EFI_ERROR(Status)) {
        LOG_ERROR(L"ReadFile: GetInfo failed, Status=%r", Status);
        goto Error;
    }

    /* Allocate buffer for file contents */
    *Buffer = AllocatePool(FileInfo->FileSize + 1);
    if (!*Buffer) {
        LOG_ERROR(L"ReadFile: AllocatePool buffer failed");
        Status = EFI_OUT_OF_RESOURCES;
        goto Error;
    }

    /* Read file into buffer */
    *BufferSize = FileInfo->FileSize;
    Status = FileHandle->Read(FileHandle, BufferSize, *Buffer);
    if (EFI_ERROR(Status)) {
        LOG_ERROR(L"ReadFile: Read failed, Status=%r", Status);
        goto Error;
    }

    /* Null-terminate the buffer */
    (*Buffer)[*BufferSize] = '\0';

    Status = EFI_SUCCESS;
    goto Success;

Error:
    if (Buffer && *Buffer) {
        LOG_DEBUG(L"ReadFile: Error label freeing output Buffer");
        FreePool(*Buffer);
        *Buffer = NULL;
    }
    if (BufferSize) {
        *BufferSize = 0;
    }

Success:
    if (FileHandle) {
        LOG_DEBUG(L"ReadFile: Success label closing FileHandle");
        FileHandle->Close(FileHandle);
    }
    if (Root) {
        LOG_DEBUG(L"ReadFile: Success label closing Root");
        Root->Close(Root);
    }
    if (FileInfo) {
        LOG_DEBUG(L"ReadFile: Success label freeing FileInfo");
        FreePool(FileInfo);
    }
    if (FileSystem) {
        LOG_DEBUG(L"ReadFile: Success label closing FileSystem protocol");
        gBS->CloseProtocol(DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, gAppImageHandle, NULL);
    }
    if (LoadedImage) {
        LOG_DEBUG(L"ReadFile: Success label closing LoadedImage protocol");
        gBS->CloseProtocol(gAppImageHandle, &gEfiLoadedImageProtocolGuid, gAppImageHandle, NULL);
    }

    LOG_DEBUG(L"ReadFile: exit with Status=%r", Status);
    return Status;
}

/**
 * @brief Builds a path from null-terminated array of path segments.
 * @param[in]  Segments      NULL-terminated array of path segments (may start/end with backslash).
 * @param[in]  SegmentCount  Number of segments (does not include the final NULL item).
 * @param[out] Path          Pointer to receive allocated NULL-terminated path string.
 * @param[out] PathLength    Pointer to receive path length (does not include the final NULL).
 * @return EFI_STATUS
 */
EFI_STATUS BuildPath(IN CHAR16 **Segments, IN UINTN SegmentCount, OUT CHAR16 **Path, OUT UINTN *PathLength)
{
    EFI_STATUS Status = EFI_SUCCESS;
    UINTN i = 0;
    UINTN TotalLength = 0;
    UINTN SegmentStart = 0;
    CHAR16 *TempPath = NULL;
    CHAR16 *Ptr = NULL;
    INTN Depth = 0;

    LOG_DEBUG(L"BuildPath: Segments=%p, SegmentCount=%lu, Path=%p, PathLength=%p",
              (VOID*)Segments, (UINT64)SegmentCount, (VOID*)Path, (VOID*)PathLength);

    /* Validate input parameters */
    if (!Segments) {
        LOG_ERROR(L"BuildPath: Segments is NULL");
        return EFI_INVALID_PARAMETER;
    }

    /* Validate the array structure: must have NULL at the end, no NULLs in the middle */
    for (i = 0; i <= SegmentCount; i++) {
        if (i < SegmentCount) {
            if (Segments[i] == NULL) {
                LOG_ERROR(L"BuildPath: NULL found in the middle of segments array at index %lu", (UINT64)i);
                return EFI_INVALID_PARAMETER;
            }
        } else {
            if (Segments[i] != NULL) {
                LOG_ERROR(L"BuildPath: segments array not NULL-terminated at index %lu", (UINT64)i);
                return EFI_INVALID_PARAMETER;
            }
        }
    }

    /* Validate output parameters */
    if (!Path) {
        LOG_ERROR(L"BuildPath: Path is NULL");
        return EFI_INVALID_PARAMETER;
    }
    if (*Path) {
        LOG_ERROR(L"BuildPath: *Path is not NULL");
        return EFI_INVALID_PARAMETER;
    }
    if (!PathLength) {
        LOG_ERROR(L"BuildPath: PathLength is NULL");
        return EFI_INVALID_PARAMETER;
    }

    /* First pass: calculate total length needed */
    for (i = 0; i < SegmentCount; i++) {
        CHAR16 *Segment = Segments[i];
        UINTN SegmentLen = 0;

        /* Skip leading backslashes */
        while (*Segment == L'\\') {
            Segment++;
        }

        /* Skip trailing backslashes */
        SegmentLen = StrLen(Segment);
        while (SegmentLen > 0 && Segment[SegmentLen - 1] == L'\\') {
            SegmentLen--;
        }

        /* Check for dot elements */
        if (SegmentLen == 1 && Segment[0] == L'.') {
            /* Single dot - skip this segment */
            continue;
        }

        if (SegmentLen == 2 && Segment[0] == L'.' && Segment[1] == L'.') {
            /* Double dot - remove previous element */
            if (Depth <= 0) {
                LOG_ERROR(L"BuildPath: too many double dots at segment %lu", (UINT64)i);
                return EFI_INVALID_PARAMETER;
            }
            Depth--;
            /* Reduce total length by the previous segment's contribution */
            if (TotalLength > 0) {
                /* Find the previous segment's end */
                UINTN j = TotalLength - 1;
                while (j > 0 && TempPath[j - 1] != L'\\') {
                    j--;
                }
                if (j > 0) {
                    TotalLength = j - 1; /* Keep the backslash before this segment */
                } else {
                    TotalLength = 0;
                }
            }
            continue;
        }

        /* Valid segment - add its length plus a backslash */
        if (TotalLength > 0) {
            TotalLength++; /* Add backslash separator */
        }
        TotalLength += SegmentLen;
        Depth++;
    }

    /* Allocate path buffer (extra space for NULL terminator) */
    TempPath = AllocatePool((TotalLength + 1) * sizeof(CHAR16));
    if (!TempPath) {
        LOG_ERROR(L"BuildPath: AllocatePool failed for TempPath");
        Status = EFI_OUT_OF_RESOURCES;
        goto Error;
    }

    /* Second pass: build the path */
    Ptr = TempPath;
    Depth = 0;

    for (i = 0; i < SegmentCount; i++) {
        CHAR16 *Segment = Segments[i];
        UINTN SegmentLen = 0;

        /* Skip leading backslashes */
        while (*Segment == L'\\') {
            Segment++;
        }

        /* Skip trailing backslashes */
        SegmentLen = StrLen(Segment);
        while (SegmentLen > 0 && Segment[SegmentLen - 1] == L'\\') {
            SegmentLen--;
        }

        /* Check for dot elements */
        if (SegmentLen == 1 && Segment[0] == L'.') {
            /* Single dot - skip this segment */
            continue;
        }

        if (SegmentLen == 2 && Segment[0] == L'.' && Segment[1] == L'.') {
            /* Double dot - remove previous element */
            if (Depth <= 0) {
                LOG_ERROR(L"BuildPath: too many double dots at segment %lu", (UINT64)i);
                Status = EFI_INVALID_PARAMETER;
                goto Error;
            }
            Depth--;
            /* Move pointer back to remove previous segment */
            if (Ptr > TempPath) {
                /* Find the start of the previous segment */
                CHAR16 *PrevStart = Ptr - 1;
                while (PrevStart > TempPath && *(PrevStart - 1) != L'\\') {
                    PrevStart--;
                }
                if (PrevStart > TempPath) {
                    Ptr = PrevStart - 1; /* Keep the backslash before this segment */
                } else {
                    Ptr = TempPath;
                }
            }
            continue;
        }

        /* Add backslash separator if not the first segment */
        if (Ptr > TempPath) {
            *Ptr++ = L'\\';
        }

        /* Copy the segment */
        CopyMem(Ptr, Segment, SegmentLen * sizeof(CHAR16));
        Ptr += SegmentLen;
        Depth++;
    }

    /* Null-terminate the path */
    *Ptr = L'\0';

    /* Set output */
    *Path = TempPath;
    *PathLength = TotalLength;
    TempPath = NULL; /* Prevent double-free */

    Status = EFI_SUCCESS;
    goto Success;

Error:
    if (Path) {
        *Path = NULL;
    }
    if (PathLength) {
        *PathLength = 0;
    }

Success:
    if (TempPath) {
        LOG_DEBUG(L"BuildPath: Success label freeing TempPath");
        FreePool(TempPath);
    }

    LOG_DEBUG(L"BuildPath: exit with Status=%r, PathLength=%lu", Status, (UINT64)*PathLength);
    return Status;
}

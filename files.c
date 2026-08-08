#include "common.h"

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
        return INVALID_PARAMETER_ERROR;
    }

    /* Validate output parameters */
    if (!Buffer) {
        LOG_ERROR(L"ReadFile: Buffer is NULL");
        return INVALID_PARAMETER_ERROR;
    }
    if (*Buffer) {
        LOG_ERROR(L"ReadFile: *Buffer is not NULL");
        return INVALID_PARAMETER_ERROR;
    }
    if (!BufferSize) {
        LOG_ERROR(L"ReadFile: BufferSize is NULL");
        return INVALID_PARAMETER_ERROR;
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

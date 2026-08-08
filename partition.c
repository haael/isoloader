#include "common.h"
#include "partition.h"

/**
 * @brief Enumerates all available partitions.
 * @param[out] PartitionSpecs     Pointer to receive NULL-terminated array of partition specs (device path, GUID, or label).
 * @param[out] PartitionSpecCount Pointer to receive number of partition specs (does not include the final NULL item).
 * @return EFI_STATUS
 */
EFI_STATUS EnumeratePartitions(OUT CHAR16 ***PartitionSpecs, OUT UINTN *PartitionSpecCount)
{
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_HANDLE *HandleBuffer = NULL;
    UINTN HandleCount = 0;
    UINTN i = 0;
    UINTN PartitionSpecCapacity = 0;
    CHAR16 **TempPartitionSpecs = NULL;
    EFI_BLOCK_IO_PROTOCOL *BlockIo = NULL;
    EFI_DEVICE_PATH_PROTOCOL *DevicePath = NULL;
    CHAR16 *DevicePathStr = NULL;
    UINTN DevicePathStrSize = 0;

    LOG_DEBUG(L"EnumeratePartitions: PartitionSpecs=%p, PartitionSpecCount=%p",
              (VOID*)PartitionSpecs, (VOID*)PartitionSpecCount);

    /* Validate output parameters */
    if (!PartitionSpecs) {
        LOG_ERROR(L"EnumeratePartitions: PartitionSpecs is NULL");
        return EFI_INVALID_PARAMETER;
    }
    if (*PartitionSpecs) {
        LOG_ERROR(L"EnumeratePartitions: *PartitionSpecs is not NULL");
        return EFI_INVALID_PARAMETER;
    }
    if (!PartitionSpecCount) {
        LOG_ERROR(L"EnumeratePartitions: PartitionSpecCount is NULL");
        return EFI_INVALID_PARAMETER;
    }
    if (*PartitionSpecCount != 0) {
        LOG_ERROR(L"EnumeratePartitions: *PartitionSpecCount is not 0");
        return EFI_INVALID_PARAMETER;
    }

    /* Get all handles that support Block IO protocol */
    Status = gBS->LocateHandleBuffer(ByProtocol, &gEfiBlockIoProtocolGuid, NULL, &HandleCount, &HandleBuffer);
    if (EFI_ERROR(Status)) {
        LOG_ERROR(L"EnumeratePartitions: LocateHandleBuffer failed, Status=%r", Status);
        goto Error;
    }

    /* Allocate initial array for partition specs (extra slot for NULL terminator) */
    PartitionSpecCapacity = HandleCount > 0 ? HandleCount + 1 : 4;
    TempPartitionSpecs = AllocatePool(PartitionSpecCapacity * sizeof(CHAR16 *));
    if (!TempPartitionSpecs) {
        LOG_ERROR(L"EnumeratePartitions: AllocatePool failed for TempPartitionSpecs");
        Status = EFI_OUT_OF_RESOURCES;
        goto Error;
    }

    /* Initialize all slots to NULL */
    for (i = 0; i < PartitionSpecCapacity; i++) {
        TempPartitionSpecs[i] = NULL;
    }

    /* Iterate through all block IO handles */
    for (i = 0; i < HandleCount; i++) {
        /* Get the Block IO protocol */
        Status = gBS->OpenProtocol(HandleBuffer[i], &gEfiBlockIoProtocolGuid, (VOID **)&BlockIo,
                                   gAppImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
        if (EFI_ERROR(Status)) {
            LOG_DEBUG(L"EnumeratePartitions: OpenProtocol BlockIo failed for handle %lu, Status=%r",
                      (UINT64)i, Status);
            continue;
        }

        /* Get the device path protocol */
        Status = gBS->OpenProtocol(HandleBuffer[i], &gEfiDevicePathProtocolGuid, (VOID **)&DevicePath,
                                   gAppImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
        if (EFI_ERROR(Status)) {
            LOG_DEBUG(L"EnumeratePartitions: OpenProtocol DevicePath failed for handle %lu, Status=%r",
                      (UINT64)i, Status);
            gBS->CloseProtocol(HandleBuffer[i], &gEfiBlockIoProtocolGuid, gAppImageHandle, NULL);
            continue;
        }

        /* Convert device path to string */
        DevicePathStr = DevicePathToStr(DevicePath);
        if (!DevicePathStr) {
            LOG_DEBUG(L"EnumeratePartitions: DevicePathToStr failed for handle %lu", (UINT64)i);
            gBS->CloseProtocol(HandleBuffer[i], &gEfiDevicePathProtocolGuid, gAppImageHandle, NULL);
            gBS->CloseProtocol(HandleBuffer[i], &gEfiBlockIoProtocolGuid, gAppImageHandle, NULL);
            continue;
        }

        /* Check if we need to grow the array */
        if (*PartitionSpecCount + 1 >= PartitionSpecCapacity) {
            UINTN NewCapacity = PartitionSpecCapacity * 2;
            CHAR16 **NewArray = ReallocatePool(TempPartitionSpecs,
                                               PartitionSpecCapacity * sizeof(CHAR16 *),
                                               NewCapacity * sizeof(CHAR16 *));
            if (!NewArray) {
                LOG_ERROR(L"EnumeratePartitions: ReallocatePool failed");
                Status = EFI_OUT_OF_RESOURCES;
                FreePool(DevicePathStr);
                goto Error;
            }
            TempPartitionSpecs = NewArray;
            PartitionSpecCapacity = NewCapacity;
            /* Initialize new slots to NULL */
            for (i = *PartitionSpecCount + 1; i < PartitionSpecCapacity; i++) {
                TempPartitionSpecs[i] = NULL;
            }
        }

        /* Store the device path string */
        TempPartitionSpecs[*PartitionSpecCount] = DevicePathStr;
        (*PartitionSpecCount)++;
        DevicePathStr = NULL; /* Ownership transferred */

        /* Close protocols */
        gBS->CloseProtocol(HandleBuffer[i], &gEfiDevicePathProtocolGuid, gAppImageHandle, NULL);
        gBS->CloseProtocol(HandleBuffer[i], &gEfiBlockIoProtocolGuid, gAppImageHandle, NULL);
    }

    /* Null-terminate the array */
    TempPartitionSpecs[*PartitionSpecCount] = NULL;

    /* Set output */
    *PartitionSpecs = TempPartitionSpecs;
    TempPartitionSpecs = NULL; /* Prevent double-free */

    Status = EFI_SUCCESS;
    goto Success;

Error:
    if (TempPartitionSpecs) {
        LOG_DEBUG(L"EnumeratePartitions: Error label freeing TempPartitionSpecs");
        UINTN j = 0;
        for (j = 0; j < PartitionSpecCapacity; j++) {
            if (TempPartitionSpecs[j]) {
                FreePool(TempPartitionSpecs[j]);
            }
        }
        FreePool(TempPartitionSpecs);
    }
    if (PartitionSpecs) {
        *PartitionSpecs = NULL;
    }
    if (PartitionSpecCount) {
        *PartitionSpecCount = 0;
    }

Success:
    if (HandleBuffer) {
        LOG_DEBUG(L"EnumeratePartitions: Success label freeing HandleBuffer");
        FreePool(HandleBuffer);
    }
    if (DevicePathStr) {
        LOG_DEBUG(L"EnumeratePartitions: Success label freeing DevicePathStr");
        FreePool(DevicePathStr);
    }

    LOG_DEBUG(L"EnumeratePartitions: exit with Status=%r, PartitionSpecCount=%lu",
              Status, (UINT64)*PartitionSpecCount);
    return Status;
}

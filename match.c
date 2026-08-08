#include "common.h"
#include "match.h"
#include "partition.h"
#include "utils.h"

/**
 * @brief Filters match files based on partition, directory, and pattern criteria.
 * @param[in]  Partitions        Array of partition infos.
 * @param[in]  PartitionCount    Number of partitions.
 * @param[in]  MatchParts        Array of partition filters.
 * @param[in]  MatchPartCount    Number of partition filters.
 * @param[in]  MatchDirs         Array of directory paths.
 * @param[in]  MatchDirCount     Number of directories.
 * @param[in]  MatchPatterns     Array of file patterns.
 * @param[in]  MatchPatternCount Number of patterns.
 * @param[out] FilePaths         Pointer to receive array of file paths.
 * @param[out] FilePathCount     Pointer to receive number of file paths.
 * @return EFI_STATUS
 */
EFI_STATUS FilterMatchFiles(
    IN PARTITION_INFO *Partitions,
    IN UINTN PartitionCount,
    IN CHAR16 **MatchParts,
    IN UINTN MatchPartCount,
    IN CHAR16 **MatchDirs,
    IN UINTN MatchDirCount,
    IN CHAR16 **MatchPatterns,
    IN UINTN MatchPatternCount,
    OUT CHAR16 ***FilePaths,
    OUT UINTN *FilePathCount
)
{
    EFI_STATUS Status = EFI_SUCCESS;
    PARTITION_INFO *DefaultPartition = NULL;
    CHAR16 **TempFilePaths = NULL;
    UINTN TempFilePathCount = 0;
    UINTN TempFilePathCapacity = 0;
    EFI_FILE_HANDLE PartitionRoot = NULL;
    CHAR16 *FullDirPath = NULL;
    CHAR16 **FileList = NULL;
    UINTN FileCount = 0;
    CHAR16 *FilePath = NULL;
    UINTN i = 0;
    UINTN j = 0;
    UINTN k = 0;
    UINTN l = 0;
    BOOLEAN MatchesPartition = FALSE;
    BOOLEAN IsRelative = FALSE;

    LOG_DEBUG(L"FilterMatchFiles: Partitions=%p, PartitionCount=%lu, MatchParts=%p, MatchPartCount=%lu, MatchDirs=%p, MatchDirCount=%lu, MatchPatterns=%p, MatchPatternCount=%lu, FilePaths=%p, FilePathCount=%p",
              (VOID*)Partitions, (UINT64)PartitionCount, (VOID*)MatchParts, (UINT64)MatchPartCount,
              (VOID*)MatchDirs, (UINT64)MatchDirCount, (VOID*)MatchPatterns, (UINT64)MatchPatternCount,
              (VOID*)FilePaths, (VOID*)FilePathCount);

    /* Validate input parameters */
    if (!Partitions) {
        LOG_ERROR(L"FilterMatchFiles: Partitions is NULL");
        return INVALID_PARAMETER_ERROR;
    }
    if (!MatchParts) {
        LOG_ERROR(L"FilterMatchFiles: MatchParts is NULL");
        return INVALID_PARAMETER_ERROR;
    }
    if (!MatchDirs) {
        LOG_ERROR(L"FilterMatchFiles: MatchDirs is NULL");
        return INVALID_PARAMETER_ERROR;
    }
    if (!MatchPatterns) {
        LOG_ERROR(L"FilterMatchFiles: MatchPatterns is NULL");
        return INVALID_PARAMETER_ERROR;
    }

    /* Validate output parameters */
    if (!FilePaths) {
        LOG_ERROR(L"FilterMatchFiles: FilePaths is NULL");
        return INVALID_PARAMETER_ERROR;
    }
    if (*FilePaths) {
        LOG_ERROR(L"FilterMatchFiles: *FilePaths is not NULL");
        return INVALID_PARAMETER_ERROR;
    }
    if (!FilePathCount) {
        LOG_ERROR(L"FilterMatchFiles: FilePathCount is NULL");
        return INVALID_PARAMETER_ERROR;
    }
    if (*FilePathCount != 0) {
        LOG_ERROR(L"FilterMatchFiles: *FilePathCount is not 0");
        return INVALID_PARAMETER_ERROR;
    }

    /* Find the default partition (the one holding the boot image) */
    Status = GetDefaultPartition(Partitions, PartitionCount, &DefaultPartition);
    if (EFI_ERROR(Status)) {
        LOG_ERROR(L"FilterMatchFiles: GetDefaultPartition failed, Status=%r", Status);
        goto Error;
    }

    /* Iterate over all partitions */
    for (i = 0; i < PartitionCount; i++) {
        MatchesPartition = FALSE;

        /* Check if the partition matches any filter in MatchParts */
        for (j = 0; j < MatchPartCount; j++) {
            if (MatchesPartitionFilter(&Partitions[i], MatchParts[j])) {
                MatchesPartition = TRUE;
                break;
            }
        }

        if (!MatchesPartition) {
            continue;
        }

        /* Open the partition */
        Status = OpenPartition(&Partitions[i], &PartitionRoot);
        if (EFI_ERROR(Status)) {
            LOG_ERROR(L"FilterMatchFiles: OpenPartition %lu failed, Status=%r", (UINT64)i, Status);
            goto Error;
        }

        /* Iterate over all directories in MatchDirs */
        for (j = 0; j < MatchDirCount; j++) {
            CHAR16 *DirPath = MatchDirs[j];
            IsRelative = (DirPath[0] != L'\\');

            /* Skip relative paths if this isn't the default partition */
            if (IsRelative && (&Partitions[i] != DefaultPartition)) {
                continue;
            }

            /* Build the full path for the directory */
            FullDirPath = NULL;
            if (IsRelative) {
                /* Prepend the default partition's path */
                Status = BuildFullPath(DefaultPartition, DirPath, &FullDirPath);
                if (EFI_ERROR(Status)) {
                    LOG_ERROR(L"FilterMatchFiles: BuildFullPath failed, Status=%r", Status);
                    goto Error;
                }
            } else {
                /* Use the absolute path as-is */
                FullDirPath = DirPath;
            }

            /* List files in the directory */
            FileList = NULL;
            FileCount = 0;
            Status = ListFilesInDirectory(PartitionRoot, FullDirPath, &FileList, &FileCount);
            if (EFI_ERROR(Status)) {
                LOG_ERROR(L"FilterMatchFiles: ListFilesInDirectory failed, Status=%r", Status);
                goto Error;
            }

            /* Check each file against the patterns */
            for (k = 0; k < FileCount; k++) {
                for (l = 0; l < MatchPatternCount; l++) {
                    if (MatchesFilePattern(FileList[k], MatchPatterns[l])) {
                        /* Build the full file path: {GUID}\\directory\\path\\file.ext */
                        FilePath = NULL;
                        Status = BuildFilePath(&Partitions[i], FullDirPath, FileList[k], &FilePath);
                        if (EFI_ERROR(Status)) {
                            LOG_ERROR(L"FilterMatchFiles: BuildFilePath failed, Status=%r", Status);
                            goto Error;
                        }

                        /* Add to the result list */
                        if (TempFilePathCount >= TempFilePathCapacity) {
                            UINTN NewCapacity = TempFilePathCapacity == 0 ? 4 : TempFilePathCapacity * 2;
                            CHAR16 **NewFilePaths = ReallocatePool(
                                TempFilePaths,
                                TempFilePathCapacity * sizeof(CHAR16 *),
                                NewCapacity * sizeof(CHAR16 *)
                            );
                            if (!NewFilePaths) {
                                LOG_ERROR(L"FilterMatchFiles: ReallocatePool failed");
                                Status = EFI_OUT_OF_RESOURCES;
                                goto Error;
                            }
                            TempFilePaths = NewFilePaths;
                            TempFilePathCapacity = NewCapacity;
                        }
                        TempFilePaths[TempFilePathCount++] = FilePath;
                        FilePath = NULL; /* Ownership transferred */
                        break; /* No need to check other patterns for this file */
                    }
                }
            }

            /* Cleanup directory iteration */
            if (FileList) {
                LOG_DEBUG(L"FilterMatchFiles: freeing FileList");
                FreeCHAR16Array(&FileList, FileCount);
                FileList = NULL;
            }
            if (IsRelative && FullDirPath) {
                LOG_DEBUG(L"FilterMatchFiles: freeing FullDirPath");
                FreePool(FullDirPath);
                FullDirPath = NULL;
            }
        }

        /* Close the partition */
        if (PartitionRoot) {
            LOG_DEBUG(L"FilterMatchFiles: closing PartitionRoot");
            PartitionRoot->Close(PartitionRoot);
            PartitionRoot = NULL;
        }
    }

    /* Set output */
    *FilePaths = TempFilePaths;
    *FilePathCount = TempFilePathCount;
    TempFilePaths = NULL; /* Prevent double-free */

    Status = EFI_SUCCESS;
    goto Success;

Error:
    if (TempFilePaths) {
        LOG_DEBUG(L"FilterMatchFiles: Error label freeing output TempFilePaths");
        FreeCHAR16Array(&TempFilePaths, TempFilePathCount);
    }
    if (FilePaths) {
        *FilePaths = NULL;
    }
    if (FilePathCount) {
        *FilePathCount = 0;
    }

Success:
    if (PartitionRoot) {
        LOG_DEBUG(L"FilterMatchFiles: Success label closing PartitionRoot");
        PartitionRoot->Close(PartitionRoot);
    }
    if (IsRelative && FullDirPath) {
        LOG_DEBUG(L"FilterMatchFiles: Success label freeing FullDirPath");
        FreePool(FullDirPath);
    }
    if (FileList) {
        LOG_DEBUG(L"FilterMatchFiles: Success label freeing FileList");
        FreeCHAR16Array(&FileList, FileCount);
    }
    if (FilePath) {
        LOG_DEBUG(L"FilterMatchFiles: Success label freeing FilePath");
        FreePool(FilePath);
    }

    LOG_DEBUG(L"FilterMatchFiles: exit with Status=%r", Status);
    return Status;
}

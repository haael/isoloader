#ifndef PARTITION_H
#define PARTITION_H

/**
 * @file partition.h
 * @brief Partition enumeration and path building helpers.
 */


/**
 * @brief Enumerates all available partitions.
 * @param[out] PartitionSpecs     Pointer to receive NULL-terminated array of partition specs (device path, GUID, or label).
 * @param[out] PartitionSpecCount Pointer to receive number of partition specs (does not include the final NULL item).
 * @return EFI_STATUS
 */
EFI_STATUS EnumeratePartitions(OUT CHAR16 ***PartitionSpecs, OUT UINTN *PartitionSpecCount);


/**
 * @brief Finds the default partition (the one holding the boot image).
 * @param[in]  PartitionSpecs     NULL-terminated array of partition specs.
 * @param[in]  PartitionSpecCount Number of partition specs (does not include the final NULL item).
 * @param[out] DefaultPartitionSpec Pointer to receive the default partition spec, that will be set to one of the pointers from PartitionSpecs
 * @return EFI_STATUS
 */
EFI_STATUS GetDefaultPartition(IN CHAR16 **PartitionSpecs, IN UINTN PartitionSpecCount, OUT CHAR16 **DefaultPartitionSpec);


/**
 * @brief Opens a partition and returns its root file handle.
 * @param[in]  PartitionSpec Partition spec (device path, GUID, or label).
 * @param[out] Root          Pointer to receive root file handle.
 * @return EFI_STATUS
 */
EFI_STATUS OpenPartition(IN CHAR16 *PartitionSpec, OUT EFI_FILE_HANDLE *Root);


/**
 * @brief Lists files in a directory.
 * @param[in]  PartitionRoot Root file handle of partition.
 * @param[in]  DirPath      Directory path.
 * @param[out] FileList     Pointer to receive NULL-terminated array of file names.
 * @param[out] FileCount    Pointer to receive number of files (does not include the final NULL item).
 * @return EFI_STATUS
 */
EFI_STATUS ListFilesInDirectory(IN EFI_FILE_HANDLE PartitionRoot, IN CHAR16 *DirPath, OUT CHAR16 ***FileList, OUT UINTN *FileCount);


#endif /* PARTITION_H */

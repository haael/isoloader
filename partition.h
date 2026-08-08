#ifndef PARTITION_H
#define PARTITION_H

#include "common.h"

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
 * @param[out] DefaultPartitionSpec Pointer to receive the default partition spec.
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
 * @brief Builds a full path from a partition spec and relative path.
 * @param[in]  PartitionSpec Partition spec (device path, GUID, or label).
 * @param[in]  RelativePath  Relative path string.
 * @param[out] FullPath      Pointer to receive allocated full path.
 * @return EFI_STATUS
 */
EFI_STATUS BuildFullPath(IN CHAR16 *PartitionSpec, IN CHAR16 *RelativePath, OUT CHAR16 **FullPath);

/**
 * @brief Builds a full file path from partition spec, directory, and filename.
 * @param[in]  PartitionSpec Partition spec (device path, GUID, or label).
 * @param[in]  DirPath        Directory path.
 * @param[in]  FileName       File name.
 * @param[out] FilePath       Pointer to receive allocated file path.
 * @return EFI_STATUS
 */
EFI_STATUS BuildFilePath(IN CHAR16 *PartitionSpec, IN CHAR16 *DirPath, IN CHAR16 *FileName, OUT CHAR16 **FilePath);

#endif /* PARTITION_H */

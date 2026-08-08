#ifndef PARTITION_H
#define PARTITION_H

#include "common.h"

/**
 * @file partition.h
 * @brief Partition enumeration and path building helpers.
 */

/**
 * @brief Enumerates all available partitions.
 * @param[out] Partitions     Pointer to receive array of partition infos.
 * @param[out] PartitionCount Pointer to receive number of partitions.
 * @return EFI_STATUS
 */
EFI_STATUS EnumeratePartitions(OUT PARTITION_INFO **Partitions, OUT UINTN *PartitionCount);

/**
 * @brief Finds the default partition (the one holding the boot image).
 * @param[in]  Partitions     Array of partition infos.
 * @param[in]  PartitionCount Number of partitions.
 * @param[out] DefaultPartition Pointer to receive the default partition.
 * @return EFI_STATUS
 */
EFI_STATUS GetDefaultPartition(IN PARTITION_INFO *Partitions, IN UINTN PartitionCount, OUT PARTITION_INFO **DefaultPartition);

/**
 * @brief Opens a partition and returns its root file handle.
 * @param[in]  Partition Partition to open.
 * @param[out] Root      Pointer to receive root file handle.
 * @return EFI_STATUS
 */
EFI_STATUS OpenPartition(IN PARTITION_INFO *Partition, OUT EFI_FILE_HANDLE *Root);

/**
 * @brief Builds a full path from a partition and relative path.
 * @param[in]  Partition     Partition info.
 * @param[in]  RelativePath  Relative path string.
 * @param[out] FullPath      Pointer to receive allocated full path.
 * @return EFI_STATUS
 */
EFI_STATUS BuildFullPath(IN PARTITION_INFO *Partition, IN CHAR16 *RelativePath, OUT CHAR16 **FullPath);

/**
 * @brief Builds a full file path from partition, directory, and filename.
 * @param[in]  Partition Partition info.
 * @param[in]  DirPath   Directory path.
 * @param[in]  FileName  File name.
 * @param[out] FilePath  Pointer to receive allocated file path.
 * @return EFI_STATUS
 */
EFI_STATUS BuildFilePath(IN PARTITION_INFO *Partition, IN CHAR16 *DirPath, IN CHAR16 *FileName, OUT CHAR16 **FilePath);

#endif /* PARTITION_H */

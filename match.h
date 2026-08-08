#ifndef MATCH_H
#define MATCH_H

#include "common.h"

/**
 * @file match.h
 * @brief File and partition matching functions.
 */

/**
 * @brief Checks if a partition matches a filter string.
 * @param[in] Partition Partition info.
 * @param[in] Filter    Filter string (GUID, label, ".", or "*").
 * @return TRUE if matches, FALSE otherwise.
 */
BOOLEAN MatchesPartitionFilter(IN PARTITION_INFO *Partition, IN CHAR16 *Filter);

/**
 * @brief Checks if a filename matches a glob-style pattern.
 * @param[in] FileName File name.
 * @param[in] Pattern  Pattern string (e.g., "*.efi").
 * @return TRUE if matches, FALSE otherwise.
 */
BOOLEAN MatchesFilePattern(IN CHAR16 *FileName, IN CHAR16 *Pattern);

/**
 * @brief Lists files in a directory.
 * @param[in]  PartitionRoot Root file handle of partition.
 * @param[in]  DirPath      Directory path.
 * @param[out] FileList     Pointer to receive array of file names.
 * @param[out] FileCount    Pointer to receive number of files.
 * @return EFI_STATUS
 */
EFI_STATUS ListFilesInDirectory(IN EFI_FILE_HANDLE PartitionRoot, IN CHAR16 *DirPath, OUT CHAR16 ***FileList, OUT UINTN *FileCount);

/**
 * @brief Filters Match files based on partition, directory, and pattern criteria.
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
);

#endif /* MATCH_H */

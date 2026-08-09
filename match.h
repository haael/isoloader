#ifndef MATCH_H
#define MATCH_H

/**
 * @file match.h
 * @brief File and partition matching functions.
 */


/**
 * @brief Filters files based on partition spec, directory, and pattern criteria.
 * @param[in]  PartitionSpecs        NULL-terminated array of partition specs (device path, GUID, or label).
 * @param[in]  PartitionSpecCount    Number of partition specs (does not include the final NULL item).
 * @param[in]  MatchParts            Array of partition filter specs.
 * @param[in]  MatchPartCount        Number of partition filter specs.
 * @param[in]  MatchDirs             Array of directory paths.
 * @param[in]  MatchDirCount         Number of directories.
 * @param[in]  MatchPatterns         Array of file patterns.
 * @param[in]  MatchPatternCount     Number of patterns.
 * @param[out] FilePaths             Pointer to receive NULL-terminated array of file paths.
 * @param[out] FilePathCount         Pointer to receive number of file paths (does not include the final NULL item).
 * @return EFI_STATUS
 */
EFI_STATUS GetMatchingFiles(
    IN CHAR16 **PartitionSpecs,
    IN UINTN PartitionSpecCount,
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

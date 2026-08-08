#include "common.h"
#include "config.h"
#include "files.h"
#include "match.h"
#include "partition.h"
#include "drivers.h"
#include "bootiso.h"

/* Global variable to store the image handle */
EFI_HANDLE gAppImageHandle = NULL;

const CHAR16* ConfigFile = L"isoloader.conf";

/**
 * @brief Main entry point for the UEFI application.
 * @param[in] ImageHandle  Handle to the loaded image.
 * @param[in] SystemTable  Pointer to the EFI system table.
 * @return EFI_STATUS
 */
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	EFI_STATUS Status;

	LOG_DEBUG(L"efi_main: ImageHandle=%p, SystemTable=%p", (VOID*)ImageHandle, (VOID*)SystemTable);

	/* Validate input parameters */
	if (!ImageHandle) {
		LOG_ERROR(L"efi_main: ImageHandle is NULL");
		Status = EFI_INVALID_PARAMETER;
		goto Cleanup;
	}
	if (!SystemTable) {
		LOG_ERROR(L"efi_main: SystemTable is NULL");
		Status = EFI_INVALID_PARAMETER;
		goto Cleanup;
	}

	/* Initialize the EFI library */
	InitializeLib(ImageHandle, SystemTable);
	/* Set the global image handle */
	gAppImageHandle = ImageHandle;

	CHAR8 *ConfigText = NULL;
	UINTN ConfigTextLength = 0;

	CHAR16 **DriverParts = NULL;
	UINTN DriverPartCount = 0;
	CHAR16 **DriverDirs = NULL;
	UINTN DriverDirCount = 0;
	CHAR16 **DriverPatterns = NULL;
	UINTN DriverPatternCount = 0;

	CHAR16 **ImageParts = NULL;
	UINTN ImagePartCount = 0;
	CHAR16 **ImageDirs = NULL;
	UINTN ImageDirCount = 0;
	CHAR16 **ImagePatterns = NULL;
	UINTN ImagePatternCount = 0;

	CHAR16 **PartitionSpecs = NULL;
	UINTN PartitionSpecCount = 0;

	CHAR16 **DriverFilePaths = NULL;
	UINTN DriverFilePathCount = 0;

	CHAR16 **ImageFilePaths = NULL;
	UINTN ImageFilePathCount = 0;

    UINTN i;

	// Read config file.
	Status = ReadFile((CHAR16*)ConfigFile, &ConfigText, &ConfigTextLength);
	if (EFI_ERROR(Status)) {
		LOG_ERROR(L"efi_main: ReadFile failed, Status=%r", Status);
		goto Cleanup;
	}
	
	// Search for key "driver_partitions" in the config.
	Status = ParseConfig(ConfigText, L"driver_partitions", &DriverParts, &DriverPartCount, ConfigTextLength);
	if (EFI_ERROR(Status)) {
		LOG_ERROR(L"efi_main: ParseConfig driver_partitions failed, Status=%r", Status);
		goto Cleanup;
	}

	// Search for key "driver_directories" in the config.
	Status = ParseConfig(ConfigText, L"driver_directories", &DriverDirs, &DriverDirCount, ConfigTextLength);
	if (EFI_ERROR(Status)) {
		LOG_ERROR(L"efi_main: ParseConfig driver_directories failed, Status=%r", Status);
		goto Cleanup;
	}

	// Search for key "driver_patterns" in the config.
	Status = ParseConfig(ConfigText, L"driver_patterns", &DriverPatterns, &DriverPatternCount, ConfigTextLength);
	if (EFI_ERROR(Status)) {
		LOG_ERROR(L"efi_main: ParseConfig driver_patterns failed, Status=%r", Status);
		goto Cleanup;
	}

	// Search for key "image_partitions" in the config.
	Status = ParseConfig(ConfigText, L"image_partitions", &ImageParts, &ImagePartCount, ConfigTextLength);
	if (EFI_ERROR(Status)) {
		LOG_ERROR(L"efi_main: ParseConfig image_partitions failed, Status=%r", Status);
		goto Cleanup;
	}

	// Search for key "image_directories" in the config.
	Status = ParseConfig(ConfigText, L"image_directories", &ImageDirs, &ImageDirCount, ConfigTextLength);
	if (EFI_ERROR(Status)) {
		LOG_ERROR(L"efi_main: ParseConfig image_directories failed, Status=%r", Status);
		goto Cleanup;
	}

	// Search for key "image_patterns" in the config.
	Status = ParseConfig(ConfigText, L"image_patterns", &ImagePatterns, &ImagePatternCount, ConfigTextLength);
	if (EFI_ERROR(Status)) {
		LOG_ERROR(L"efi_main: ParseConfig image_patterns failed, Status=%r", Status);
		goto Cleanup;
	}
	
	// Release the buffer of config file text.
	FreePool(ConfigText);
	ConfigText = NULL;
	LOG_DEBUG(L"efi_main: freed ConfigText");
	
	// Find all partitions on the system.
	Status = EnumeratePartitions(&PartitionSpecs, &PartitionSpecCount);
	if (EFI_ERROR(Status)) {
		LOG_ERROR(L"efi_main: EnumeratePartitions failed, Status=%r", Status);
		goto Cleanup;
	}

	// Find the driver files in each configured partition/directory/pattern combination.
	if (DriverPartCount > 0 && DriverDirCount > 0 && DriverPatternCount > 0) {
		Status = FilterMatchFiles(PartitionSpecs, PartitionSpecCount, DriverParts, DriverPartCount, DriverDirs, DriverDirCount, DriverPatterns, DriverPatternCount, &DriverFilePaths, &DriverFilePathCount);
		if (EFI_ERROR(Status)) {
			LOG_ERROR(L"efi_main: FilterMatchFiles (drivers) failed, Status=%r", Status);
			goto Cleanup;
		}
	}
	else
	{
		LOG_ERROR(L"efi_main: driver path list empty");
		Status = EFI_SUCCESS;
		goto Cleanup;
	}

    for (i = 0; i < DriverFilePathCount; i++) {
		Status = LoadDriver(DriverFilePaths[i]);
		if (EFI_ERROR(Status)) {
			LOG_ERROR(L"efi_main: driver '%ls' load failed, Status=%r", DriverFilePaths[i], Status);
			// driver load error is not fatal; continue
		}
	}

	// Find the ISO files in each configured partition/directory/pattern combination.
	if (ImagePartCount > 0 && ImageDirCount > 0 && ImagePatternCount > 0) {
		Status = FilterMatchFiles(PartitionSpecs, PartitionSpecCount, ImageParts, ImagePartCount, ImageDirs, ImageDirCount, ImagePatterns, ImagePatternCount, &ImageFilePaths, &ImageFilePathCount);
		if (EFI_ERROR(Status)) {
			LOG_ERROR(L"efi_main: FilterMatchFiles (ISO) failed, Status=%r", Status);
			goto Cleanup;
		}
	}
	else
	{
		LOG_ERROR(L"efi_main: ISO path list empty");
		Status = EFI_SUCCESS;
		goto Cleanup;
	}


    i = 0; // get image number

	Status = BootImage(ImageFilePaths[i]);
	if (EFI_ERROR(Status)) {
		LOG_ERROR(L"efi_main: image '%ls' load failed, Status=%r", ImageFilePaths[i], Status);
		// driver load error is not fatal; continue
	}


	// Set the return status to success.
	Status = EFI_SUCCESS;

Cleanup:
	if (ConfigText) {
		LOG_DEBUG(L"efi_main: cleanup freeing ConfigText");
		FreePool(ConfigText);
	}
	if (DriverParts) {
		LOG_DEBUG(L"efi_main: cleanup freeing DriverParts");
		FreeCHAR16Array(&DriverParts, DriverPartCount);
	}
	if (DriverDirs) {
		LOG_DEBUG(L"efi_main: cleanup freeing DriverDirs");
		FreeCHAR16Array(&DriverDirs, DriverDirCount);
	}
	if (DriverPatterns) {
		LOG_DEBUG(L"efi_main: cleanup freeing DriverPatterns");
		FreeCHAR16Array(&DriverPatterns, DriverPatternCount);
	}
	if (ImageParts) {
		LOG_DEBUG(L"efi_main: cleanup freeing ImageParts");
		FreeCHAR16Array(&ImageParts, ImagePartCount);
	}
	if (ImageDirs) {
		LOG_DEBUG(L"efi_main: cleanup freeing ImageDirs");
		FreeCHAR16Array(&ImageDirs, ImageDirCount);
	}
	if (ImagePatterns) {
		LOG_DEBUG(L"efi_main: cleanup freeing ImagePatterns");
		FreeCHAR16Array(&ImagePatterns, ImagePatternCount);
	}
	if (PartitionSpecs) {
		LOG_DEBUG(L"efi_main: cleanup freeing PartitionSpecs");
		FreeCHAR16Array(&PartitionSpecs, PartitionSpecCount);
	}
	if (DriverFilePaths) {
		LOG_DEBUG(L"efi_main: cleanup freeing DriverFilePaths");
		FreeCHAR16Array(&DriverFilePaths, DriverFilePathCount);
	}
	if (DriverFilePaths) {
		LOG_DEBUG(L"efi_main: cleanup freeing ImageFilePaths");
		FreeCHAR16Array(&ImageFilePaths, ImageFilePathCount);
	}

	LOG_DEBUG(L"efi_main: exit with Status=%r", Status);
	return Status;
}

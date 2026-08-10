

#include "common.h"
#include "config.h"
#include "files.h"
#include "match.h"
#include "partition.h"
#include "drivers.h"
#include "bootiso.h"
#include "uiface.h"


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

	LOG_DEBUG(L"ISOLoader: ImageHandle=%p, SystemTable=%p", (VOID*)ImageHandle, (VOID*)SystemTable);

	/* Validate input parameters */
	if (!ImageHandle) {
		LOG_ERROR(L"ISOLoader: ImageHandle is NULL");
		Status = EFI_INVALID_PARAMETER;
		goto Cleanup;
	}
	if (!SystemTable) {
		LOG_ERROR(L"ISOLoader: SystemTable is NULL");
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
	
	INTN UIChoice;
	UINTN UIState;
	
	// Read config file.
	Status = ReadFile((CHAR16*)ConfigFile, &ConfigText, &ConfigTextLength);
	if (EFI_ERROR(Status)) {
		LOG_ERROR(L"ISOLoader: ReadFile failed, Status=%r", Status);
		goto Cleanup;
	}
	
	// Search for key "driver_partitions" in the config.
	Status = ParseConfig(ConfigText, L"driver_partitions", &DriverParts, &DriverPartCount, ConfigTextLength);
	if (EFI_ERROR(Status)) {
		LOG_ERROR(L"ISOLoader: ParseConfig driver_partitions failed, Status=%r", Status);
		goto Cleanup;
	}

	// Search for key "driver_directories" in the config.
	Status = ParseConfig(ConfigText, L"driver_directories", &DriverDirs, &DriverDirCount, ConfigTextLength);
	if (EFI_ERROR(Status)) {
		LOG_ERROR(L"ISOLoader: ParseConfig driver_directories failed, Status=%r", Status);
		goto Cleanup;
	}

	// Search for key "driver_patterns" in the config.
	Status = ParseConfig(ConfigText, L"driver_patterns", &DriverPatterns, &DriverPatternCount, ConfigTextLength);
	if (EFI_ERROR(Status)) {
		LOG_ERROR(L"ISOLoader: ParseConfig driver_patterns failed, Status=%r", Status);
		goto Cleanup;
	}

	// Search for key "image_partitions" in the config.
	Status = ParseConfig(ConfigText, L"image_partitions", &ImageParts, &ImagePartCount, ConfigTextLength);
	if (EFI_ERROR(Status)) {
		LOG_ERROR(L"ISOLoader: ParseConfig image_partitions failed, Status=%r", Status);
		goto Cleanup;
	}

	// Search for key "image_directories" in the config.
	Status = ParseConfig(ConfigText, L"image_directories", &ImageDirs, &ImageDirCount, ConfigTextLength);
	if (EFI_ERROR(Status)) {
		LOG_ERROR(L"ISOLoader: ParseConfig image_directories failed, Status=%r", Status);
		goto Cleanup;
	}

	// Search for key "image_patterns" in the config.
	Status = ParseConfig(ConfigText, L"image_patterns", &ImagePatterns, &ImagePatternCount, ConfigTextLength);
	if (EFI_ERROR(Status)) {
		LOG_ERROR(L"ISOLoader: ParseConfig image_patterns failed, Status=%r", Status);
		goto Cleanup;
	}
	
	// Release the buffer of config file text.
	FreePool(ConfigText);
	ConfigText = NULL;
	LOG_DEBUG(L"ISOLoader: freed ConfigText");
	
	// Find all partitions on the system.
	Status = EnumeratePartitions(&PartitionSpecs, &PartitionSpecCount);
	if (EFI_ERROR(Status)) {
		LOG_ERROR(L"ISOLoader: EnumeratePartitions failed, Status=%r", Status);
		goto Cleanup;
	}
	
	// Find the driver files in each configured partition/directory/pattern combination.
	if (DriverPartCount > 0 && DriverDirCount > 0 && DriverPatternCount > 0) {
		Status = GetMatchingFiles(PartitionSpecs, PartitionSpecCount, DriverParts, DriverPartCount, DriverDirs, DriverDirCount, DriverPatterns, DriverPatternCount, &DriverFilePaths, &DriverFilePathCount);
		if (EFI_ERROR(Status)) {
			LOG_ERROR(L"ISOLoader: GetMatchingFiles (drivers) failed, Status=%r", Status);
			goto Cleanup;
		}
	}
	else
	{
		LOG_INFO(L"ISOLoader: Driver path list empty.");
		DriverFilePaths = NULL;
		DriverFilePathCount = 0;
	}
	
    for (i = 0; i < DriverFilePathCount; i++) {
		Status = LoadDriver(DriverFilePaths[i]);
		if (EFI_ERROR(Status)) {
			LOG_ERROR(L"ISOLoader: driver '%ls' load failed, Status=%r", DriverFilePaths[i], Status);
			// driver load error is not fatal; continue
		} else {
			LOG_INFO(L"ISOLoader: Loaded filesystem driver '%ls'.", DriverFilePaths[i]);
		}
	}
	
	// Find the ISO files in each configured partition/directory/pattern combination.
	if (ImagePartCount > 0 && ImageDirCount > 0 && ImagePatternCount > 0) {
		Status = GetMatchingFiles(PartitionSpecs, PartitionSpecCount, ImageParts, ImagePartCount, ImageDirs, ImageDirCount, ImagePatterns, ImagePatternCount, &ImageFilePaths, &ImageFilePathCount);
		if (EFI_ERROR(Status)) {
			LOG_ERROR(L"ISOLoader: GetMatchingFiles (ISO) failed, Status=%r", Status);
			goto Cleanup;
		}
	}
	else
	{
		LOG_INFO(L"ISOLoader: ISO path list empty.");
		ImageFilePaths = NULL;
		ImageFilePathCount = 0;
	}
	
	UIState = 0;
	while(1) {
		Status = PrintIsoImages(ImageFilePaths, ImageFilePathCount, UIState);
		if (EFI_ERROR(Status)) {
			LOG_ERROR(L"ISOLoader: PrintIsoImages, Status=%r", Status);
			goto Cleanup;
		}
		
	    Status = GetUserChoice(ImageFilePaths, ImageFilePathCount, &UIChoice, &UIState); // get image number: 0...ImageFilePathCount
		if (EFI_ERROR(Status)) {
			LOG_ERROR(L"ISOLoader: GetUserChoice, Status=%r", Status);
			goto Cleanup;
		}
		
		if (UIChoice == -1) // exit
			break;
		
		if (UIChoice == -2) // non-action UI change
			continue;
		
		// boot
		LOG_INFO(L"ISOLoader: Booting image '%ls'.", ImageFilePaths[i]);
		Status = BootImage(ImageFilePaths[i]); // does not exit on successful boot
		if (EFI_ERROR(Status)) {
			LOG_ERROR(L"ISOLoader: image '%ls' load failed, Status=%r", ImageFilePaths[i], Status);
			// non-fatal error, try again
		} else {
			LOG_INFO(L"ISOLoader: not booting image '%ls'", ImageFilePaths[i]);
			// booting of image abandoned for some reason
		}
	};
	
	// Set the return status to success.
	Status = EFI_SUCCESS;

Cleanup:
	if (ConfigText) {
		LOG_DEBUG(L"ISOLoader: freeing ConfigText");
		FreePool(ConfigText);
	}
	
	if (DriverParts) {
		LOG_DEBUG(L"ISOLoader: freeing DriverParts");
		Status = FreeCHAR16Array(&DriverParts, DriverPartCount);
		if (EFI_ERROR(Status)) {
			LOG_ERROR(L"ISOLoader: FreeCHAR16Array failed on DriverParts=%x, Status=%r", DriverParts, Status);
		}
	}
	
	if (DriverDirs) {
		LOG_DEBUG(L"ISOLoader: freeing DriverDirs");
		Status = FreeCHAR16Array(&DriverDirs, DriverDirCount);
		if (EFI_ERROR(Status)) {
			LOG_ERROR(L"ISOLoader: FreeCHAR16Array failed on DriverDirs=%x, Status=%r", DriverDirs, Status);
		}
	}
	
	if (DriverPatterns) {
		LOG_DEBUG(L"ISOLoader: freeing DriverPatterns");
		Status = FreeCHAR16Array(&DriverPatterns, DriverPatternCount);
		if (EFI_ERROR(Status)) {
			LOG_ERROR(L"ISOLoader: FreeCHAR16Array failed on DriverPatterns=%x, Status=%r", DriverPatterns, Status);
		}
	}
	
	if (ImageParts) {
		LOG_DEBUG(L"ISOLoader: freeing ImageParts");
		Status = FreeCHAR16Array(&ImageParts, ImagePartCount);
		if (EFI_ERROR(Status)) {
			LOG_ERROR(L"ISOLoader: FreeCHAR16Array failed on ImageParts=%x, Status=%r", ImageParts, Status);
		}
	}
	
	if (ImageDirs) {
		LOG_DEBUG(L"ISOLoader: freeing ImageDirs");
		Status = FreeCHAR16Array(&ImageDirs, ImageDirCount);
		if (EFI_ERROR(Status)) {
			LOG_ERROR(L"ISOLoader: FreeCHAR16Array failed on ImageDirs=%x, Status=%r", ImageDirs, Status);
		}
	}
	
	if (ImagePatterns) {
		LOG_DEBUG(L"ISOLoader: freeing ImagePatterns");
		Status = FreeCHAR16Array(&ImagePatterns, ImagePatternCount);
		if (EFI_ERROR(Status)) {
			LOG_ERROR(L"ISOLoader: FreeCHAR16Array failed on ImagePatterns=%x, Status=%r", ImagePatterns, Status);
		}
	}
	
	if (PartitionSpecs) {
		LOG_DEBUG(L"ISOLoader: freeing PartitionSpecs");
		Status = FreeCHAR16Array(&PartitionSpecs, PartitionSpecCount);
		if (EFI_ERROR(Status)) {
			LOG_ERROR(L"ISOLoader: FreeCHAR16Array failed on PartitionSpecs=%x, Status=%r", PartitionSpecs, Status);
		}
	}
	
	if (DriverFilePaths) {
		LOG_DEBUG(L"ISOLoader: freeing DriverFilePaths");
		Status = FreeCHAR16Array(&DriverFilePaths, DriverFilePathCount);
		if (EFI_ERROR(Status)) {
			LOG_ERROR(L"ISOLoader: FreeCHAR16Array failed on DriverFilePaths=%x, Status=%r", DriverFilePaths, Status);
		}
	}
	
	if (DriverFilePaths) {
		LOG_DEBUG(L"ISOLoader: freeing ImageFilePaths");
		Status = FreeCHAR16Array(&ImageFilePaths, ImageFilePathCount);
		if (EFI_ERROR(Status)) {
			LOG_ERROR(L"ISOLoader: FreeCHAR16Array failed on ImageFilePaths=%x, Status=%r", ImageFilePaths, Status);
		}
	}
	
	LOG_DEBUG(L"ISOLoader: exit with Status=%r", Status);
	return Status;
}

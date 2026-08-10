
#include "drivers.h"


EFI_STATUS LoadDriver(IN CHAR16 *FilePath)
{
/*
	// Load and start *.efi driver.
	
	EFI_HANDLE DriverImage = NULL;
	EFI_HANDLE FilesystemHandle = NULL;
	EFI_DEVICE_PATH_PROTOCOL *DriverDevicePath = NULL;

	DriverDevicePath = FileDevicePath(FilesystemHandle, FilePath);

	gBS->LoadImage(FALSE, gAppImageHandle, DriverDevicePath, NULL, 0, &DriverImage);
	gBS->StartImage(DriverImage, NULL, NULL);
*/
}

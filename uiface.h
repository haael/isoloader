#ifndef UIFACE_H
#define UIFACE_H


EFI_STATUS PrintIsoImages(IN CHAR16 **ImagePaths, IN UINTN ImagePathCount, IN UINTN UIState);

EFI_STATUS GetUserChoice(IN CHAR16 **ImagePaths, IN UINTN ImagePathCount, OUT INTN *UIChoice, OUT UINTN *UIState);


#endif /* UIFACE_H */

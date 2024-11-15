#ifndef _UEFI_EFI_SIMPLE_TEXT_OUTPUT_H_
#define _UEFI_EFI_SIMPLE_TEXT_OUTPUT_H_ 1
#include <uefi/efi/types.h>



typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL{
	efi_status_t (EFI_API *Reset)(struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,efi_bool_t ExtendedVerification);
	efi_status_t (EFI_API *OutputString)(struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* This,efi_wchar_t* String);
	void* TestString;
	void* QueryMode;
	void* SetMode;
	void* SetAttribute;
	void* ClearScreen;
	void* SetCursorPosition;
	void* EnableCursor;
	void* Mode;
} efi_simple_text_output_protocol_t;



#endif

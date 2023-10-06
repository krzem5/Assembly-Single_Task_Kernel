#include <efi.h>
#include <uefi/relocator.h>



EFI_STATUS efi_main(EFI_HANDLE image,EFI_SYSTEM_TABLE* system_table){
	relocate_executable();
	system_table->ConOut->OutputString(system_table->ConOut,L"Press a key: ");
	UINTN index;
	system_table->BootServices->WaitForEvent(1,&(system_table->ConIn->WaitForKey),&index);
	system_table->ConOut->OutputString(system_table->ConOut,L"\r\n");
	return EFI_SUCCESS;
}

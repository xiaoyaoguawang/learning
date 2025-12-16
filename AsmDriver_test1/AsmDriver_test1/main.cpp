#include <ntddk.h>
#include <wdf.h>

extern "C" void MainASM();
extern "C" void MainASM2();

VOID DriverUnload(_In_ WDFDRIVER Driver)
{
}

extern "C" NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
	NTSTATUS status = STATUS_SUCCESS;
	WDF_DRIVER_CONFIG config;
	WDF_DRIVER_CONFIG_INIT(&config, WDF_NO_EVENT_CALLBACK);
	config.EvtDriverUnload = DriverUnload;
	MainASM();
	return status;
}
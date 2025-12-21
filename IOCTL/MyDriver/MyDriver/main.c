#include <ntdef.h>
#include <ntddk.h>
#include <wdf.h>
#include <wdm.h>
#include "driver.h"

VOID Unload(_In_ PDRIVER_OBJECT DriverObject);
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath);
NTSTATUS DrvUnsupported(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
NTSTATUS DrvCreate(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
NTSTATUS DrvIoCtlDispatcher(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);
#ifdef __cplusplus
extern "C" {
#endif
void EnableVmx();
#ifdef __cplusplus
}
#endif

NTSTATUS
DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
	//创建设备与创建符号链接
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING deviceName;
	UNICODE_STRING symbolicLinkName;
	PDEVICE_OBJECT deviceObject = NULL; 

	RtlInitUnicodeString(&deviceName, L"\\Device\\Myhv");
	RtlInitUnicodeString(&symbolicLinkName, L"\\DosDevices\\Myhv");

	DbgPrint("Myhv DriverEntry Called\n");
	
	status = IoCreateDevice(
		DriverObject,
		0,
		&deviceName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&deviceObject
	);
	if (!NT_SUCCESS(status)) {
		DbgPrint("IoCreateDevice failed with status: 0x%08X\n", status);
		return status;
	}

	DbgPrint("IoCreateDevice succeeded. Device object created.\n");
	//初始化驱动程序对象的主要功能指针
	for (int i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
		DriverObject->MajorFunction[i] = DrvUnsupported;
	}
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DrvCreate;
	DriverObject->DriverUnload = Unload;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DrvIoCtlDispatcher;
	deviceObject->Flags &= (~DO_DEVICE_INITIALIZING);
	status = IoCreateSymbolicLink(&symbolicLinkName, &deviceName);
	if (!NT_SUCCESS(status)) {
		DbgPrint("IoCreateSymbolicLink failed with status: 0x%08X\n", status);
		// 如果符号链接创建失败，需要删除已创建的设备对象
		IoDeleteDevice(deviceObject);
		return status;
	}

	DbgPrint("Symbolic link created successfully.\n");
	return STATUS_SUCCESS;
}

VOID Unload(_In_ PDRIVER_OBJECT DriverObject)
{
	DbgPrint("Myhv Unload Called\n");
	UNICODE_STRING symbolicLinkName;
	
	RtlInitUnicodeString(&symbolicLinkName, L"\\DosDevices\\Myhv");
	IoDeleteSymbolicLink(&symbolicLinkName);
	IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS DrvUnsupported(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp) {
	DbgPrint("this function is unsupported\n");
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS DrvCreate(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp) {
	DbgPrint("Myhv DrvCreate Called\n");
	EnableVmx();
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

VOID
PrintChars(
	PCHAR  BufferAddress,
	size_t CountChars)
{
	PAGED_CODE();

	if (CountChars)
	{
		while (CountChars--)
		{
			if (*BufferAddress > 31 && *BufferAddress != 127)
			{
				KdPrint(("%c", *BufferAddress));
			}
			else
			{
				KdPrint(("."));
			}
			BufferAddress++;
		}
		KdPrint(("\n"));
	}
	return;
}

//IOCTL在“用户态和内核态之间传递数据的四种方法”
NTSTATUS DrvIoCtlDispatcher(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp) {
	PIO_STACK_LOCATION IrpStack;
	NTSTATUS status = STATUS_SUCCESS;
	ULONG InBufferLen;
	ULONG OutBufferLen;
	PCHAR InBuf, OutBuf;
	PCHAR Data = "the string is from Device";
	size_t DataLen = strlen(Data + 1);
	PCHAR Buffer;
	PMDL MDL;

	UNREFERENCED_PARAMETER(DeviceObject);
	PAGED_CODE();
	IrpStack = IoGetCurrentIrpStackLocation(Irp);
	InBufferLen = IrpStack->Parameters.DeviceIoControl.InputBufferLength;   //输入缓冲区长度,对应DeviceIoControl的InBufferLength
	OutBufferLen = IrpStack->Parameters.DeviceIoControl.OutputBufferLength; //输出缓冲区长度,对应DeviceIoControl的OutBufferLength

	if (!InBufferLen || !OutBufferLen) {
		status = STATUS_INVALID_PARAMETER;
		goto END;
	}

	switch (IrpStack->Parameters.DeviceIoControl.IoControlCode) {
		//同一缓冲区Irp->AssociatedIrp.SystemBuffer承载了输入和输出数据
	case IOCTL_SIOCTL_METHOD_BUFFERED:
		DbgPrint("Called METHOD_BUFFERED\n");

		InBuf = Irp->AssociatedIrp.SystemBuffer;
		OutBuf = Irp->AssociatedIrp.SystemBuffer;

		DbgPrint("Data from User:\n");
		DbgPrint(InBuf);
		PrintChars(InBuf, InBufferLen);

		RtlCopyBytes(OutBuf, Data, OutBufferLen);

		DbgPrint("Data to User:\n");
		PrintChars(OutBuf, DataLen);

		Irp->IoStatus.Information = (OutBufferLen < DataLen ? OutBufferLen : DataLen);
		break;
		//输出缓冲区指针是Irp->UserBuffer，输入缓冲区指针是IrpStack->Parameters.DeviceIoControl.Type3InputBuffer
		//需要分别通过MDL=IoAllocateMdl(InBuf, InBufferLen, FALSE, TRUE, NULL);
		//            MDL=IoAllocateMdl(OutBuf, InBufferLen, FALSE, TRUE, NULL);
		//将MDL与输入输出缓冲区绑定，通过MmProbeAndLockPages锁定用户缓冲区页
		//然后通过MmGetSystemAddressForMdlSafe获取系统地址进行访问
	case IOCTL_SIOCTL_METHOD_NEITHER:
		DbgPrint("Called METHOD_NEITHER\n");

		InBuf = IrpStack->Parameters.DeviceIoControl.Type3InputBuffer;
		OutBuf = Irp->UserBuffer;

		try {
			ProbeForRead(InBuf, InBufferLen, sizeof(UCHAR));
			DbgPrint("Data from User:\n");
			DbgPrint(InBuf);
			PrintChars(InBuf, InBufferLen);
		}except(EXCEPTION_EXECUTE_HANDLER) {
			DbgPrint("Exception occurred while probing input buffer\n");
			status = GetExceptionCode();
			break;
		}
		MDL = IoAllocateMdl(InBuf, InBufferLen, FALSE, TRUE, NULL);
		if (!MDL) {
			DbgPrint("IoAllocateMdl failed\n");
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		try {
			MmProbeAndLockPages(MDL, UserMode, IoReadAccess);
		}except(EXCEPTION_EXECUTE_HANDLER) {
			DbgPrint("Exception occurred while probing and locking pages\n");
			IoFreeMdl(MDL);
			status = GetExceptionCode();
			break;
		}
		Buffer = MmGetSystemAddressForMdlSafe(MDL, NormalPagePriority | MdlMappingNoExecute);
		if (!Buffer) {
			DbgPrint("MmGetSystemAddressForMdlSafe failed\n");
			MmUnlockPages(MDL);
			IoFreeMdl(MDL);
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		DbgPrint("Data from User(SystemAddress):\n");
		DbgPrint(Buffer);
		PrintChars(Buffer, InBufferLen);

		MmUnlockPages(MDL);
		IoFreeMdl(MDL);

		MDL = IoAllocateMdl(OutBuf, OutBufferLen, FALSE, TRUE, NULL);
		if (!MDL) {
			DbgPrint("IoAllocateMdl for output buffer failed\n");
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		try {
			MmProbeAndLockPages(MDL, UserMode, IoWriteAccess);
		}except(EXCEPTION_EXECUTE_HANDLER) {
			DbgPrint("Exception occurred while probing and locking output pages\n");
			IoFreeMdl(MDL);
			status = GetExceptionCode();
			break;
		}
		Buffer = MmGetSystemAddressForMdlSafe(MDL, NormalPagePriority | MdlMappingNoExecute);
		if (!Buffer) {
			DbgPrint("MmGetSystemAddressForMdlSafe for output buffer failed\n");
			MmUnlockPages(MDL);
			IoFreeMdl(MDL);
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		RtlCopyBytes(Buffer, Data, OutBufferLen);

		DbgPrint("Data to User(SystemAddress):\n");
		PrintChars(Buffer, DataLen);

		MmUnlockPages(MDL);
		IoFreeMdl(MDL);

		Irp->IoStatus.Information = (OutBufferLen < DataLen ? OutBufferLen : DataLen);
		break;

		//METHOD_IN_DIRECT和METHOD_OUT_DIRECT中使用的MDL的起始地址来自用户输出缓冲区指针，长度来自输出缓冲区长度
	case IOCTL_SIOCTL_METHOD_IN_DIRECT:
		DbgPrint("Called METHOD_IN_DIRECT\n");
		InBuf = Irp->AssociatedIrp.SystemBuffer;
		DbgPrint("Data from User:\n");
		DbgPrint(InBuf);
		PrintChars(InBuf, InBufferLen);

		Buffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority | MdlMappingNoExecute);//对应输出缓冲区
		if (!Buffer) {
			DbgPrint("MmGetSystemAddressForMdlSafe for METHOD_IN_DIRECT failed\n");
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		DbgPrint("Data from User:\n");
		DbgPrint(Buffer);
		PrintChars(Buffer, OutBufferLen);

		Irp->IoStatus.Information = MmGetMdlByteCount(Irp->MdlAddress);//最大可写长度
		break;

	case IOCTL_SIOCTL_METHOD_OUT_DIRECT:
		DbgPrint("Called METHOD_OUT_DIRECT\n");
		InBuf = Irp->AssociatedIrp.SystemBuffer;
		DbgPrint("Data from User:\n");
		DbgPrint(InBuf);
		PrintChars(InBuf, InBufferLen);

		Buffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority | MdlMappingNoExecute);//对应输出缓冲区
		if (!Buffer) {
			DbgPrint("MmGetSystemAddressForMdlSafe for METHOD_OUT_DIRECT failed\n");
			status = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		RtlCopyBytes(Buffer, Data, OutBufferLen);
		DbgPrint("Data to User(SystemAddress):\n");
		PrintChars(Buffer, DataLen);

		Irp->IoStatus.Information = (OutBufferLen < DataLen ? OutBufferLen : DataLen);
		break;
	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}
END:
	Irp->IoStatus.Status = status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}
#include "FdtTable.h"

STATIC VOID     *mFdtTable = NULL;
STATIC INT32    mReservedMemParentNode = 0;

//
// Fdt need to add by order, so need some variable to save the target data first.
//
STATIC FDT_ALLOC_LIST                mFdtAllocList   = {0};
STATIC ADDRESS                       mFdtPayloadBase = NULL;

#define MEMORY_ATTRIBUTE_DEFAULT  (  RESOURCE_ATTRIBUTE_PRESENT                 | \
                                     RESOURCE_ATTRIBUTE_INITIALIZED             | \
                                     RESOURCE_ATTRIBUTE_TESTED                  | \
                                     RESOURCE_ATTRIBUTE_UNCACHEABLE             | \
                                     RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE       | \
                                     RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE | \
                                     RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE    )

CHAR8  *mMemoryAllocType[] = {
  "Reserved",
  "LoaderCode",
  "LoaderData",
  "BootServicesCode",
  "BootServicesData",
  "RuntimeServicesCode",
  "RuntimeServicesData",
  "ConventionalMemory",
  "UnusableMemory",
  "ACPIReclaimMemory",
  "ACPIMemoryNVS",
  "MemoryMappedIO",
  "MemoryMappedIOPortSpace",
  "PalCode",
  "PersistentMemory",
};

/**
  Returns the pointer to the Fdt table.

  @return The pointer to the Fdt table.

**/
VOID *
GetFdtTable (
  VOID
  )
{
  return mFdtTable;
}

/**
  Builds a Fdt for the every memory allocation node.

  @param  BaseAddress  The 64 bit address of the memory.
  @param  Length       The length of the memory allocation in bytes.
  @param  MemoryType   Type of memory allocated by this HOB.

  @retval SUCCESS  If it completed successfully.

**/
RETURN_STATUS
BuildMemAllocFdtNode (
  IN ADDRESS        BaseAddress,
  IN UINT64         Length,
  IN MEMORY_TYPE    MemoryType
  )
{
  FDT_ALLOC_NODE  *TmpNode;

  TmpNode = &mFdtAllocList.Node[mFdtAllocList.Length];
  TmpNode->BaseAddress = BaseAddress;
  TmpNode->Length      = Length;
  TmpNode->MemoryType  = MemoryType;
  mFdtAllocList.Length++;

  return SUCCESS;
}

/**
  Builds a Fdt for the memory allocation.

  @param  BaseAddress  The 64 bit address of the memory.
  @param  Length       The length of the memory allocation in bytes.
  @param  MemoryType   Type of memory allocated by this HOB.
  @param  ParentNode   The target node in fdt.

  @retval SUCCESS  If it completed successfully.

**/
RETURN_STATUS
BuildMemAllocFdt (
  IN ADDRESS        BaseAddress,
  IN UINT64         Length,
  IN MEMORY_TYPE    MemoryType,
  IN INT32          ParentNode
  )
{
  RETURN_STATUS    Status;
  INT32            TempNode;
  UINT8            TempStr[32];
  UINT8            TempAddrStr[9];
  UINT64           RegTmp[2];

  ZeroMem (TempStr, sizeof(TempStr));
  ZeroMem (TempAddrStr, sizeof(TempAddrStr));

  CopyMem (TempStr, mMemoryAllocType[MemoryType], sizeof(CHAR8) * AsciiStrnLenS(mMemoryAllocType[MemoryType], 32));
  AddrToAsciiS ((UINTN)BaseAddress, TempAddrStr);
  AsciiStrCat ((CHAR8 *)TempStr, sizeof("@"), "@");
  AsciiStrCat ((CHAR8 *)TempStr, sizeof(TempAddrStr), (CHAR8 *)TempAddrStr);

  TempNode  = fdt_add_subnode(mFdtTable, ParentNode, (CHAR8 *)TempStr);
  RegTmp[0] = cpu_to_fdt64(BaseAddress);
  RegTmp[1] = cpu_to_fdt64(Length);

  Status = fdt_setprop(mFdtTable, TempNode, "reg", &RegTmp, sizeof(RegTmp));
  if (ERROR (Status)) {
    return Status;
  }

  Status = fdt_setprop_u32(mFdtTable, TempNode, "type", MemoryType);
  if (ERROR (Status)) {
    return Status;
  }

  return SUCCESS;
}

/**
  Initialize Fdt table.

  @retval SUCCESS  If it completed successfully.

**/
RETURN_STATUS
FdtTableInit (
  VOID
  )
{
  RETURN_STATUS    Status;
  UINTN            FdtSize;
  UINTN            FdtPages;

  if (mFdtTable != NULL) {
    return SUCCESS;
  }

  FdtSize = 8 * PAGE_SIZE;
  FdtPages = SIZE_TO_PAGES (FdtSize);
  mFdtTable  = AllocatePages (FdtPages);
  if (mFdtTable == NULL) {
    return NOT_FOUND;
  }

  Status = fdt_create_empty_tree (mFdtTable, FdtSize);
  if (ERROR (Status)) {
    return Status;
  }

  // Set cell property of root node
  fdt_setprop_cell(mFdtTable, 0, "#address-cells", 2);
  fdt_setprop_cell(mFdtTable, 0, "#size-cells", 2);

  return SUCCESS;
}

/**
  Build payload base address fdt node.

  @retval SUCCESS    If it completed successfully.

**/
RETURN_STATUS
BuildPayloadBaseFdt (
  VOID
  )
{
  RETURN_STATUS    Status;
  INT32            TempNode;

  TempNode = fdt_add_subnode(mFdtTable, 0, "PayloadBase");

  Status = fdt_setprop_u64(mFdtTable, TempNode, "entry", (UINT64)mFdtPayloadBase);
  if (ERROR (Status)) {
    return Status;
  }
}

/**
  It will build FDT based on memory allocation information from Hobs.

  @retval SUCCESS  If it completed successfully.
  @retval Others       If it failed to build required FDT.
**/
RETURN_STATUS
BuildFdtMemAlloc (
  VOID
  )
{
  INT32           ParentNode;
  INT32           i;
  FDT_ALLOC_NODE  *TmpNode;

  // Create allocate memory parent node
  ParentNode = fdt_add_subnode(mFdtTable, 0, "memory-allocation");

  fdt_setprop_cell(mFdtTable, ParentNode, "#address-cells", 2);
  fdt_setprop_cell(mFdtTable, ParentNode, "#size-cells", 2);

  for (i = 0; i < mFdtAllocList.Length; i++) {
    TmpNode = &mFdtAllocList.Node[i];
    BuildMemAllocFdt (TmpNode->BaseAddress, TmpNode->Length, TmpNode->MemoryType, ParentNode);
  }

  return SUCCESS;
}

/**
  It will build FDT based on serial information.

  @param  ParentNode   Offset of parent node

  @retval SUCCESS  If it completed successfully.
  @retval Others       If it failed to build required FDT.

**/
RETURN_STATUS
BuildFdtForSerial (
  IN  INT32  ParentNode
  )
{
  RETURN_STATUS     Status;
  SERIAL_PORT_INFO  SerialPortInfo;
  INT32             TempNode;
  UINT8             TempStr[32];
  UINT8             TempAddrStr[9];
  UINT64            RegData[2];

  ZeroMem (TempStr, sizeof(TempStr));
  ZeroMem (TempAddrStr, sizeof(TempAddrStr));

  Status = ParseSerialInfo (&SerialPortInfo);
  if (!ERROR (Status)) {
    //
    // Create SerialPortInfo FDT node.
    //
    CopyMem (TempStr, "serial@", sizeof("serial@"));
    AddrToAsciiS ((UINTN)SerialPortInfo.BaseAddr, TempAddrStr);
    AsciiStrCat ((CHAR8 *)TempStr, sizeof (TempAddrStr), (CHAR8 *)TempAddrStr);
    // TempNode = fdt_add_subnode(mFdtTable, ParentNode, (CHAR8 *)TempStr);
    TempNode = fdt_add_subnode(mFdtTable, 0, (CHAR8 *)TempStr);

    Status = fdt_setprop_u32(mFdtTable, TempNode, "current-speed", SerialPortInfo.Baud);

    RegData[0] = cpu_to_fdt64(SerialPortInfo.BaseAddr);
    RegData[1] = 0;
    Status = fdt_setprop(mFdtTable, TempNode, "reg", &RegData, sizeof(RegData));
    if (ERROR (Status)) {
      return Status;
    }

    Status = fdt_setprop_u32(mFdtTable, TempNode, "stride", (UINT8)SerialPortInfo.RegWidth);
    if (ERROR (Status)) {
      return Status;
    }

    Status = fdt_setprop_u32(mFdtTable, TempNode, "mmio", (UINT32)((SerialPortInfo.Type == 1) ? FALSE : TRUE));
    if (ERROR (Status)) {
      return Status;
    }
  }

  return Status;
}

/**
  It will build FDT for UPL required data.

  @param  ParentNode    Offset of parent node

  @retval SUCCESS    If it completed successfully.
  @retval Others     If it failed to build required FDT.

**/
RETURN_STATUS
BuildFdtForUplRequired (
  IN  INT32  ParentNode
  )
{
  RETURN_STATUS                     Status;
  UNIVERSAL_PAYLOAD_SMBIOS_TABLE    SmBiosTable;
  UNIVERSAL_PAYLOAD_ACPI_TABLE      AcpiTable;
  EFI_PEI_GRAPHICS_INFO_HOB         GfxInfo;
  EFI_PEI_GRAPHICS_DEVICE_INFO_HOB  GfxDeviceInfo;
  INT32                             TempNode;
  UINT32                            RegEax;
  UINT32                            Data32[3];
  UINT8                             PhysicalAddressBits;
  UINT64                            RegData[2];
  INT32                             i;
  UINT8                             TempStr[32];
  UINT8                             TempAddrStr[9];

  //
  // Build CPU memory space and IO space hob
  //
  AsmCpuid (0x80000000, &RegEax, NULL, NULL, NULL);
  if (RegEax >= 0x80000008) {
    AsmCpuid (0x80000008, &RegEax, NULL, NULL, NULL);
    PhysicalAddressBits = (UINT8)RegEax;
  } else {
    PhysicalAddressBits = 36;
  }

  // TempNode = fdt_add_subnode(mFdtTable, ParentNode, "cpu_info");
  TempNode = fdt_add_subnode(mFdtTable, 0, "cpu-info");

  Status = fdt_setprop_u32(mFdtTable, TempNode, "memoryspace", (UINT32)PhysicalAddressBits);
  if (ERROR (Status)) {
    return Status;
  }

  //
  // ACPI table 
  //
  Status = ParseAcpiTableInfo (&AcpiTable);
  if (ERROR (Status)) {
    return Status;
  }

  // TempNode = fdt_add_subnode(mFdtTable, ParentNode, "acpi");
  TempNode = fdt_add_subnode(mFdtTable, 0, "acpi");

  Status = fdt_setprop_u64(mFdtTable, TempNode, "rsdp", (UINT64)AcpiTable.Rsdp);
  if (ERROR (Status)) {
    return Status;
  }

  //
  // SmBios table
  //
  Status = ParseSmbiosTable (&SmBiosTable);
  if (ERROR (Status)) {
    return Status;
  }

  // TempNode = fdt_add_subnode(mFdtTable, ParentNode, "smbios");
  TempNode = fdt_add_subnode(mFdtTable, 0, "smbios");

  Status = fdt_setprop_u64(mFdtTable, TempNode, "entry", (UINT64)SmBiosTable.SmBiosEntryPoint);
  if (ERROR (Status)) {
    return Status;
  }

  //
  // Create fdt for frame buffer information
  //
  Status = ParseGfxInfo (&GfxInfo);
  if (!ERROR (Status)) {
    TempNode = fdt_add_subnode(mFdtTable, 0, "graphic-info");
  
    RegData[0] = cpu_to_fdt64(GfxInfo.FrameBufferBase);
    RegData[1] = cpu_to_fdt64(GfxInfo.FrameBufferSize);
    Status = fdt_setprop(mFdtTable, TempNode, "reg", &RegData, sizeof(RegData));
    if (ERROR (Status)) {
      return Status;
    }

    Data32[0] = cpu_to_fdt32(GfxInfo.GraphicsMode.HorizontalResolution);
    Data32[1] = cpu_to_fdt32(GfxInfo.GraphicsMode.VerticalResolution);
    Status = fdt_setprop(mFdtTable, TempNode, "resolution", &Data32, sizeof(UINT32) * 2);
    if (ERROR (Status)) {
      return Status;
    }

    Status = fdt_setprop_u32(mFdtTable, TempNode, "pixel-format", (UINT32)GfxInfo.GraphicsMode.PixelFormat);
    if (ERROR (Status)) {
      return Status;
    }

    Data32[0] = cpu_to_fdt32(GfxInfo.GraphicsMode.PixelInformation.RedMask);
    Data32[1] = cpu_to_fdt32(GfxInfo.GraphicsMode.PixelInformation.GreenMask);
    Data32[2] = cpu_to_fdt32(GfxInfo.GraphicsMode.PixelInformation.BlueMask);
    Status = fdt_setprop(mFdtTable, TempNode, "pixel-mask", &Data32, sizeof(UINT32) * 3);
    if (ERROR (Status)) {
      return Status;
    }

    Status = fdt_setprop_u32(mFdtTable, TempNode, "pixel-scanline", (UINT32)GfxInfo.GraphicsMode.PixelsPerScanLine);
    if (ERROR (Status)) {
      return Status;
    }
  }

  Status = ParseGfxDeviceInfo (&GfxDeviceInfo);
  if (!ERROR (Status)) {
    TempNode = fdt_add_subnode(mFdtTable, 0, "graphic-device");

    Status = fdt_setprop_u32(mFdtTable, TempNode, "vendor-id", (UINT32)GfxDeviceInfo.VendorId);
    if (ERROR (Status)) {
      return Status;
    }

    Status = fdt_setprop_u32(mFdtTable, TempNode, "device-id", (UINT32)GfxDeviceInfo.DeviceId);
    if (ERROR (Status)) {
      return Status;
    }

    Status = fdt_setprop_u32(mFdtTable, TempNode, "subsystem-vendor-id", (UINT32)GfxDeviceInfo.SubsystemVendorId);
    if (ERROR (Status)) {
      return Status;
    }

    Status = fdt_setprop_u32(mFdtTable, TempNode, "subsystem-id", (UINT32)GfxDeviceInfo.SubsystemId);
    if (ERROR (Status)) {
      return Status;
    }

    Status = fdt_setprop_u32(mFdtTable, TempNode, "revision-id", (UINT32)GfxDeviceInfo.RevisionId);
    if (ERROR (Status)) {
      return Status;
    }

    Status = fdt_setprop_u32(mFdtTable, TempNode, "bar-index", (UINT32)GfxDeviceInfo.BarIndex);
    if (ERROR (Status)) {
      return Status;
    }
  }

  //
  // Build payload base address.
  //
  BuildPayloadBaseFdt ();

  return SUCCESS;
}

/**
  It will build FDT for UPL consumed.

  @retval SUCCESS    If it completed successfully.
  @retval Others     If it failed to build required FDT.

**/
RETURN_STATUS
BuildFdtForUPL (
  VOID
  )
{
  RETURN_STATUS    Status;
  INT32            ParentNode;

  //
  // Build FDT nodes for UPL consumed
  //
  // ParentNode = fdt_add_subnode(mFdtTable, 0, "upl");

  // fdt_setprop_cell(mFdtTable, ParentNode, "#address-cells", 2);
  // fdt_setprop_cell(mFdtTable, ParentNode, "#size-cells", 2);

  Status = BuildFdtForSerial (ParentNode);
  if (ERROR (Status)) {
    return Status;
  }

  Status = BuildFdtForUplRequired (ParentNode);
  if (ERROR (Status)) {
    return Status;
  }

  return Status;
}

/**
  Create mmio memory fdt node.

  @retval SUCCESS    If it completed successfully.

**/
RETURN_STATUS
CreateReservedMemFdt (
  VOID
  )
{
  //
  // Create mmio momory parent node
  //
  mReservedMemParentNode = fdt_add_subnode(mFdtTable, 0, "reserved-memory");

  fdt_setprop_cell(mFdtTable, mReservedMemParentNode, "#address-cells", 2);
  fdt_setprop_cell(mFdtTable, mReservedMemParentNode, "#size-cells", 2);

  return SUCCESS;
}

/**
  Build memory information fdt node.

  @param  BaseAddress  The 64 bit physical address of the memory.
  @param  Length       The length of the memory in bytes.
  @param  Attribute    Attribute of the memory .

  @retval SUCCESS      If it completed successfully.

**/
RETURN_STATUS
BuildMemInfoFdt (
  IN  ADDRESS                  BaseAddress,
  IN  UINT64                   Length,
  IN  RESOURCE_ATTRIBUTE_TYPE  Attribute
  )
{
  RETURN_STATUS    Status;
  INT32            TempNode;
  UINT8            TempStr[32];
  UINT8            TempAddrStr[9];
  UINT64           RegTmp[2];

  ZeroMem (TempStr, sizeof(TempStr));
  ZeroMem (TempAddrStr, sizeof(TempAddrStr));

  CopyMem (TempStr, "memory@", sizeof("memory@"));
  AddrToAsciiS ((UINTN)BaseAddress, TempAddrStr);
  AsciiStrCat ((CHAR8 *)TempStr, sizeof(TempAddrStr), (CHAR8 *)TempAddrStr);
  TempNode = fdt_add_subnode(mFdtTable, 0, (CHAR8 *)TempStr);

  Status = fdt_setprop_string(mFdtTable, TempNode, "device_type", "memory");
  if (ERROR (Status)) {
    return Status;
  }

  RegTmp[0] = cpu_to_fdt64(BaseAddress);
  RegTmp[1] = cpu_to_fdt64(Length);
  Status = fdt_setprop(mFdtTable, TempNode, "reg", &RegTmp, sizeof(RegTmp));
  if (ERROR (Status)) {
    return Status;
  }

  if (Attribute != MEMORY_ATTRIBUTE_DEFAULT) {
    Status = fdt_setprop_u32(mFdtTable, TempNode, "attr", Attribute);
    if (ERROR (Status)) {
      return Status;
    }
  }

  return SUCCESS;
}

/**
  Build memory information fdt node.

  @param  BaseAddress   The 64 bit physical address of the memory.
  @param  Length        The length of the memory in bytes.
  @param  ResourceType  Type of the memory.
  @param  Attribute     Attribute of the memory .

  @retval SUCCESS    If it completed successfully.

**/
RETURN_STATUS
BuildReservedMemFdt (
  IN  ADDRESS                  BaseAddress,
  IN  UINT64                   Length,
  IN  RESOURCE_TYPE            ResourceType,
  IN  RESOURCE_ATTRIBUTE_TYPE  Attribute
  )
{
  RETURN_STATUS    Status;
  INT32            TempNode;
  UINT8            TempStr[32];
  UINT8            TempAddrStr[9];
  UINT64           RegTmp[2];

  // MMIO and Reserved Memory
  if (ResourceType != RESOURCE_SYSTEM_MEMORY &&
      ResourceType != RESOURCE_IO &&
      ResourceType != RESOURCE_IO_RESERVED) {
    ZeroMem (TempStr, sizeof(TempStr));
    ZeroMem (TempAddrStr, sizeof(TempAddrStr));
    if (ResourceType == RESOURCE_MEMORY_MAPPED_IO ||
        ResourceType == RESOURCE_MEMORY_MAPPED_IO_PORT) {
      CopyMem (TempStr, "mmio@", sizeof("mmio@"));
      AddrToAsciiS ((UINTN)BaseAddress, TempAddrStr);
      AsciiStrCat ((CHAR8 *)TempStr, sizeof(TempAddrStr), (CHAR8 *)TempAddrStr);
    } else if (ResourceType == RESOURCE_FIRMWARE_DEVICE) {
      CopyMem (TempStr, "firmware@", sizeof("firmware@"));
      AddrToAsciiS ((UINTN)BaseAddress, TempAddrStr);
      AsciiStrCat ((CHAR8 *)TempStr, sizeof(TempAddrStr), (CHAR8 *)TempAddrStr);
    } else if (ResourceType == RESOURCE_MEMORY_RESERVED) {
      CopyMem (TempStr, "reserved@", sizeof("reserved@"));
      AddrToAsciiS ((UINTN)BaseAddress, TempAddrStr);
      AsciiStrCat ((CHAR8 *)TempStr, sizeof(TempAddrStr), (CHAR8 *)TempAddrStr);
    }

    TempNode = fdt_add_subnode(mFdtTable, mReservedMemParentNode, (CHAR8 *)TempStr);

    RegTmp[0] = cpu_to_fdt64(BaseAddress);
    RegTmp[1] = cpu_to_fdt64(Length);
    Status = fdt_setprop(mFdtTable, TempNode, "reg", &RegTmp, sizeof(RegTmp));
    if (ERROR (Status)) {
      return Status;
    }

    if (Attribute != MEMORY_ATTRIBUTE_DEFAULT) {
      Status = fdt_setprop_u32(mFdtTable, TempNode, "attr", Attribute);
      if (ERROR (Status)) {
        return Status;
      }
    }
  }

  return SUCCESS;
}

/**
  Build hand off information fdt node.

  @param  FreeMemoryBottom   Free memory start address
  @param  FreeMemoryTop      Free memory end address.

  @retval SUCCESS    If it completed successfully.

**/
RETURN_STATUS
BuildReservedMemUefi (
  IN VOID  *FreeMemoryBottom,
  IN VOID  *FreeMemoryTop
  )
{
  RETURN_STATUS    Status;
  INT32            TempNode;
  UINT64           RegData[2];

  // Create mmio momory parent node
  TempNode = fdt_add_subnode(mFdtTable, mReservedMemParentNode, "uefi");

  RegData[0] = cpu_to_fdt64((UINT64)FreeMemoryBottom);
  RegData[1] = cpu_to_fdt64((UINT64)FreeMemoryTop - (UINT64)FreeMemoryBottom);
  Status = fdt_setprop(mFdtTable, TempNode, "reg", &RegData, sizeof(RegData));
  if (ERROR (Status)) {
    return Status;
  }

  return SUCCESS;
}

/**
  Save payload base address.

  @param  BaseAddress    Extra data start address

  @retval SUCCESS    If it completed successfully.

**/
RETURN_STATUS
SetFdtPayloadBase (
  IN  ADDRESS  BaseAddress
  )
{
  if (BaseAddress == NULL) {
    return INVALID_PARAMETER;
  }

  mFdtPayloadBase = BaseAddress;
  return SUCCESS;
}
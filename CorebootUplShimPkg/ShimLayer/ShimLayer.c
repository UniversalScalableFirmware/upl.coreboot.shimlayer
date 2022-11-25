/** @file

Copyright (c) 2022, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "ShimLayer.h"
#include "FdtTable.h"

STATIC UINT32   mTopOfLowerUsableDram = 0;
STATIC MEM_POOL mShimLayerMemory = {0};

GUID gGraphicsInfoHobGuid                   = { 0x39f62cce, 0x6825, 0x4669, { 0xbb, 0x56, 0x54, 0x1a, 0xba, 0x75, 0x3a, 0x07 }};
GUID gGraphicsDeviceInfoHobGuid             = { 0xe5cb2ac9, 0xd35d, 0x4430, { 0x93, 0x6e, 0x1d, 0xe3, 0x32, 0x47, 0x8d, 0xe7 }};
GUID gUniversalPayloadSmbiosTableGuid       = { 0x590a0d26, 0x06e5, 0x4d20, { 0x8a, 0x82, 0x59, 0xea, 0x1b, 0x34, 0x98, 0x2d }};
GUID gUniversalPayloadAcpiTableGuid         = { 0x9f9a9506, 0x5597, 0x4515, { 0xba, 0xb6, 0x8b, 0xcd, 0xe7, 0x84, 0xba, 0x87 }};
GUID gUniversalPayloadExtraDataGuid         = { 0x15a5baf6, 0x1c91, 0x467d, { 0x9d, 0xfb, 0x31, 0x9d, 0x17, 0x8d, 0x4b, 0xb4 }};
GUID gUniversalPayloadSerialPortInfoGuid    = { 0xaa7e190d, 0xbe21, 0x4409, { 0x8e, 0x67, 0xa2, 0xcd, 0x0f, 0x61, 0xe1, 0x70 }};

/**
  Allocates one or more pages of type BootServicesData.

  Allocates the number of pages of MemoryType and returns a pointer to the
  allocated buffer.  The buffer returned is aligned on a 4KB boundary.
  If Pages is 0, then NULL is returned.
  If there is not enough memory availble to satisfy the request, then NULL
  is returned.

  @param   Pages                 The number of 4 KB pages to allocate.
  @return  A pointer to the allocated buffer or NULL if allocation fails.
**/
VOID *
AllocatePages (
  IN UINTN  Pages
  )
{
  ADDRESS    Offset;

  if (Pages == 0) {
    return NULL;
  }

  // Make sure allocation address is page alligned.
  Offset = mShimLayerMemory.FreeMemoryTop & PAGE_MASK;
  if (Offset != 0) {
    mShimLayerMemory.FreeMemoryTop -= Offset;
  }

  //
  // Check available memory for the allocation
  //
  if (mShimLayerMemory.FreeMemoryTop - (Pages * PAGE_SIZE) < mShimLayerMemory.FreeMemoryBottom) {
    return NULL;
  }

  mShimLayerMemory.FreeMemoryTop -= Pages * PAGE_SIZE;
  BuildMemAllocFdtNode (mShimLayerMemory.FreeMemoryTop, Pages * PAGE_SIZE, BootServicesData);

  return (VOID *)(UINTN)mShimLayerMemory.FreeMemoryTop;
}

/**
  Acquire the coreboot memory table with the given table id

  @param  TableId            Table id to be searched
  @param  MemTable           Pointer to the base address of the memory table
  @param  MemTableSize       Pointer to the size of the memory table

  @retval RETURN_SUCCESS     Successfully find out the memory table.
  @retval RETURN_INVALID_PARAMETER  Invalid input parameters.
  @retval RETURN_NOT_FOUND   Failed to find the memory table.

**/
RETURN_STATUS
ParseCbmemInfo (
  IN  UINT32  TableId,
  OUT VOID    **MemTable,
  OUT UINT32  *MemTableSize
  )
{
  RETURN_STATUS           Status;
  CB_MEMORY               *Rec;
  struct cb_memory_range  *Range;
  UINT64                  Start;
  UINT64                  Size;
  UINTN                   Index;
  struct cbmem_root       *CbMemLgRoot;
  VOID                    *CbMemSmRoot;
  VOID                    *CbMemSmRootTable;
  UINT32                   SmRootTableSize;
  struct imd_root_pointer *SmRootPointer;

  if (MemTable == NULL) {
    return RETURN_INVALID_PARAMETER;
  }

  *MemTable = NULL;
  Status    = RETURN_NOT_FOUND;

  //
  // Get the coreboot memory table
  //
  Rec = (CB_MEMORY *)FindCbTag (CB_TAG_MEMORY);
  if (Rec == NULL) {
    return Status;
  }

  for (Index = 0; Index < MEM_RANGE_COUNT (Rec); Index++) {
    Range = MEM_RANGE_PTR (Rec, Index);
    Start = cb_unpack64 (Range->start);
    Size  = cb_unpack64 (Range->size);

    if ((Range->type == CB_MEM_TABLE) && (Start > 0x1000)) {
      CbMemLgRoot = (struct  cbmem_root *)(UINTN)(Start + Size - DYN_CBMEM_ALIGN_SIZE);
      Status      = FindCbMemTable (CbMemLgRoot, TableId, MemTable, MemTableSize);
      if (!ERROR (Status)) {
        break;
      } else {
        /* Try to locate small root table and find the target CBMEM entry in small root table */
        Status        = FindCbMemTable (CbMemLgRoot, CBMEM_ID_IMD_SMALL, &CbMemSmRootTable, &SmRootTableSize);
        SmRootPointer = (struct imd_root_pointer *)(UINTN)((UINTN) CbMemSmRootTable + SmRootTableSize - sizeof (struct imd_root_pointer));
        CbMemSmRoot   = (struct cbmem_root *)(UINTN)(SmRootPointer->root_offset + (UINTN) SmRootPointer);
        if (!ERROR (Status)) {
          Status = FindCbMemTable ((struct cbmem_root *) CbMemSmRoot, TableId, MemTable, MemTableSize);
          if (!ERROR (Status)) {
            break;
          }
        }
      }
    }
  }

  return Status;
}

/**
   Callback function to find TOLUD (Top of Lower Usable DRAM)

   Estimate where TOLUD (Top of Lower Usable DRAM) resides. The exact position
   would require platform specific code.

   @param MemoryMapEntry         Memory map entry info got from bootloader.
   @param Params                 Not used for now.

  @retval SUCCESS            Successfully updated mTopOfLowerUsableDram.
**/
RETURN_STATUS
FindToludCallback (
  IN MEMORY_MAP_ENTRY  *MemoryMapEntry,
  IN VOID              *Params
  )
{
  //
  // This code assumes that the memory map on this x86 machine below 4GiB is continous
  // until TOLUD. In addition it assumes that the bootloader provided memory tables have
  // no "holes" and thus the first memory range not covered by e820 marks the end of
  // usable DRAM. In addition it's assumed that every reserved memory region touching
  // usable RAM is also covering DRAM, everything else that is marked reserved thus must be
  // MMIO not detectable by bootloader/OS
  //

  //
  // Skip memory types not RAM or reserved
  //
  if ((MemoryMapEntry->Type == E820_UNUSABLE) || (MemoryMapEntry->Type == E820_DISABLED) ||
      (MemoryMapEntry->Type == E820_PMEM))
  {
    return SUCCESS;
  }

  //
  // Skip resources above 4GiB
  //
  if ((MemoryMapEntry->Base + MemoryMapEntry->Size) > 0x100000000ULL) {
    return SUCCESS;
  }

  if ((MemoryMapEntry->Type == E820_RAM) || (MemoryMapEntry->Type == E820_ACPI) ||
      (MemoryMapEntry->Type == E820_NVS))
  {
    //
    // It's usable DRAM. Update TOLUD.
    //
    if (mTopOfLowerUsableDram < (MemoryMapEntry->Base + MemoryMapEntry->Size)) {
      mTopOfLowerUsableDram = (UINT32)(MemoryMapEntry->Base + MemoryMapEntry->Size);
    }
  } else {
    //
    // It might be 'reserved DRAM' or 'MMIO'.
    //
    // If it touches usable DRAM at Base assume it's DRAM as well,
    // as it could be bootloader installed tables, TSEG, GTT, ...
    //
    if (mTopOfLowerUsableDram == MemoryMapEntry->Base) {
      mTopOfLowerUsableDram = (UINT32)(MemoryMapEntry->Base + MemoryMapEntry->Size);
    }
  }

  return SUCCESS;
}

/**
   Callback function to build resource descriptor HOB

   This function build a HOB based on the memory map entry info.
   Only add RESOURCE_SYSTEM_MEMORY.

   @param MemoryMapEntry         Memory map entry info got from bootloader.
   @param Params                 Not used for now.

  @retval RETURN_SUCCESS        Successfully build a HOB.
**/
RETURN_STATUS
MemInfoCallback (
  IN MEMORY_MAP_ENTRY  *MemoryMapEntry,
  IN VOID              *Params
  )
{
  RESOURCE_TYPE            ResourceType;
  RESOURCE_ATTRIBUTE_TYPE  ResourceAttribute;
  ADDRESS                  PhysicalStart;
  UINT64                   NumberOfBytes;

  //
  // Skip everything not known to be usable DRAM.
  // It will be added later.
  //
  if ((MemoryMapEntry->Type != E820_RAM) && (MemoryMapEntry->Type != E820_ACPI) &&
      (MemoryMapEntry->Type != E820_NVS))
  {
    return RETURN_SUCCESS;
  }

  ResourceType = RESOURCE_SYSTEM_MEMORY;
  PhysicalStart = (ADDRESS)MemoryMapEntry->Base;
  NumberOfBytes = MemoryMapEntry->Size;
  ResourceAttribute = RESOURCE_ATTRIBUTE_PRESENT |
                      RESOURCE_ATTRIBUTE_INITIALIZED |
                      RESOURCE_ATTRIBUTE_TESTED |
                      RESOURCE_ATTRIBUTE_UNCACHEABLE |
                      RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
                      RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
                      RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE;

  BuildMemInfoFdt (PhysicalStart, NumberOfBytes, ResourceAttribute);

  if (MemoryMapEntry->Type == E820_ACPI) {
    BuildMemAllocFdtNode (PhysicalStart, NumberOfBytes, ACPIReclaimMemory);
  } else if (MemoryMapEntry->Type == E820_NVS) {
    BuildMemAllocFdtNode (PhysicalStart, NumberOfBytes, ACPIMemoryNVS);
  }

  return RETURN_SUCCESS;
}

/**
   Callback function to build resource descriptor HOB

   This function build a HOB based on the memory map entry info.
   It creates only RESOURCE_MEMORY_MAPPED_IO and RESOURCE_MEMORY_RESERVED
   resources.

   @param MemoryMapEntry         Memory map entry info got from bootloader.
   @param Params                 A pointer to ACPI_BOARD_INFO.

  @retval SUCCESS            Successfully build a HOB.
  @retval INVALID_PARAMETER  Invalid parameter provided.
**/
RETURN_STATUS
MemInfoCallbackMmio (
  IN MEMORY_MAP_ENTRY  *MemoryMapEntry,
  IN VOID              *Params
  )
{
  RESOURCE_TYPE            ResourceType;
  RESOURCE_ATTRIBUTE_TYPE  ResourceAttribute;
  ADDRESS         PhysicalStart;
  UINT64                       NumberOfBytes;

  //
  // Skip types already handled in MemInfoCallback
  //
  if ((MemoryMapEntry->Type == E820_RAM) || (MemoryMapEntry->Type == E820_ACPI)) {
    return SUCCESS;
  }

  if (MemoryMapEntry->Base < mTopOfLowerUsableDram) {
    //
    // It's in DRAM and thus must be reserved
    //
    ResourceType = RESOURCE_MEMORY_RESERVED;
  } else if ((MemoryMapEntry->Base < 0x100000000ULL) && (MemoryMapEntry->Base >= mTopOfLowerUsableDram)) {
    //
    // It's not in DRAM, must be MMIO
    //
    ResourceType = RESOURCE_MEMORY_MAPPED_IO;
  } else {
    ResourceType = RESOURCE_MEMORY_RESERVED;
  }

  PhysicalStart = MemoryMapEntry->Base;
  NumberOfBytes = MemoryMapEntry->Size;
  ResourceAttribute = RESOURCE_ATTRIBUTE_PRESENT |
                      RESOURCE_ATTRIBUTE_INITIALIZED |
                      RESOURCE_ATTRIBUTE_TESTED |
                      RESOURCE_ATTRIBUTE_UNCACHEABLE |
                      RESOURCE_ATTRIBUTE_WRITE_COMBINEABLE |
                      RESOURCE_ATTRIBUTE_WRITE_THROUGH_CACHEABLE |
                      RESOURCE_ATTRIBUTE_WRITE_BACK_CACHEABLE;

  BuildReservedMemFdt (PhysicalStart, NumberOfBytes, ResourceType, ResourceAttribute);

  if ((MemoryMapEntry->Type == E820_UNUSABLE) ||
      (MemoryMapEntry->Type == E820_DISABLED))
  {
    BuildMemAllocFdtNode (PhysicalStart, NumberOfBytes, UnusableMemory);
  } else if (MemoryMapEntry->Type == E820_PMEM) {
    BuildMemAllocFdtNode (PhysicalStart, NumberOfBytes, PersistentMemory);
  }

  return SUCCESS;
}

/**
  It will build FDT based on memory information from Fdt.
  @param[in] FdtBase         Address of the Fdt data.
  @retval SUCCESS        If it completed successfully.
  @retval Others             If it failed to build required FDT.
**/
RETURN_STATUS
BuildFdtForMemory (
  VOID
  )
{
  RETURN_STATUS Status;

  //
  // Parse memory info and build memory HOBs for Usable RAM
  //
  Status = ParseMemoryInfo (MemInfoCallback, NULL);
  if (ERROR (Status)) {
    return Status;
  }

  return SUCCESS;
}

/**
  It will build FDT based on MMIO information from Cbmem.
  Add free memory region to here.

  @param[in] FdtBase         Address of the Fdt data.
  @retval SUCCESS            If it completed successfully.
  @retval Others             If it failed to parse CBMEM data.
**/
RETURN_STATUS
BuildFdtForReservedMemory (
  VOID
  )
{
  RETURN_STATUS                   Status;
  RESOURCE_ATTRIBUTE_TYPE         Attribue;
  UNIVERSAL_PAYLOAD_ACPI_TABLE    AcpiTable;
  struct acpi_rsdp         *Rsdp = NULL;
  struct acpi_xsdt         *Xsdt = NULL;
  struct acpi_table_header *AcpiTableHeader = NULL;
  struct acpi_madt         *TargetMadt = NULL;
  INT32 i;
  UINT8 *XsdtEnd = NULL;

  CreateReservedMemFdt ();

  //
  // Parse memory info and build memory HOBs for reserved DRAM and MMIO
  //
  Status = ParseMemoryInfo (MemInfoCallbackMmio, NULL);
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

  Rsdp = (struct acpi_rsdp *)AcpiTable.Rsdp;
  Xsdt = (struct acpi_xsdt *)((UINT32)Rsdp->xsdt_address);
  XsdtEnd = (UINT8 *)Xsdt + Xsdt->header.length;

  for (i = 0; ((UINT8 *)&Xsdt->entry[i]) < XsdtEnd; i++) {
    AcpiTableHeader = (struct acpi_table_header *)Xsdt->entry[i];
    if (AsciiStrnCmp ((CONST CHAR8 *)AcpiTableHeader->signature, "APIC", 4) == 0){
      TargetMadt = (struct acpi_madt *)AcpiTableHeader;
      break;
    }
  }

  //
  // Report Local APIC range, cause sbl HOB to be NULL, comment now
  //
  Attribue = (
              RESOURCE_ATTRIBUTE_PRESENT |
              RESOURCE_ATTRIBUTE_INITIALIZED |
              RESOURCE_ATTRIBUTE_UNCACHEABLE |
              RESOURCE_ATTRIBUTE_TESTED
              );

  if (TargetMadt != NULL) {
    BuildReservedMemFdt ((ADDRESS)TargetMadt->lapic_addr, SIZE_512KB, RESOURCE_MEMORY_MAPPED_IO, Attribue);
    BuildMemAllocFdtNode ((ADDRESS)TargetMadt->lapic_addr, SIZE_512KB, MemoryMappedIO);
  } else {
    return NOT_FOUND;
  }

  BuildReservedMemUefi ((VOID  *)mShimLayerMemory.FreeMemoryBottom, (VOID  *)mShimLayerMemory.FreeMemoryTop);

  return SUCCESS;
}

/**
  It will build FDT based on memory allocation information from Fdt.
  @retval SUCCESS        If it completed successfully.
  @retval Others             If it failed to build required FDT.
**/
RETURN_STATUS
BuildFdtForMemAlloc (
  VOID
  )
{
  BuildFdtMemAlloc ();

  return SUCCESS;
}

/**
  It will build FDT based on information from Hobs.

  @retval SUCCESS        If it completed successfully.
  @retval Others             If it failed to build required FDT.
**/
RETURN_STATUS
BuildFdtMemory (
  VOID
  )
{
  RETURN_STATUS                          Status;

  //
  // First find TOLUD
  //
  Status = ParseMemoryInfo (FindToludCallback, NULL);
  if (ERROR (Status)) {
    return Status;
  }

  Status = BuildFdtForMemory ();
  if (ERROR (Status)) {
    return Status;
  }

  Status = BuildFdtForReservedMemory ();
  if (ERROR (Status)) {
    return Status;
  }

  Status = BuildFdtForMemAlloc ();
  if (ERROR (Status)) {
    return Status;
  }

  return Status;
}

/**
  Initialize memory pool by definition.

  @retval SUCCESS        If it completed successfully.
  @retval Others         If it failed to set memory.
**/
RETURN_STATUS
InitShimMem () {
  UINTN    MemBase;
  UINTN    HobMemBase;
  UINTN    HobMemTop;

  //
  // MEMBASE, MEMSIZE and UEFI_REGION_SIZE define in GNUmakefile.
  //
  MemBase    = MEMBASE;
  HobMemBase = ALIGN_VALUE (MemBase + MEMSIZE, SIZE_1MB);
  HobMemTop  = HobMemBase + UEFI_REGION_SIZE;

  mShimLayerMemory.MemoryBottom     = MemBase;
  mShimLayerMemory.MemoryTop        = HobMemTop;
  mShimLayerMemory.FreeMemoryBottom = HobMemBase;
  mShimLayerMemory.FreeMemoryTop    = HobMemTop;

  return SUCCESS;
}

RETURN_STATUS
ConvertCbmemToFdt (
  VOID
  )
{
  RETURN_STATUS                          Status;

  FdtTableInit ();

  Status = BuildFdtForUPL ();
  if (ERROR (Status)) {
    return Status;
  }

  Status = BuildFdtMemory ();
  if (ERROR (Status)) {
    return Status;
  }

  return SUCCESS;
}

RETURN_STATUS
HandOffToPayload (
  IN     ADDRESS    UniversalPayloadEntry,
  IN     VOID       *FdtTable
  )
{
  typedef VOID (*PayloadEntry) (UINTN);
  ((PayloadEntry) (UINTN) UniversalPayloadEntry) ((UINTN)FdtTable);

  return SUCCESS;
}

RETURN_STATUS
LocateAndDecompressPayload (
  OUT VOID **Dest
  )
{
  RETURN_STATUS                Status;
  ADDRESS                      SourceAddress;
  ADDRESS                      CBFSAddress;
  UINT64                       ImageSize;
  UINT64                       CBFSEntryAddrEnd;
  UINT32                       FMapEntrySize;
  UINT32                       Index;
  UINT32                       Alignment;
  UINTN                        DestSize, ScratchSize;
  UINTN                        CBFSEntrySize;
  VOID                         *FMapEntry;
  VOID                         *MyDestAddress, *ScratchAddress;
  struct fmap_area             *FMapArea;
  struct cbfs_payload_segment  *FirstSegment;
  union cbfs_mdata             *CBFSEntry;

  SourceAddress     = 0;
  CBFSAddress       = 0;
  CBFSEntryAddrEnd  = 0;

  Status = ParseCbmemInfo (CBMEM_ID_FMAP, &FMapEntry, &FMapEntrySize);
  if (ERROR (Status)) {
    return NOT_FOUND;
  }
  /*Locate fmap from CBMEM*/
  FMapArea = (struct fmap_area *)((UINTN)FMapEntry + sizeof (struct fmap));
  for (Index = 0; Index < ((struct fmap *)FMapEntry)->nareas; Index++) {
    if (AsciiStrCmp ((const CHAR8 *)FMapArea->name, "COREBOOT") == 0){
      CBFSAddress   = ((struct fmap *)FMapEntry)->base + FMapArea->offset;
      CBFSEntrySize = (UINTN)FMapArea->size;
      break;
    }
    FMapArea = (struct fmap_area *)((UINTN)FMapArea + sizeof (struct fmap_area));
  }
  if (!CBFSAddress) {
    return NOT_FOUND;
  }

  //
  // Parse payload address from CBFS.
  //
  CBFSEntry         = (union cbfs_mdata *)CBFSAddress;
  CBFSEntryAddrEnd  = (UINT64)(CBFSAddress + CBFSEntrySize);

  while ((UINT64)CBFSEntry < CBFSEntryAddrEnd) {
    if (AsciiStrCmp (CBFSEntry->h.filename, CBFS_UNIVERSAL_PAYLOAD) == 0) {
      FirstSegment  = &((struct cbfs_payload *)(UINTN)((UINT64)CBFSEntry + SWAP32 (CBFSEntry->h.offset)))->segments;
      SourceAddress = (UINTN)FirstSegment + SWAP32 (FirstSegment->offset);
      ImageSize     = SWAP32 (FirstSegment->len);
      Alignment     = (FirstSegment->load_addr)>>32;
      Alignment     = SWAP32 (Alignment);
      break;
    }
    CBFSEntry = (union cbfs_mdata *)((UINTN)CBFSEntry + ALIGN_UP (SWAP32 (CBFSEntry->h.offset) + SWAP32 (CBFSEntry->h.len), CBFS_ALIGNMENT));
  }

  if (!SourceAddress) {
    return NOT_FOUND;
  }

  Status = LzmaUefiDecompressGetInfo((VOID *)(UINTN)SourceAddress, ImageSize, &DestSize, &ScratchSize);
  if (ERROR (Status)) {
    return Status;
  }
  DestSize += Alignment;
  MyDestAddress  = AllocatePages(SIZE_TO_PAGES(DestSize));
  MyDestAddress  = (VOID *) ALIGN_VALUE ((UINTN) MyDestAddress, Alignment);
  ScratchAddress = AllocatePages(SIZE_TO_PAGES(ScratchSize));

  Status = LzmaUefiDecompress ((VOID *)(UINTN)SourceAddress, ImageSize, MyDestAddress, ScratchAddress);
  *Dest = MyDestAddress;
  if (ERROR (Status)) {
    return Status;
  }
  return SUCCESS;
}

RETURN_STATUS
LoadPayload (
  OUT    ADDRESS        *ImageAddressArg   OPTIONAL,
  OUT    UINT64         *ImageSizeArg,
  OUT    ADDRESS        *UniversalPayloadEntry
  )
{
  RETURN_STATUS                  Status;
  UINT32                         Index;
  UINT16                         ExtraDataIndex;
  CHAR8                          *SectionName;
  UINTN                          Offset;
  UINTN                          Size;
  UINTN                          Length;
  UINT32                         ExtraDataCount;
  ELF_IMAGE_CONTEXT              Context;
  UNIVERSAL_PAYLOAD_EXTRA_DATA   *ExtraData;
  VOID *Dest;

  Status = LocateAndDecompressPayload (&Dest);
  if (ERROR (Status)) {
    return Status;
  }
  Status = ParseElfImage (Dest, &Context);
  if (ERROR (Status)) {
    return Status;
  }

  //
  // Get UNIVERSAL_PAYLOAD_INFO_HEADER and number of additional PLD sections.
  //

  ExtraDataCount = 0;
  for (Index = 0; Index < Context.ShNum; Index++) {
    Status = GetElfSectionName (&Context, Index, &SectionName);
    if (ERROR (Status)) {
      continue;
    }

    if (AsciiStrnCmp (SectionName, UNIVERSAL_PAYLOAD_EXTRA_SEC_NAME_PREFIX, UNIVERSAL_PAYLOAD_EXTRA_SEC_NAME_PREFIX_LENGTH) == 0) {
      Status = GetElfSectionPos (&Context, Index, &Offset, &Size);
      if (!ERROR (Status)) {
        ExtraDataCount++;
      }
    }
  }

  //
  // Report the additional PLD sections through HOB.
  //
  Length    = sizeof (UNIVERSAL_PAYLOAD_EXTRA_DATA) + ExtraDataCount * sizeof (UNIVERSAL_PAYLOAD_EXTRA_DATA_ENTRY);
  ExtraData = AllocatePages (SIZE_TO_PAGES (Length));
  ExtraData->Count           = ExtraDataCount;
  ExtraData->Header.Revision = UNIVERSAL_PAYLOAD_EXTRA_DATA_REVISION;
  ExtraData->Header.Length   = (UINT16)Length;
  if (ExtraDataCount != 0) {
    for (ExtraDataIndex = 0, Index = 0; Index < Context.ShNum; Index++) {
      Status = GetElfSectionName (&Context, Index, &SectionName);
      if (ERROR (Status)) {
        continue;
      }

      if (AsciiStrnCmp (SectionName, UNIVERSAL_PAYLOAD_EXTRA_SEC_NAME_PREFIX, UNIVERSAL_PAYLOAD_EXTRA_SEC_NAME_PREFIX_LENGTH) == 0) {
        Status = GetElfSectionPos (&Context, Index, &Offset, &Size);
        if (!ERROR (Status)) {
          AsciiStrCpyS (
            ExtraData->Entry[ExtraDataIndex].Identifier,
            sizeof (ExtraData->Entry[ExtraDataIndex].Identifier),
            SectionName + UNIVERSAL_PAYLOAD_EXTRA_SEC_NAME_PREFIX_LENGTH
            );
          ExtraData->Entry[ExtraDataIndex].Base = (UINTN)(Context.FileBase + Offset);
          ExtraData->Entry[ExtraDataIndex].Size = Size;
          ExtraDataIndex++;
        }
      }
    }
  }
  if (Context.ReloadRequired || (Context.PreferredImageAddress != Context.FileBase)) {
    Context.ImageAddress = AllocatePages (SIZE_TO_PAGES (Context.ImageSize));
  } else {
    Context.ImageAddress = Context.FileBase;
  }

  SetFdtUplExtraData ((ADDRESS)ExtraData);

  //
  // Load ELF into the required base
  //
  Status = LoadElfImage (&Context);
  if (!ERROR (Status)) {
    *ImageAddressArg        = (UINTN)Context.ImageAddress;
    *UniversalPayloadEntry  = Context.EntryPoint;
    *ImageSizeArg           = Context.ImageSize;
  }
  return Status;
}


/**

  Entry point to the C language phase of Shim Layer before UEFI payload.

  @param[in]   BootloaderParameter    The starting address of bootloader parameter block.

  @retval      It will not return if SUCCESS, and return error when passing bootloader parameter.

**/
RETURN_STATUS
_ModuleEntryPoint (
  IN  UINTN  BootloaderParameter
  )
{
  RETURN_STATUS   Status;
  ADDRESS         ImageAddress;
  UINT64          ImageSize;
  ADDRESS         UniversalPayloadEntry;
  VOID            *FdtBase;

  //
  // Set coreboot memory base address.
  //
  SetBootloaderParameter (BootloaderParameter);

  //
  // Initialize local memory pool.
  //
  InitShimMem ();

  //
  // Load ELF into the required base
  //
  Status = LoadPayload (&ImageAddress, &ImageSize, &UniversalPayloadEntry);

  //
  // Covert coreboot memory information and general information to FDT.
  // Due to passing the correct free memory region, do not AllocatePages() after this function.
  //
  ConvertCbmemToFdt ();

  //
  // Jump to Universal Payload Entry and pass FDT base address.
  //
  FdtBase = GetFdtTable ();
  HandOffToPayload (UniversalPayloadEntry, FdtBase);

  return SUCCESS;
}

#ifndef __FDTTABLE_H__
#define __FDTTABLE_H__

#include "ShimLayer.h"
#include <Library/FdtLib/libfdt.h>

typedef struct {
  IN ADDRESS      BaseAddress;
  IN UINT64       Length;
  IN MEMORY_TYPE  MemoryType;
} FDT_ALLOC_NODE;

typedef struct {
  FDT_ALLOC_NODE  Node[32];
  UINTN           Length;
} FDT_ALLOC_LIST;

typedef struct {
  UINT64  RelocateType;
  UINT64  Offset;
} FIT_RELOCATE_ITEM;

typedef struct {
  ADDRESS  ImageBase;
  ADDRESS  PayloadBaseAddress;
  UINT64   PayloadSize;
  UINTN    PayloadEntryOffset;
  UINTN    PayloadEntrySize;
  ADDRESS  PayloadEntryPoint;
  UINTN    RelocateTableOffset;
  UINTN    RelocateTableCount;
  ADDRESS  PayloadLoadAddress;
} FIT_IMAGE_CONTEXT;

typedef struct {
  UINT8  *Name;
  UINT32 Offset;
} PROPERTY_DATA;

typedef struct {
  UINT32    Tag;
  UINT32    Length;
  UINT32    NameOffset;
  CHAR8     Data[];
} FDT_PROPERTY;

#define IMAGE_BASE_OFFSET               OFFSET_OF (FIT_IMAGE_CONTEXT, ImageBase)
#define PAYLOAD_BASE_ADDR_OFFSET        OFFSET_OF (FIT_IMAGE_CONTEXT, PayloadBaseAddress)
#define PAYLOAD_BASE_SIZE_OFFSET        OFFSET_OF (FIT_IMAGE_CONTEXT, PayloadSize)
#define PAYLOAD_ENTRY_OFFSET_OFFSET     OFFSET_OF (FIT_IMAGE_CONTEXT, PayloadEntryOffset)
#define PAYLOAD_ENTRY_SIZE_OFFSET       OFFSET_OF (FIT_IMAGE_CONTEXT, PayloadEntrySize)
#define PAYLOAD_ENTRY_POINT_OFFSET      OFFSET_OF (FIT_IMAGE_CONTEXT, PayloadEntryPoint)
#define RELOCATE_TABLE_OFFSET_OFFSET    OFFSET_OF (FIT_IMAGE_CONTEXT, RelocateTableOffset)
#define RELOCATE_TABLE_COUNT_OFFSET     OFFSET_OF (FIT_IMAGE_CONTEXT, RelocateTableCount)
#define PAYLOAD_LOAD_ADDR_OFFSET        OFFSET_OF (FIT_IMAGE_CONTEXT, PayloadLoadAddress)

/**
  Returns the pointer to the Fdt table.

  @return The pointer to the Fdt table.

**/
VOID *
GetFdtTable (
  VOID
  );

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
  );

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
  );

/**
  Build memory information fdt node.

  @param  BaseAddress  The 64 bit address of the memory.
  @param  Length       The length of the memory in bytes.
  @param  Attribute    Attribute of the memory .

  @retval SUCCESS    If it completed successfully.

**/
RETURN_STATUS
BuildMemInfoFdt (
  IN  ADDRESS                  BaseAddress,
  IN  UINT64                   Length,
  IN  RESOURCE_ATTRIBUTE_TYPE  Attribute
  );

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
  );

/**
  Create mmio memory fdt node.

  @retval SUCCESS    If it completed successfully.

**/
RETURN_STATUS
CreateReservedMemFdt (
  VOID
  );

/**
  It will build FDT based on memory allocation information from Hobs.

  @retval SUCCESS  If it completed successfully.
  @retval Others   If it failed to build required FDT.
**/
RETURN_STATUS
BuildFdtMemAlloc (
  VOID
  );

/**
  Initialize Fdt table.

  @retval SUCCESS  If it completed successfully.

**/
RETURN_STATUS
FdtTableInit (
  VOID
  );

/**
  It will build FDT for UPL consumed.

  @retval SUCCESS        If it completed successfully.
  @retval Others         If it failed to build required FDT.

**/
RETURN_STATUS
BuildFdtForUPL (
  VOID
  );


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
  );

/**
  Parse the FIT image info.

  @param[in]  ImageBase      Memory address of an image.
  @param[out] Context        The FIT image context pointer.

  @retval UNSUPPORTED         Unsupported binary type.
  @retval SUCCESS             FIT binary is loaded successfully.

**/
RETURN_STATUS
ParseFitImage (
  IN   VOID                  *ImageBase,
  OUT  FIT_IMAGE_CONTEXT     *Context
  );

#endif
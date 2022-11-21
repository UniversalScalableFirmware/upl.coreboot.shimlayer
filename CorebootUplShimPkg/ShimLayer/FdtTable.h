
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
  Save extra data allocate address.

  @param  BaseAddress    Extra data start address

  @retval SUCCESS    If it completed successfully.

**/
RETURN_STATUS
SetFdtUplExtraData (
  IN  ADDRESS  BaseAddress
  );

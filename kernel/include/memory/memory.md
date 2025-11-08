# Memory Management

## Physical Memory Manager

The physical memory manager maintains a list of all memory map entries, referred to as _regions_.  

\image html pmm_dfd0.png

Each region is represented by a struct that includes a bitmap and associated metadata:

- Bitmap
- Bitmap length
- Base address
- Number of free pages
- Memory type

The bitmap tracks the allocation status of each page within the region.  
It’s implemented as an array of bytes, where each bit corresponds to a single page:
- A set bit indicates the page is in use.
- A cleared bit means the page is free and available.

The base address allows calculation of individual page addresses within the region.  
To quickly determine whether a region has available memory, the number of free pages is stored separately.  
The memory type field specifies the nature of the region—whether it contains usable RAM, a framebuffer, or other types of memory.

The memory manager exposes functions to allocate and free physical pages, enabling low-level memory handling.

\image html pmm_dfd1.png
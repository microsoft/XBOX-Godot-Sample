#pragma once

// This header defines a platform abstraction layer for the Party headers.

// The following SAL is defined as no-ops for platforms where they are not implemented.

#ifndef _Field_range_
#define _Field_range_(min, max)
#endif

#ifndef _Field_size_
#define _Field_size_(bytes)
#endif

#ifndef _Field_size_bytes_
#define _Field_size_bytes_(bytes)
#endif

#ifndef _In_
#define _In_
#endif

#ifndef _In_opt_
#define _In_opt_
#endif

#ifndef _In_reads_opt_
#define _In_reads_opt_(size)
#endif

#ifndef _In_reads_
#define _In_reads_(size)
#endif

#ifndef _Maybenull_
#define _Maybenull_
#endif

#ifndef _Null_terminated_
#define _Null_terminated_
#endif

#ifndef _Out_
#define _Out_
#endif

#ifndef _Out_opt_
#define _Out_opt_
#endif

#ifndef _Out_writes_all_
#define _Out_writes_all_(size)
#endif

#ifndef _Outptr_
#define _Outptr_
#endif

#ifndef _Outptr_opt_
#define _Outptr_opt_
#endif

#ifndef _Outptr_result_buffer_
#define _Outptr_result_buffer_(s)
#endif

#ifndef _Outptr_result_maybenull_
#define _Outptr_result_maybenull_
#endif

#ifndef _Out_writes_
#define _Out_writes_(bytes)
#endif

#ifndef _Out_writes_z_
#define _Out_writes_z_(bytes)
#endif

#ifndef _Out_writes_opt_z_
#define _Out_writes_opt_z_(bytes)
#endif

#ifndef _Pre_maybenull_
#define _Pre_maybenull_
#endif

#ifndef _Post_invalid_
#define _Post_invalid_
#endif

#ifndef _Ret_maybenull_
#define _Ret_maybenull_
#endif

#ifndef _Post_writable_byte_size_
#define _Post_writable_byte_size_(X)
#endif

#ifndef _Return_type_success_
#define _Return_type_success_(c)
#endif

#ifndef __stdcall
#define __stdcall
#endif

/* glibc bindings for target ABI version glibc 2.5 */
#if !defined (__LIBC_CUSTOM_BINDINGS_H__)

#  if !defined (__OBJC__) && !defined (__ASSEMBLER__)
#    if defined (__cplusplus)
extern "C" {
#    endif

/* Symbols redirected to earlier glibc versions */
__asm__(".symver inet6_opt_init, inet6_opt_init@GLIBC_2.5");
__asm__(".symver inet6_rth_init, inet6_rth_init@GLIBC_2.5");
__asm__(".symver splice, splice@GLIBC_2.5");
__asm__(".symver inet6_opt_find, inet6_opt_find@GLIBC_2.5");
__asm__(".symver vmsplice, vmsplice@GLIBC_2.5");
__asm__(".symver inet6_opt_get_val, inet6_opt_get_val@GLIBC_2.5");
__asm__(".symver __readlinkat_chk, __readlinkat_chk@GLIBC_2.5");
__asm__(".symver inet6_opt_set_val, inet6_opt_set_val@GLIBC_2.5");
__asm__(".symver inet6_rth_reverse, inet6_rth_reverse@GLIBC_2.5");
__asm__(".symver inet6_opt_next, inet6_opt_next@GLIBC_2.5");
__asm__(".symver inet6_opt_append, inet6_opt_append@GLIBC_2.5");
__asm__(".symver inet6_rth_getaddr, inet6_rth_getaddr@GLIBC_2.5");
__asm__(".symver inet6_opt_finish, inet6_opt_finish@GLIBC_2.5");
__asm__(".symver tee, tee@GLIBC_2.5");
__asm__(".symver inet6_rth_space, inet6_rth_space@GLIBC_2.5");
__asm__(".symver inet6_rth_add, inet6_rth_add@GLIBC_2.5");
__asm__(".symver inet6_rth_segments, inet6_rth_segments@GLIBC_2.5");

/* Symbols introduced in newer glibc versions, which must not be used */
__asm__(".symver ns_name_pton, ns_name_pton@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver __sched_cpualloc, __sched_cpualloc@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver __dprintf_chk, __dprintf_chk@GLIBC_DONT_USE_THIS_VERSION_2.8");
__asm__(".symver ns_put16, ns_put16@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver mkostemp, mkostemp@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver eventfd, eventfd@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver ns_msg_getflag, ns_msg_getflag@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver ns_put32, ns_put32@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver eventfd_read, eventfd_read@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver utimensat, utimensat@GLIBC_DONT_USE_THIS_VERSION_2.6");
__asm__(".symver futimens, futimens@GLIBC_DONT_USE_THIS_VERSION_2.6");
__asm__(".symver __obstack_vprintf_chk, __obstack_vprintf_chk@GLIBC_DONT_USE_THIS_VERSION_2.8");
__asm__(".symver timerfd_gettime, timerfd_gettime@GLIBC_DONT_USE_THIS_VERSION_2.8");
__asm__(".symver timerfd_settime, timerfd_settime@GLIBC_DONT_USE_THIS_VERSION_2.8");
__asm__(".symver eventfd_write, eventfd_write@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver ns_makecanon, ns_makecanon@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver __isoc99_vfwscanf, __isoc99_vfwscanf@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver __isoc99_fwscanf, __isoc99_fwscanf@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver mkostemp64, mkostemp64@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver __openat_2, __openat_2@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver ns_datetosecs, ns_datetosecs@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver epoll_pwait, epoll_pwait@GLIBC_DONT_USE_THIS_VERSION_2.6");
__asm__(".symver inotify_init1, inotify_init1@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver timerfd_create, timerfd_create@GLIBC_DONT_USE_THIS_VERSION_2.8");
__asm__(".symver strerror_l, strerror_l@GLIBC_DONT_USE_THIS_VERSION_2.6");
__asm__(".symver ns_parse_ttl, ns_parse_ttl@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver __vdprintf_chk, __vdprintf_chk@GLIBC_DONT_USE_THIS_VERSION_2.8");
__asm__(".symver __isoc99_vswscanf, __isoc99_vswscanf@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver __fread_chk, __fread_chk@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver ns_format_ttl, ns_format_ttl@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver __isoc99_swscanf, __isoc99_swscanf@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver __isoc99_vscanf, __isoc99_vscanf@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver ns_subdomain, ns_subdomain@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver ns_name_ntol, ns_name_ntol@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver __asprintf_chk, __asprintf_chk@GLIBC_DONT_USE_THIS_VERSION_2.8");
__asm__(".symver ns_name_rollback, ns_name_rollback@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver ns_name_ntop, ns_name_ntop@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver __vasprintf_chk, __vasprintf_chk@GLIBC_DONT_USE_THIS_VERSION_2.8");
__asm__(".symver ns_samename, ns_samename@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver ns_initparse, ns_initparse@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver __open64_2, __open64_2@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver __sched_cpufree, __sched_cpufree@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver __isoc99_vwscanf, __isoc99_vwscanf@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver ns_sprintrrf, ns_sprintrrf@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver ns_sprintrr, ns_sprintrr@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver __gnu_mcount_nc, __gnu_mcount_nc@GLIBC_DONT_USE_THIS_VERSION_2.8");
__asm__(".symver ns_name_skip, ns_name_skip@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver __isoc99_wscanf, __isoc99_wscanf@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver __isoc99_vsscanf, __isoc99_vsscanf@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver __fread_unlocked_chk, __fread_unlocked_chk@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver qsort_r, qsort_r@GLIBC_DONT_USE_THIS_VERSION_2.8");
__asm__(".symver ns_parserr, ns_parserr@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver __isoc99_vfscanf, __isoc99_vfscanf@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver ns_samedomain, ns_samedomain@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver __obstack_printf_chk, __obstack_printf_chk@GLIBC_DONT_USE_THIS_VERSION_2.8");
__asm__(".symver __isoc99_sscanf, __isoc99_sscanf@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver epoll_create1, epoll_create1@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver ns_name_pack, ns_name_pack@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver __sched_cpucount, __sched_cpucount@GLIBC_DONT_USE_THIS_VERSION_2.6");
__asm__(".symver __isoc99_fscanf, __isoc99_fscanf@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver ns_name_uncompress, ns_name_uncompress@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver signalfd, signalfd@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver ns_name_unpack, ns_name_unpack@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver sync_file_range, sync_file_range@GLIBC_DONT_USE_THIS_VERSION_2.6");
__asm__(".symver dup3, dup3@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver __isoc99_scanf, __isoc99_scanf@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver ns_get16, ns_get16@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver __mq_open_2, __mq_open_2@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver __open_2, __open_2@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver ns_get32, ns_get32@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver sched_getcpu, sched_getcpu@GLIBC_DONT_USE_THIS_VERSION_2.6");
__asm__(".symver __openat64_2, __openat64_2@GLIBC_DONT_USE_THIS_VERSION_2.7");
__asm__(".symver ns_skiprr, ns_skiprr@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver ns_name_compress, ns_name_compress@GLIBC_DONT_USE_THIS_VERSION_2.9");
__asm__(".symver pipe2, pipe2@GLIBC_DONT_USE_THIS_VERSION_2.9");

#    if defined (__cplusplus)
}
#    endif
#  endif /* !defined (__OBJC__) && !defined (__ASSEMBLER__) */
#endif

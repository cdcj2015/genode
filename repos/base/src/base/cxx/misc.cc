/*
 * \brief  Dummy functions to make the damn thing link
 * \author Norman Feske
 * \date   2006-04-07
 */

/*
 * Copyright (C) 2006-2013 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

#include <base/env.h>
#include <base/printf.h>
#include <base/stdint.h>
#include <base/sleep.h>
#include <base/thread.h>
#include <util/string.h>

using namespace Genode;


extern "C" void __cxa_pure_virtual()
{
	PWRN("cxa pure virtual function called, return addr is %p",
	     __builtin_return_address(0));
}


extern "C" void __pure_virtual()
{
	PWRN("pure virtual function called");
}


/**
 * Prototype for exit-handler support function provided by '_main.cc'
 */
extern int genode___cxa_atexit(void(*func)(void*), void *arg,
                               void *dso_handle);


extern "C" int __cxa_atexit(void(*func)(void*), void *arg,
                            void *dso_handle)
{
	return genode___cxa_atexit(func, arg, dso_handle);
}


/**
 * Prototype for finalize support function provided by '_main.cc'
 */
extern int genode___cxa_finalize(void *dso);


extern "C" void __cxa_finalize(void *dso)
{
	genode___cxa_finalize(dso);
}


/***********************************
 ** Support required for ARM EABI **
 ***********************************/


extern "C" int __aeabi_atexit(void *arg, void(*func)(void*),
                              void *dso_handle)
{
	return genode___cxa_atexit(func, arg, dso_handle);
}


extern "C" __attribute__((weak))
void *__tls_get_addr()
{
	static long dummy_tls;
	return &dummy_tls;
}


/**
 * Not needed for exception-handling init on ARM EABI,
 * but called from 'init_exception'
 */
extern "C" __attribute__((weak))
void __register_frame(void *) { }


/**
 * Needed to build for OKL4-gta01 with ARM EABI
 */
extern "C" __attribute__((weak)) void raise()
{
	PDBG("raise called - not implemented\n");
}


/***************************
 ** Support for libsupc++ **
 ***************************/

extern "C" void *abort(void)
{
	Genode::Thread_base * myself = Genode::Thread_base::myself();
	char thread_name[64] = { "unknown" };

	if (myself)
		myself->name(thread_name, sizeof(thread_name));
	PWRN("abort called - thread: '%s'", thread_name);

	sleep_forever();
	return 0;
}


extern "C" void *fputc(void) {
	return 0;
}


extern "C" void *fputs(const char *s, void *) {
	PWRN("C++ runtime: %s", s);
	return 0;
}


extern "C" void *fwrite(void) {
	return 0;
}


extern "C" int memcmp(const void *p0, const void *p1, size_t size)
{
	return Genode::memcmp(p0, p1, size);
}


extern "C" __attribute__((weak))
void *memcpy(void *dst, void *src, size_t n)
{
//	PDBG("dst=%p, src=%p, n=%d", dst, src, n);
	return Genode::memcpy(dst, src, n);
}


extern "C" __attribute__((weak))
void *memmove(void *dst, void *src, size_t n)
{
//	PDBG("dst=%p, src=%p, n=%d", dst, src, n);
	return Genode::memmove(dst, src, n);
}


extern "C" __attribute__((weak))
void *memset(void *s, int c, size_t n)
{
//	PDBG("s=%p, c=%d, n=%d", s, c, n);
	return Genode::memset(s, c, n);
}


extern "C" void *stderr(void) {
	PWRN("stderr - not yet implemented");
	return 0;
}


/*
 * Used when having compiled libsupc++ with the FreeBSD libc
 */
struct FILE;
FILE *__stderrp;


extern "C" void *strcat(void)
{
	PWRN("strcat - not yet implemented");
	return 0;
}


extern "C" int strncmp(const char *s1, const char *s2, size_t n)
{
	return Genode::strcmp(s1, s2, n);
}


extern "C" char *strcpy(char *dest, const char *src)
{
	return Genode::strncpy(dest, src, ~0UL);
}


extern "C" size_t strlen(const char *s)
{
	return Genode::strlen(s);
}


extern "C" int strcmp(const char *s1, const char *s2)
{
	return Genode::strcmp(s1, s2);
}


/*
 * Needed by ARM EABI (gcc-4.4 Codesourcery release1039)
 */
extern "C" int sprintf(char *str, const char *format, ...)
{
	PWRN("sprintf - not implemented");
	return 0;
}


/**********************************
 ** Support for stack protection **
 **********************************/

extern "C" __attribute__((weak)) void __stack_chk_fail_local(void)
{
	PERR("Violated stack boundary");
}

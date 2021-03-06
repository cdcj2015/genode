/*
 * \brief  Support code for the thread API
 * \author Norman Feske
 * \author Stefan Kalkowski
 * \date   2010-01-13
 */

/*
 * Copyright (C) 2010-2015 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU General Public License version 2.
 */

/* Genode includes */
#include <rm_session/rm_session.h>
#include <ram_session/ram_session.h>
#include <base/printf.h>
#include <base/thread.h>

/* local includes */
#include <platform.h>
#include <map_local.h>
#include <dataspace_component.h>

using namespace Genode;


/**
 * Region-manager session for allocating thread contexts
 *
 * This class corresponds to the managed dataspace that is normally
 * used for organizing thread contexts with the thread context area.
 * In contrast to the ordinary implementation, core's version does
 * not split between allocation of memory and virtual memory management.
 * Due to the missing availability of "real" dataspaces and capabilities
 * refering to it without having an entrypoint in place, the allocation
 * of a dataspace has no effect, but the attachment of the thereby "empty"
 * dataspace is doing both: allocation and attachment.
 */
class Context_area_rm_session : public Rm_session
{
	private:

		using Ds_slab = Synchronized_allocator<Tslab<Dataspace_component,
		                                             get_page_size()> >;

		Ds_slab _ds_slab { platform()->core_mem_alloc() };

		enum { verbose = false };

	public:

		/**
		 * Allocate and attach on-the-fly backing store to thread-context area
		 */
		Local_addr attach(Dataspace_capability ds_cap, /* ignored capability */
		                  size_t size, off_t offset,
		                  bool use_local_addr, Local_addr local_addr,
		                  bool executable)
		{
			size = round_page(size);

			/* allocate physical memory */
			Untyped_address const untyped_addr =
				Untyped_memory::alloc(*platform_specific()->ram_alloc(), size);

			Untyped_memory::convert_to_page_frames(untyped_addr.phys(),
			                                       size >> get_page_size_log2());

			Dataspace_component *ds = new (&_ds_slab)
				Dataspace_component(size, 0, untyped_addr.phys(), CACHED, true, 0);
			if (!ds) {
				PERR("dataspace for core context does not exist");
				return (addr_t)0;
			}

			addr_t const core_local_addr =
				Native_config::context_area_virtual_base() + (addr_t)local_addr;

			if (verbose)
				PDBG("core_local_addr = %lx, phys_addr = %lx, size = 0x%zx",
				     core_local_addr, ds->phys_addr(), ds->size());

			if (!map_local(ds->phys_addr(), core_local_addr,
			               ds->size() >> get_page_size_log2())) {
				PERR("could not map phys %lx at local %lx",
				     ds->phys_addr(), core_local_addr);
				return (addr_t)0;
			}

			ds->assign_core_local_addr((void*)core_local_addr);

			return local_addr;
		}

		void detach(Local_addr local_addr) { PWRN("Not implemented!"); }

		Pager_capability add_client(Thread_capability) {
			return Pager_capability(); }

		void remove_client(Pager_capability) { }

		void fault_handler(Signal_context_capability) { }

		State state() { return State(); }

		Dataspace_capability dataspace() { return Dataspace_capability(); }
};


class Context_area_ram_session : public Ram_session
{
	public:

		Ram_dataspace_capability alloc(size_t size, Cache_attribute cached) {
			return reinterpret_cap_cast<Ram_dataspace>(Native_capability()); }

		void free(Ram_dataspace_capability ds) {
			PWRN("Not implemented!"); }

		int ref_account(Ram_session_capability ram_session) { return 0; }

		int transfer_quota(Ram_session_capability ram_session, size_t amount) {
			return 0; }

		size_t quota() { return 0; }

		size_t used() { return 0; }
};


/**
 * Return single instance of the context-area RM and RAM session
 */
namespace Genode {

	Rm_session *env_context_area_rm_session()
	{
		static Context_area_rm_session inst;
		return &inst;
	}

	Ram_session *env_context_area_ram_session()
	{
		static Context_area_ram_session inst;
		return &inst;
	}
}

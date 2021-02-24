/*
 * Copyright 2003-2021 The Music Player Daemon Project
 * http://www.musicpd.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "Init.hxx"
#include "../Internal.hxx"
#include "Client.hxx"
#include "ConnectionListener.hxx"
#include "ErrorHandler.hxx"
#include "Publisher.hxx"
#include "Service.hxx"
#include "util/RuntimeError.hxx"
#include "Log.hxx"

#include <avahi-common/domain.h>

class AvahiGlue final : Avahi::ErrorHandler {
public:
	Avahi::Client client;
	Avahi::Publisher publisher;

	AvahiGlue(EventLoop &event_loop,
		  const char *name, std::forward_list<Avahi::Service> services)
		:client(event_loop, *this),
		 publisher(client, name, std::move(services), *this)
	{
	}

	/* virtual methods from class Avahi::ErrorHandler */
	bool OnAvahiError(std::exception_ptr e) noexcept override {
		LogError(e);
		return true;
	}
};

static AvahiGlue *avahi_glue;

void
AvahiInit(EventLoop &loop, const char *serviceName, unsigned port)
{
	if (!avahi_is_valid_service_name(serviceName))
		throw FormatRuntimeError("Invalid zeroconf_name \"%s\"", serviceName);

	std::forward_list<Avahi::Service> services;
	services.emplace_front(AVAHI_IF_UNSPEC,
			       AVAHI_PROTO_UNSPEC,
			       SERVICE_TYPE, port);

	avahi_glue = new AvahiGlue(loop, serviceName, std::move(services));
}

void
AvahiDeinit()
{
	delete avahi_glue;
}
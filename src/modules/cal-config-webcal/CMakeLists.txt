set(extra_deps)
set(sources
	evolution-cal-config-webcal.c
)
set(extra_defines)
set(extra_cflags)
set(extra_incdirs)
set(extra_ldflags)

add_evolution_module(module-cal-config-webcal
	sources
	extra_deps
	extra_defines
	extra_cflags
	extra_incdirs
	extra_ldflags
)

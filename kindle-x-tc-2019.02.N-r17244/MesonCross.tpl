# c.f., https://mesonbuild.com/Cross-compilation.html
#
# Because of course we can't have nice things,
# we cannot (directly) use env vars in meson, so,
# this is a template that we'll process for each TC before using it...
#
# And if this seems fairly insane to you, welcome to the club.
# c.f., _meson_create_cross_file in Portage's meson eclass,
# which essentially resorts to the same trick:
# building a cross-file on-demand from the env right before calling meson...
#

[binaries]
c = '%CC%'				# $(command -v ${CROSS_TC}-gcc)
cpp = '%CXX%'				# $(command -v ${CROSS_TC}-g++)
ar = '%AR%'				# $(command -v ${CROSS_TC}-gcc-ar)
strip = '%STRIP%'			# $(command -v ${CROSS_TC}-strip)
# NOTE: Because setting PKG_CONFIG_SYSROOT_DIR usually fucks everything up, work it around...
#       c.f., the NOTEs on sys_root below...
#       Of course, we can't pass options to pkgconfig here, so we have to use a shell wrapper...
#pkgconfig = '%PKGCONFIG%'		# Other than that, we don't need a cross pkg-config, just to swap the search paths
# NOTE: But, since we're now avoiding the whole PKG_CONFIG_SYSROOT_DIR mess, just use our native pkg-config,
#       we're already asking it to only look for cross .pc files via PKG_CONFIG_LIBDIR,
#       and said cross .pc files already feed us prefixed paths.
# NOTE: See also https://mesonbuild.com/Release-notes-for-0-54-0.html#added-pkg_config_libdir-property
pkgconfig = '/usr/bin/pkg-config'
#exe_wrapper = 'qemu-arm-static'	# The whole point was not having to use qemu, so, no, thanks.

[properties]
# Build & run mesonx.c on the target if you're in doubt
sizeof_int = 4
sizeof_wchar_t = 4
sizeof_void* = 4

alignment_char = 1
alignment_void* = 4
alignment_double = 8

# Since 0.51.1, meson stupidly sanity checks the native compiler with the CFLAGS from the env.
# We deal with this insanity by clearing the env while invoking meson, and setting them properly here...
# NOTE: The docs (https://mesonbuild.com/Running-Meson.html#environment-variables)
#       seem to imply that env vars will only apply to the native TC, and NOT the cross one...
#       Besides being massively counter-intuitive, this is... weird,
#       and doesn't necessarily appear to always hold true in practice...
#       In any case, assume it is, so set them here, and clear them we we invoke meson... -_-"
# NOTE: Things are potentially better since 0.54.0,
#       c.f., https://mesonbuild.com/Release-notes-for-0-54-0.html#environment-variables-with-cross-builds
c_args = [ %CFLAGS% ]			# ${CPPFLAGS} ${CFLAGS}
c_link_args = [ %LDFLAGS% ]		# ${LDFLAGS}
cpp_args = [ %CXXFLAGS% ]		# ${CPPFLAGS} ${CXXFLAGS}
cpp_link_args = [ %LDFLAGS% ]		# ${LDFLAGS}
#sys_root = '%SYSROOT%'			# ${HOME}/x-tools/${CROSS_TC}/${CROSS_TC}/sysroot
# NOTE: Except this is used to set PKG_CONFIG_SYSROOT_DIR since meson 0.52, so we actually want to use %PREFIX% here...
#       This also happens to usually be extremely harmful, so we work around it in a pkg-config wrapper...
# NOTE: The good news is, this is apparently not used anywhere except to fuck pkg-config up (c.f., get_sys_root).
#       You'd think this would be used to pass --sysroot to the compiler, but, apparently, no.
#       So, just don't set it, it makes our lives markedly easier.
#sys_root = '%PREFIX%'			# c.f., below

# But, since 0.54.0 we now have the possibility of killing sanity checks with fire, yay!
# c.f., https://mesonbuild.com/Release-notes-for-0-54-0.html#skip-sanity-tests-when-cross-compiling
#skip_sanity_check = true

# Where we'll stuff custom properties if need be
#%MESON_PROPS%

[host_machine]
system = 'linux'
cpu_family = 'arm'
cpu = '%MARCH%'				# Documentation is wonderfully vague. Might be -march or -mtune?
					# echo ${CFLAGS} | tr ' ' '\n' | grep mtune | cut -d'=' -f 2
					# Doesn't appear terribly relevant anyway, since it appears to be a vague x86_64 on native...
endian = 'little'

[paths]
prefix = '%PREFIX%'			# ${TC_BUILD_DIR}
libdir = 'lib'
bindir = 'bin'

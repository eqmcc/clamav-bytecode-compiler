/*
 *  ClamAV bytecode startup checks and self-check.
 *
 *  Copyright (C) 2010 Sourcefire, Inc.
 *
 *  Authors: Török Edvin
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301, USA.
 */
const uint16_t __clambc_kind = BC_STARTUP;
int entrypoint()
{
  // Whole platform specific bugs can be disabled with check_platform,
  // see clamscan --debug for meaning of bits.
  // For example:
  //disable_jit_if("Pax mprotect on, with RWX", 0,
  //              check_platform(0x0affffff, 0xffffffff, 0x19));

  struct cli_environment env;
  get_environment(&env, sizeof(env));
  if (env.has_jit_compiled) {
    /* CPU checks */
    switch (env.arch) {
    case arch_i386:
      disable_jit_if("i[34]86 detected, JIT needs pentium or better",0,
                     !memcmp(env.cpu,"i386",4) ||
                     !memcmp(env.cpu,"i486",4));
      break;
    default:
      break;
    }

    /* RWX checks */
    if (!(env.os_features & (1 << feature_map_rwx))) {
      disable_jit_if("RWX mapping denied.", 0, 1);
      if (env.os == os_linux) {
        if (env.os_features & (1 << feature_selinux))
          /* all SELinux versions deny RWX mapping when policy says so */
          disable_jit_if("^SELinux is preventing 'execmem' access.\n"
                         "Run  'setsebool -P clamd_use_jit on'.", 0, 1);
        else if (env.os_features & (1 << feature_pax))
          /* recent versions of PaX deny RWX mapping */
          disable_jit_if("^PaX is preventing 'mprotect' access.\n"
                         "Run 'paxctl -cm <executable>'", 0, 1);
        else
          /* RWX mapping got denied but apparently not due to SELinux/PaX */
          disable_jit_if("^RWX mapping denied for unknown reason."
            "Please report to http://bugs.clamav.net\n", 0, 1);
      }
    } else {
      if ((env.os == os_linux || env.os_category == llvm_os_Linux) &&
          (env.os_features & (1 << feature_pax_mprotect))) {
        /* older versions of PaX allow RWX mapping but silently degrade it to RW
         * mapping and kill the program if it tries to execute. */
        disable_jit_if("^PaX is preventing 'mprotect' access.\n"
                       "Run 'paxctl -cm <executable>'", 0, 1);
      }
    }
  }
  int s = disable_bytecode_if("",0,0);
  switch (s) {
  case 0:
    debug("startup: bytecode execution in auto mode");
    break;
  case 1:
    debug("startup: bytecode execution with interpreter only");
    break;
  case 2:
    debug("startup: bytecode disabled");
    break;
  }

  /* check that the OS information is consistent */
  /* JIT == C++ code compiled */
  if (env.has_jit_compiled && !env.cpp_version) {
    return 0xdead1;
  }
  if (env.dconf_level < env.functionality_level) {
    return 0xdead2;
  }
  if (env.functionality_level != engine_functionality_level()) {
    return 0xdead3;
  }
  if (env.dconf_level != engine_dconf_level()) {
    return 0xdead4;
  }
  if (env.big_endian != __is_bigendian()) {
    return 0xdead5;
  }

  uint32_t a = (env.os_category << 24) | (env.arch << 20) |
    (env.compiler <<  16) | (env.functionality_level << 8) |
    (env.dconf_level);
  uint32_t b = (env.big_endian << 28) | (env.sizeof_ptr << 24) |
    env.cpp_version;
  uint32_t c = (env.os_features << 24) | env.c_version;
  if (a != env.platform_id_a) {
    debug_print_uint(a);
    return 0xdead6;
  }
  if (b != env.platform_id_b) {
    debug_print_uint(b);
    return 0xdead7;
  }
  if (c != env.platform_id_c) {
    debug_print_uint(c);
    return 0xdead8;
  }
  c = test1(0xf00dbeef, 0xbeeff00d);
  if (c != 0x12345678) {
    debug_print_uint(c);
    return 0xdead9;
  }
  c = test2(0xf00d);
  if (c != 0xd00f) {
    debug_print_uint(c);
    return 0xdead10;
  }

  /* check endianness and bswap */
  char buf[16] = {
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
  };
  char buf2[16];
  uint32_t x;
  memset(&buf2, 0, sizeof(buf2));
  if (cli_readint16(&buf) != 0x100)
    return 0xdead11;
  x = cli_readint32(&buf);
  if (x != 0x03020100)
    return 0xdead12;
  x = __builtin_bswap32(x);
  if (x != 0x00010203)
    return 0xdead13;
  cli_writeint32(&buf2, 0x12345678);
  if (cli_readint32(&buf2) != 0x12345678)
    return 0xdead14;
  if (buf2[0] != 0x78)
    return 0xdead15;

  /* magic number to tell libclamav that selftest succeeded */
  return 0xda7aba5e;
}

{
  'variables': {
    'gas_version%': '0.0',
    'llvm_version%': '0.0',
    'nasm_version%': '0.0',
  },
  'targets': [
    {
      'target_name': 'openssl',
      'type': '<(library)',
      'includes': ['./openssl_common.gypi'],
      'defines': [
        # Compile out hardware engines.  Most are stubs that dynamically load
        # the real driver but that poses a security liability when an attacker
        # is able to create a malicious DLL in one of the default search paths.
        'OPENSSL_NO_HW',
      ],
      'conditions': [
        [ 'openssl_no_asm==1', {
          'includes': ['./openssl_no_asm.gypi'],
        }, 'target_arch=="arm64" and OS=="win" or OS=="zos"', {
          # VC-WIN64-ARM inherits from VC-noCE-common that has no asms.
          # z/OS has no openssl asm
          'includes': ['./openssl_no_asm.gypi'],
        }, 'gas_version and v(gas_version) >= v("2.26") or '
           'nasm_version and v(nasm_version) >= v("2.11.8")', {
           # Require AVX512IFMA supported. See
           # https://www.openssl.org/docs/man1.1.1/man3/OPENSSL_ia32cap.html
           # Currently crypto/poly1305/asm/poly1305-x86_64.pl requires AVX512IFMA.
          'includes': ['./openssl_asm.gypi'],
        }, {
          'includes': ['./openssl_asm_avx2.gypi'],
        }],
      ],
      'direct_dependent_settings': {
        'include_dirs': [ 'openssl/include']
      }
    }, {
      # openssl-cli target
      'target_name': 'openssl-cli',
      'type': 'executable',
      'dependencies': ['openssl'],
      'includes': ['./openssl_common.gypi'],
      'conditions': [
        ['openssl_no_asm==1', {
          'includes': ['./openssl-cl_no_asm.gypi'],
        }, 'target_arch=="arm64" and OS=="win"', {
          # VC-WIN64-ARM inherits from VC-noCE-common that has no asms.
          'includes': ['./openssl-cl_no_asm.gypi'],
        }, {
          'includes': ['./openssl-cl_asm.gypi'],
        }],
      ],
    },
  ],
}

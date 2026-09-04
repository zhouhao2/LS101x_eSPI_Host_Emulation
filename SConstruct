import os

env = Environment(ENV=os.environ)
sdk_root = os.environ.get('SDK_ROOT', '../ls_sdk')
env['SDK_ROOT'] = Dir(sdk_root)
if env.get('IC') is None:
    env['IC'] = ARGUMENTS.get('ic', 'leo')
env.SConscript(env['SDK_ROOT'].File('soc/SConscript'), exports=['env'])

src = [
    'main.c',
    'espi_crc8.c',
    'espi_host.c',
]
inc = [
    '.',
]
env.app_build('espi_host_emulation', src, inc, ble=False)

project('vkforcepriority', ['c', 'cpp'], default_options: ['c_std=c11', 'cpp_std=c++17'])

cpp = meson.get_compiler('cpp')

lib_dir = get_option('libdir')
ld_lib_dir = ''

if false
    lib_dir = join_paths(lib_dir, 'vkforcepriority')
    ld_lib_dir = get_option('prefix') + '/$LIB/vkforcepriority/'
endif

data_dir = get_option('datadir')

vulkan_layer_dir = join_paths(data_dir, 'vulkan', 'implicit_layer.d')

configure_file(
    input : 'vkforcepriority.json.in',
    output : 'vkforcepriority.json',
    configuration : {'ld_lib_dir' : ld_lib_dir},
    install : true,
    install_dir : vulkan_layer_dir,
)

subdir('src')

vk_force_priority_src = [
    'layer_device.cpp',
    'layer_entry.cpp',
    'layer_instance.cpp',
    'vkdispatch.cpp',
]

shared_library(meson.project_name().to_lower(), 
    vk_force_priority_src,
    link_with: [ ],
    install : true,
    install_dir : lib_dir,
)

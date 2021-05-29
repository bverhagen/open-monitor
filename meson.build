project('open-monitor', 'cpp',
  version: '0.1.0',
  default_options: [
    'warning_level=3',
    'cpp_std=c++17',
    'cpp_rtti=true',
    'default_library=static'
  ]
)

gst_dep = dependency('gstreamer-1.0',
    fallback : ['gstreamer', 'gst_dep'])
gstwebrtc_dep = dependency('gstreamer-webrtc-1.0', version: '>= 1.14.0',
    fallback: ['gst-plugins-bad', 'gstwebrtc_dep'])
gstsdp_dep = dependency('gstreamer-sdp-1.0',
    fallback : ['gst-plugins-bad', 'gstwebrtc_dep'])
libsoup_dep = dependency('libsoup-2.4', version : '>=2.48',
    fallback : ['libsoup', 'libsoup_dep'])
json_glib_dep = dependency('json-glib-1.0',
    fallback : ['json-glib', 'json_glib_dep'])
glibmm_dep = dependency('glibmm-2.4')
boost_dep = dependency('boost', version: '>=1.67.0', static: get_option('use-static-boost'))
boost_program_options_dep = dependency('boost', modules: ['program_options'], static: get_option('use-static-boost'))

lua_dep = dependency('lua-5.3', required: get_option('use-system-lua'))
if not lua_dep.found()
  lua_proj = subproject('lua', default_options: ['default_library=static', 'werror=false'])
  lua_dep = lua_proj.get_variable('lua_dep')
endif

luawrapper = declare_dependency(
  include_directories: ['3rdparty/luawrapper'],
  dependencies: lua_dep,
)

install_data('LICENSE', install_dir : get_option('license-prefix'))
install_data('default_config.lua', rename : 'open-monitor.conf', install_dir : get_option('conf-prefix'))

subdir('src')
subdir('init')

if get_option('install-examples')
  subdir('example_configs')
endif

summary({
  'Prefix': get_option('prefix'),
  'License prefix': get_option('license-prefix')
}, section: 'Directories')

summary({
  'Copyright notice': get_option('copyright'),
  'Systemd': get_option('systemd').enabled()
}, section: 'Configuration', bool_yn: true)

if get_option('systemd').enabled()
  summary({
    'Systemd prefix': get_option('systemd-prefix')
  }, section: 'Systemd', bool_yn: true)
endif

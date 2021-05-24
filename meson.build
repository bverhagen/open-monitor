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

install_data('LICENSE', install_dir : get_option('license-prefix'))

subdir('src')
subdir('init')

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
